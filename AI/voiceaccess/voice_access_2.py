import warnings
warnings.filterwarnings("ignore", message="SpeechBrain ")
warnings.filterwarnings("ignore", category=FutureWarning)
warnings.filterwarnings("ignore", module="speechbrain")
warnings.filterwarnings("ignore", module="torch")

import os
import json
import time
from typing import Dict, List

import torch
import torchaudio
import torch.nn as nn
import sounddevice as sd
import shutil
from pathlib import Path
import numpy as np

# ================== fix Windows ==================

# 1) torchaudio 新版拿掉 list_audio_backends，SpeechBrain 會炸
if not hasattr(torchaudio, "list_audio_backends"):
    def _fake_list_audio_backends():
        # 只是讓 SpeechBrain 初始化不要掛
        return ["sox_io"]
    torchaudio.list_audio_backends = _fake_list_audio_backends

if not hasattr(torchaudio, "get_audio_backend"):
    def _fake_get_audio_backend():
        return "sox_io"
    torchaudio.get_audio_backend = _fake_get_audio_backend

# 2) 匯入 speechbrain 的 fetching，改掉它用 symlink 的行為
import speechbrain.utils.fetching as sb_fetching

def _safe_link_with_strategy(src, dst, local_strategy=None):
    """
    SpeechBrain 預設會用 symlink，把 cache 連到 savedir。
    Windows 一堆情況沒權限建立 symlink → 直接改成 copy 檔案。
    """
    src = Path(src)
    dst = Path(dst)
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)
    return dst

# 覆蓋掉 SpeechBrain 原本的 link_with_strategy
sb_fetching.link_with_strategy = _safe_link_with_strategy

# 3) 再來才匯入 SpeakerRecognition
from speechbrain.inference import SpeakerRecognition

# 某些舊版 torch 沒有 Dropout1d，這裡做相容處理（新版本也不會有問題）
if not hasattr(nn, "Dropout1d"):
    nn.Dropout1d = nn.Dropout

# ================== 基本設定 ==================

DB_PATH = "db/speakers_db.pt"       # 存 embedding 的檔案
CFG_PATH = "db/config.json"         # 存設定（例如 threshold）
DEFAULT_THRESHOLD = 0.55            # 建議 0.50~0.60
SAMPLE_RATE = 16000                 # ECAPA 模型使用 16kHz
DEFAULT_REC_SECS = 5.0              # 每次錄音秒數
SUGGEST_LINES_ENROLL = [
    "你好，我正在測試語音辨識系統，請聽清楚我的聲音。",
    "我希望這個模型能正確記住我，辨識出我的聲音特色。",
    "這是一段普通的描述，用來讓模型學習我的聲音特徵。",
    "每個人都有獨特的頻率組成，這段話能更準確辨識我。",
    "明亮的燈光映照在寧靜的房間裡，我緩緩地說著這段話。"
]
# ================== 模型與資料庫 ==================

def ensure_dirs():
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)

def load_model():
    """
    下載 / 載入 SpeechBrain 的 ECAPA 語者辨識模型。
    """
    model_dir = os.path.join(os.path.expanduser("~"), "sb_pretrained_ecapa")
    os.makedirs(model_dir, exist_ok=True)

    rec = SpeakerRecognition.from_hparams(
        source="speechbrain/spkrec-ecapa-voxceleb",
        savedir=model_dir,
        run_opts={"device": "cpu"},  # 筆電只有 CPU
    )
    return rec

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
# =================== 逼聲 ==================
def beep(freq=600, duration=0.25, sr=16000):
    """播放一個簡單的sine嗶聲"""
    t = np.linspace(0, duration, int(sr * duration), endpoint=False)
    wave = 0.2 * np.sin(2 * np.pi * freq * t)  # 0.2 避免太大聲爆音
    sd.play(wave, samplerate=sr)
    sd.wait()
# ================== 音訊處理 ==================

def record_audio(duration: float = DEFAULT_REC_SECS, sr: int = SAMPLE_RATE) -> torch.Tensor:
    """
    從麥克風錄音 `duration` 秒，回傳 shape = (1, T) 的 torch.Tensor，float32。
    """
    num_samples = int(duration * sr)
    print(f"\n準備開始錄音，將錄 {duration:.1f} 秒。")
    #print("請在稍後說 : 我正在進行語音識別登入 請確認我身分資訊。")
    input("按 Enter 開始錄音，請在「嗶」聲後開始說話...")
    print("3...")
    time.sleep(1.0)
    print("2...")
    time.sleep(1.0)
    print("1...")
    time.sleep(1.0)
    beep() # 嗶
    print("錄音中，請開始說話！")

    audio = sd.rec(num_samples, samplerate=sr, channels=1, dtype="float32")
    sd.wait()
    print("錄音結束。\n")

    # audio: shape (T, 1) -> (1, T)
    wav = torch.from_numpy(audio.squeeze(1)).unsqueeze(0)  # (1, T)
    return wav

