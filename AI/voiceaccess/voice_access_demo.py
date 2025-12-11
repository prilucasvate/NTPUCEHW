import warnings
warnings.filterwarnings("ignore", message="SpeechBrain ")
warnings.filterwarnings("ignore", category=FutureWarning)
warnings.filterwarnings("ignore", module="speechbrain")
warnings.filterwarnings("ignore", module="torch")

import os
import glob
import json

import torch
import torchaudio
import soundfile as sf
import torch.nn as nn
from typing import Dict, List
# === torchaudio 修補區：把新版 torchaudio 補上舊 API，避免 SpeechBrain 報錯 ===
if not hasattr(torchaudio, "list_audio_backends"):
    # 新版沒有這個函式，SpeechBrain 會掛掉 → 補一個假的給它用
    def _fake_list_audio_backends():
        # 我們其實沒有用 torchaudio 讀檔，只是讓 SpeechBrain 初始化順利過
        return ["sox_io"]
    torchaudio.list_audio_backends = _fake_list_audio_backends

if not hasattr(torchaudio, "get_audio_backend"):
    # 保險起見，也補上 get_audio_backend
    def _fake_get_audio_backend():
        return "sox_io"
    torchaudio.get_audio_backend = _fake_get_audio_backend
# === torchaudio 修補區結束 ===

from speechbrain.inference import SpeakerRecognition
# 如果下面有別的 speechbrain import，也全部放在這行後面
if not hasattr(nn, "Dropout1d"):
    nn.Dropout1d = nn.Dropout  # 若 PyTorch 版本太舊，臨時替代

DB_PATH = "db/speakers_db.pt"       # 存 embedding 的檔案
CFG_PATH = "db/config.json"         # 存設定（例如 threshold）
DEFAULT_THRESHOLD = 0.55            # 你可改 0.50~0.60

# ---------- 音訊與模型 ----------

def load_model():
    rec = SpeakerRecognition.from_hparams(
        source="speechbrain/spkrec-ecapa-voxceleb",
        savedir="pretrained_ecapa"
    )
    return rec

def load_wav(path: str, target_sr: int = 16000) -> torch.Tensor:
    data, sr = sf.read(path, always_2d=True)  # (T, C)
    data = data.mean(axis=1, keepdims=True).T  # (1, T)
    wav = torch.from_numpy(data).float()
    if sr != target_sr:
        wav = torchaudio.functional.resample(wav, sr, target_sr)
    return wav  # (1, T)

@torch.no_grad()
def embed(rec, wav: torch.Tensor) -> torch.Tensor:
    # SpeechBrain encode_batch 接受 (batch, time)
    if wav.dim() == 2:
        wav = wav[0]  # (1, T) -> (T,)
    emb = rec.encode_batch(wav.unsqueeze(0)).squeeze(0)  # (dim,)
    emb = torch.nn.functional.normalize(emb, dim=-1)     # L2 normalize
    return emb

# ---------- 資料庫 I/O ----------

def ensure_dirs():
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)

def load_db() -> Dict[str, torch.Tensor]:
    if os.path.exists(DB_PATH):
        return torch.load(DB_PATH)
    return {}

def save_db(db: Dict[str, torch.Tensor]):
    ensure_dirs()
    torch.save(db, DB_PATH)

def load_cfg() -> dict:
    if os.path.exists(CFG_PATH):
        with open(CFG_PATH, "r", encoding="utf-8") as f:
            return json.load(f)
    return {"threshold": DEFAULT_THRESHOLD}

def save_cfg(cfg: dict):
    ensure_dirs()
    with open(CFG_PATH, "w", encoding="utf-8") as f:
        json.dump(cfg, f, ensure_ascii=False, indent=2)

# ---------- 功能：註冊 / 驗證 ----------

def enroll_from_files(rec, name: str, wav_paths: List[str], db: Dict[str, torch.Tensor]):
    if not wav_paths:
        print("沒有提供任何音檔路徑。")
        return
    embs = []
    for p in wav_paths:
        if not os.path.exists(p):
            print(f"[警告] 找不到檔案：{p}")
            continue
        wav = load_wav(p)
        embs.append(embed(rec, wav))
    if not embs:
        print("所有檔案都讀取失敗，註冊中止。")
        return
    spk_emb = torch.stack(embs, dim=0).mean(dim=0)
    spk_emb = torch.nn.functional.normalize(spk_emb, dim=-1)
    db[name] = spk_emb
    save_db(db)
    print(f"✅ 註冊完成：{name}（使用 {len(embs)} 段語音，已寫入資料庫）")