@torch.no_grad()
def embed(rec: SpeakerRecognition, wav: torch.Tensor) -> torch.Tensor:
    """
    把一段語音轉成語者 embedding（已做 L2 normalize）
    wav: shape (1, T)
    """
    if wav.dim() == 2:
        wav = wav[0]  # (1, T) -> (T,)
    emb = rec.encode_batch(wav.unsqueeze(0)).squeeze(0)  # (dim,)
    emb = torch.nn.functional.normalize(emb, dim=-1)     # L2 normalize，方便直接內積當 cosine
    return emb

# ================== 註冊 & 登入（麥克風版） ==================

def enroll_user(rec: SpeakerRecognition, db: Dict[str, torch.Tensor]):
    name = input("輸入要註冊的語者名稱（例如: name）：").strip()
    if not name:
        print("名稱不能是空白，註冊失敗。")
        return

    if name in db:
        yn = input(f"名稱「{name}」已存在，要覆蓋舊的資料嗎？(y/N)：").strip().lower()
        if yn != "y":
            print("取消覆蓋，註冊失敗。")
            return

    try:
        n_str = input("要錄幾段語音？（建議 3~5，預設 3）：").strip()
        num_segments = int(n_str) if n_str else 3
    except ValueError:
        num_segments = 3

    num_segments = max(1, num_segments)
    embs: List[torch.Tensor] = []
    print(f"\n提示!! : 請清楚的朗讀以下建議語句，不須全部念完，但要盡量清晰且在較安靜的環境錄音。")
    for i in range(num_segments):
        print(f"\n--- 第 {i+1}/{num_segments} 段錄音 ---")
        print("建議唸的內容：")
        print(f"  「{SUGGEST_LINES_ENROLL[i % len(SUGGEST_LINES_ENROLL)]}」")
        try:
            wav = record_audio()
        except Exception as e:
            print(f"錄音失敗：{e}")
            continue
        emb = embed(rec, wav)
        embs.append(emb)

    if not embs:
        print("沒有成功錄到任何一段語音，註冊中止。")
        return

    spk_emb = torch.stack(embs, dim=0).mean(dim=0)
    spk_emb = torch.nn.functional.normalize(spk_emb, dim=-1)
    db[name] = spk_emb
    save_db(db)
    print(f"註冊完成：{name}（使用 {len(embs)} 段語音，已寫入資料庫）")

def verify_user(rec: SpeakerRecognition, db: Dict[str, torch.Tensor], threshold: float):
    if not db:
        print("資料庫是空的，請先註冊至少一位語者。")
        return

    print("\n--- 登入驗證 ---")
    print("請說出驗證語音 : 我正在進行語音識別登入 請確認我身分資訊。")
    try:
        wav = record_audio(duration=3.0)
    except Exception as e:
        print(f"錄音失敗：{e}")
        return

    q = embed(rec, wav)

    best_name = None
    best_score = -1.0
    all_scores = []

    for name, ref in db.items():
        score = torch.sum(q * ref).item()  # 內積 = cosine（因為已 L2N）
        all_scores.append((name, score))
        if score > best_score:
            best_name, best_score = name, score

    all_scores.sort(key=lambda x: x[1], reverse=True)

    print("=== Score 排序 ===")
    for n, s in all_scores:
        print(f"{n:>10s} : {s:.3f}")
    print("============================")

    if best_score >= threshold:
        print(f"通過：{best_name}（cos={best_score:.3f} ≥ 門檻 {threshold:.2f}）→ 門已開啟")
    else:
        print(f"拒絕：Unknown（最佳 {best_name} cos={best_score:.3f} < 門檻 {threshold:.2f}）→ 門鎖住")

# ================== 其他工具 ==================

def list_speakers(db: Dict[str, torch.Tensor]):
    if not db:
        print("資料庫目前沒有任何語者。")
        return
    print("目前已註冊語者：")
    for name in sorted(db.keys()):
        print(f" - {name}")

def set_threshold(cfg: dict):
    val = input(f"輸入新的門檻（目前 {cfg['threshold']:.2f}，建議 0.50~0.60）：").strip()
    try:
        threshold = float(val)
    except ValueError:
        print("門檻必須是數字，設定失敗。")
        return
    cfg["threshold"] = threshold
    save_cfg(cfg)
    print(f"已更新門檻 threshold = {cfg['threshold']:.2f}")

# ================== 主選單 ==================

def main_menu():
    print("載入模型中，第一次稍微久一點...") #（需下載 ECAPA 權重）
    rec = load_model()
    db = load_db()
    cfg = load_cfg()

    while True:
        print("\n=== 語音門禁  ===")
        print("1) 註冊（使用麥克風錄音）")
        print("2) 登入驗證（使用麥克風錄音）")
        print("3) 列出已註冊語者")
        print("4) 設定門檻")
        print("0) 離開")
        choice = input("請輸入選項：").strip()

        if choice == "1":
            enroll_user(rec, db)
        elif choice == "2":
            verify_user(rec, db, cfg["threshold"])
        elif choice == "3":
            list_speakers(db)
        elif choice == "4":
            set_threshold(cfg)
        elif choice == "0":
            print("bye !!")
            break
        else:
            print("無效選項，請重新輸入。")

if __name__ == "__main__":
    main_menu()