def enroll_from_folder(rec, root_dir: str, db: Dict[str, torch.Tensor]):
    """
    讀取 data/enroll/<SpeakerName>/*.wav 批次註冊
    """
    if not os.path.exists(root_dir):
        print(f"資料夾不存在：{root_dir}")
        return
    speakers = sorted([d for d in os.listdir(root_dir) if os.path.isdir(os.path.join(root_dir, d))])
    if not speakers:
        print("找不到任何語者資料夾。")
        return
    for spk in speakers:
        paths = glob.glob(os.path.join(root_dir, spk, "*.wav"))
        if not paths:
            print(f"[跳過] {spk} 無音檔")
            continue
        enroll_from_files(rec, spk, paths, db)

def identify_file(rec, query_path: str, db: Dict[str, torch.Tensor], threshold: float):
    if not db:
        print("資料庫為空，請先註冊。")
        return
    if not os.path.exists(query_path):
        print(f"找不到檔案：{query_path}")
        return
    wav = load_wav(query_path)
    q = embed(rec, wav)
    best_name, best_score = None, -1.0
    # 也印出所有分數方便在報告裡展示
    all_scores = []
    for name, ref in db.items():
        score = torch.sum(q * ref).item()  # cosine（已 L2N）
        all_scores.append((name, score))
        if score > best_score:
            best_name, best_score = name, score
    all_scores.sort(key=lambda x: x[1], reverse=True)
    print("=== Score 排序（由高到低）===")
    for n, s in all_scores:
        print(f"{n:>10s} : {s:.3f}")
    print("============================")
    if best_score >= threshold:
        print(f"✅ 通過：{best_name}（cos={best_score:.3f} ≥ 門檻 {threshold:.2f}）→ 模擬：門已開啟")
    else:
        print(f"❌ 拒絕：Unknown（最佳 {best_name} cos={best_score:.3f} < 門檻 {threshold:.2f}）→ 模擬：門鎖住")

# ---------- 小工具 ----------

def list_speakers(db: Dict[str, torch.Tensor]):
    if not db:
        print("資料庫目前沒有任何語者。")
        return
    print("目前已註冊語者：")
    for name in sorted(db.keys()):
        print(f" - {name}")

def set_threshold(cfg: dict, value: float):
    cfg["threshold"] = float(value)
    save_cfg(cfg)
    print(f"已更新門檻 threshold = {cfg['threshold']:.2f}")

# ---------- 互動式選單 ----------

def main_menu():
    rec = load_model()
    db = load_db()
    cfg = load_cfg()
    while True:
        print("\n=== 語音門禁 Demo ===")
        print("1) 批次註冊（讀取資料夾 data/enroll/<name>/*.wav）")
        print("2) 單人註冊（輸入名稱與多個檔案路徑）")
        print("3) 登入驗證（選一個測試音檔）")
        print("4) 列出已註冊語者")
        print(f"5) 設定門檻（目前 {cfg['threshold']:.2f}）")
        print("0) 離開")
        choice = input("請輸入選項：").strip()
        if choice == "1":
            enroll_from_folder(rec, "data/enroll", db)
        elif choice == "2":
            name = input("輸入語者名稱：").strip()
            files = input("輸入音檔路徑，多個以逗號分隔：").strip().split(",")
            files = [f.strip() for f in files if f.strip()]
            enroll_from_files(rec, name, files, db)
        elif choice == "3":
            path = input("輸入要驗證的音檔路徑：").strip()
            identify_file(rec, path, db, cfg["threshold"])
        elif choice == "4":
            list_speakers(db)
        elif choice == "5":
            val = input("輸入新的門檻（建議 0.50~0.60）：").strip()
            try:
                set_threshold(cfg, float(val))
            except:
                print("門檻需為數字。")
        elif choice == "0":
            print("bye 👋")
            break
        else:
            print("無效選項，重新輸入。")

if __name__ == "__main__":
    main_menu()
