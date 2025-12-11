import warnings
# 過濾警告
warnings.filterwarnings("ignore", message="SpeechBrain ")
warnings.filterwarnings("ignore", category=FutureWarning)
warnings.filterwarnings("ignore", module="speechbrain")
warnings.filterwarnings("ignore", module="torch")
warnings.filterwarnings("ignore", message="FP16 is not supported on CPU; using FP32 instead")

import os
import json
import time
import shutil
import random
import difflib
from typing import Dict, List
from pathlib import Path

import torch
import torchaudio
import torch.nn as nn
import sounddevice as sd
import numpy as np
import whisper

# ================== Fix Windows (相容性修正) ==================

if not hasattr(torchaudio, "list_audio_backends"):
    def _fake_list_audio_backends(): return ["sox_io"]
    torchaudio.list_audio_backends = _fake_list_audio_backends

if not hasattr(torchaudio, "get_audio_backend"):
    def _fake_get_audio_backend(): return "sox_io"
    torchaudio.get_audio_backend = _fake_get_audio_backend

import speechbrain.utils.fetching as sb_fetching
def _safe_link_with_strategy(src, dst, local_strategy=None):
    src = Path(src)
    dst = Path(dst)
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)
    return dst
sb_fetching.link_with_strategy = _safe_link_with_strategy

from speechbrain.inference import SpeakerRecognition
if not hasattr(nn, "Dropout1d"): nn.Dropout1d = nn.Dropout

# ================== 基本設定 ==================

DB_PATH = "db/speakers_db.pt"       
CFG_PATH = "db/config.json"         
DEFAULT_THRESHOLD = 0.55            
SAMPLE_RATE = 16000                 

SUGGEST_LINES_ENROLL = [
    "你好，我正在測試語音辨識系統，請聽清楚我的聲音。",
    "我希望這個模型能正確記住我，辨識出我的聲音特色。",
    "這是一段普通的描述，用來讓模型學習我的聲音特徵。",
    "每個人都有獨特的頻率組成，這段話能更準確辨識我。",
    "明亮的燈光映照在寧靜的房間裡，我緩緩地說著這段話。"
]

CHALLENGE_SENTENCES = [
    "人工智慧期末專題一定會高分通過",
    "拜託程式碼不要再出現任何錯誤了",
    "最近特別想吃學校附近的小籠包",
    "希望我的期末考試都能順利通過",
    "希望我在展示時不要出現意外狀況",
    "湖邊的那隻鵝今天看起來特別好吃",
    "這裡的天氣真的不適合人類生存",
    "又要過完一年了時間過得真快"
]

# ================== 模型與工具 ==================

def ensure_dirs():
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)

def load_model():
    print("正在載入聲紋辨識模型 (ECAPA-TDNN)...")
    model_dir = os.path.join(os.path.expanduser("~"), "sb_pretrained_ecapa")
    os.makedirs(model_dir, exist_ok=True)
    return SpeakerRecognition.from_hparams(
        source="speechbrain/spkrec-ecapa-voxceleb",
        savedir=model_dir,
        run_opts={"device": "cpu"},
    )

def load_asr_model():
    print("正在載入語音轉文字模型 (Whisper)...")
    return whisper.load_model("base", device="cpu")

def load_db() -> Dict[str, torch.Tensor]:
    if os.path.exists(DB_PATH): return torch.load(DB_PATH)
    return {}

def save_db(db):
    ensure_dirs()
    torch.save(db, DB_PATH)

def load_cfg() -> dict:
    if os.path.exists(CFG_PATH):
        with open(CFG_PATH, "r", encoding="utf-8") as f: return json.load(f)
    return {"threshold": DEFAULT_THRESHOLD}

def save_cfg(cfg):
    ensure_dirs()
    with open(CFG_PATH, "w", encoding="utf-8") as f: json.dump(cfg, f, indent=2)

def beep(freq=600, duration=0.25, sr=16000):
    t = np.linspace(0, duration, int(sr * duration), endpoint=False)
    wave = 0.2 * np.sin(2 * np.pi * freq * t)
    sd.play(wave, samplerate=sr)
    sd.wait()
def play_success_sound():
    """成功聲調"""
    beep(1046, 0.25)

def play_fail_sound():
    """失敗低音"""
    beep(411, 0.4)

# 靜音切除功能 (提升分數)
def trim_silence(wav: torch.Tensor, threshold=0.015, pad_samples=2000) -> torch.Tensor:
    """
    切除錄音檔頭尾的靜音，只保留人聲部分。
    threshold: 音量門檻 (0.015 適合一般安靜房間)
    pad_samples: 留一點緩衝 (2000 samples 約 0.125秒)
    """
    if wav.dim() == 2:
        wav_flat = wav.squeeze(0) # (T,)
    else:
        wav_flat = wav

    # 計算振幅
    amplitude = torch.abs(wav_flat)
    
    # 找出大於門檻的索引
    mask = amplitude > threshold
    indices = torch.nonzero(mask).squeeze()

    if indices.numel() == 0:
        return wav # 完全沒聲音，回傳原本的

    start = max(0, indices[0].item() - pad_samples)
    end = min(len(wav_flat), indices[-1].item() + pad_samples)

    # 如果切完太短(小於0.5秒)，可能切錯了，回傳原本的
    if (end - start) < 8000: 
        return wav

    return wav[:, start:end]

def record_audio(duration: float = 5.0, sr: int = SAMPLE_RATE, need_enter: bool = True) -> torch.Tensor:
    num_samples = int(duration * sr)
    
    if need_enter:
        input(f"\n>>> 按 Enter 鍵開始錄音 ({int(duration)}秒)...")

    # 倒數
    print("3...", end=" ", flush=True)
    time.sleep(1.0)
    print("2...", end=" ", flush=True)
    time.sleep(1.0)
    print("1...", end=" ", flush=True)
    time.sleep(1.0)
    
    beep() 
    print("\n 錄音中...")

    audio = sd.rec(num_samples, samplerate=sr, channels=1, dtype="float32")
    sd.wait()
    print(" 錄音結束。")

    wav = torch.from_numpy(audio.squeeze(1)).unsqueeze(0)
    return wav

@torch.no_grad()
def embed(rec: SpeakerRecognition, wav: torch.Tensor) -> torch.Tensor:
    if wav.dim() == 2: wav = wav[0]
    emb = rec.encode_batch(wav.unsqueeze(0)).squeeze(0)
    return torch.nn.functional.normalize(emb, dim=-1)

# ================== 核心功能 ==================

def enroll_user(rec, db):
    print("\n--- 註冊模式 ---")
    name = input("輸入要註冊的語者名稱（例如: wali) : ").strip()
    if not name:
        print("名稱不能是空白。")
        return

    if name in db:
        if input(f"名稱「{name}」已存在，覆蓋嗎？(y/N) : ").lower() != "y": return

    embs = []
    print(f"\n請錄製 3 段語音以建立聲紋模型。")
    
    for i in range(3):
        print(f"\n--- 第 {i+1}/3 段 ---")
        line = SUGGEST_LINES_ENROLL[i % len(SUGGEST_LINES_ENROLL)]
        print(f"請唸出 ： 「{line}」")
        
        try:
            # 註冊時固定錄 5 秒，並手動 Enter
            wav = record_audio(duration=5.0, need_enter=True)
            # 註冊也要切除靜音，建立更純淨的模型
            wav = trim_silence(wav) 
        except Exception as e:
            print(f"Error: {e}")
            continue
        
        embs.append(embed(rec, wav))

    if not embs: return

    spk_emb = torch.stack(embs, dim=0).mean(dim=0)
    db[name] = torch.nn.functional.normalize(spk_emb, dim=-1)
    save_db(db)
    print(f"註冊完成！已儲存 {name} 的資料。")

# ---------- 驗證功能 ----------
def verify_user(rec, asr_model, db, threshold):
    if not db:
        print("資料庫是空的，請先註冊。")
        return

    challenge_text = random.choice(CHALLENGE_SENTENCES)
    
    # [優化 1] 動態計算錄音時間
    # 每個字給 0.3 秒，加上 0.5 秒緩衝 (反應時間 + 尾音)
    # 最短不小於 2 秒，最長不超過 8 秒
    calc_duration = len(challenge_text) * 0.3 + 0.5
    final_duration = max(2.0, min(calc_duration, 8.0))

    print("\n================ 登入驗證 ================")
    print(f"請唸出辨識文字： 「{challenge_text}」")
    
    try:
        # 這裡 need_enter=True 讓你準備好再唸
        wav = record_audio(duration=final_duration, need_enter=True)
    except Exception as e:
        print(f"錄音失敗：{e}")
        return

    # 聲紋辨識前，先切除靜音 (大幅提升分數)
    # 保留原始 wav 給 ASR 用 (Whisper 對靜音比較沒差)
    wav_clean = trim_silence(wav) 

    # 1. Whisper ASR
    print("正在辨識內容...")
    audio_np = wav.squeeze().numpy()
    result = asr_model.transcribe(audio_np, language="zh", fp16=False)
    transcribed_text = result["text"]
    print(f"系統聽到：{transcribed_text}")

    # 內容比對
    def clean(t): return t.replace(",", "").replace("，", "").replace("。", "").replace(" ", "").replace(".", "")
    similarity = difflib.SequenceMatcher(None, clean(challenge_text), clean(transcribed_text)).ratio()
    
    if similarity < 0.6:
        play_fail_sound()
        print(f"內容驗證失敗！ (相似度 {similarity*100:.0f}%)，請照著唸。")
        print("\n>>> 門鎖保持關閉 (Access Denied) <<<")
        return
    else:
        print("內容正確，檢查聲紋...")

    # 2. 聲紋比對 (使用乾淨的 wav_clean)
    q = embed(rec, wav_clean)
    best_name, best_score = None, -1.0

    for name, ref in db.items():
        score = torch.sum(q * ref).item()
        if score > best_score: best_name, best_score = name, score

    print(f"最佳匹配：{best_name} (相似度: {best_score:.3f})")
    
    if best_score >= threshold:
        play_success_sound()
        print(f"驗證成功！歡迎，{best_name}。")
        print("\n>>> 門鎖已開啟 (Door Opened) <<<")
    else:
        play_fail_sound()
        print(f"聲紋不符 (未達門檻 {threshold})。")
        print("\n>>> 門鎖保持關閉 (Access Denied) <<<")

# ---------- 列出語者 ----------
def list_speakers(db: Dict[str, torch.Tensor]):
    if not db:
        print("資料庫目前沒有任何語者。")
        return
    print("目前已註冊語者：")
    for name in sorted(db.keys()):
        print(f" - {name}")
# ================== 主程式 ==================

def main_menu():
    rec = load_model()
    asr_model = load_asr_model()
    db = load_db()
    cfg = load_cfg()

    while True:
        print(f"\n=== 智慧語音門禁系統 (AI Voice Access) ===")
        print("1. 註冊使用者")
        print("2. 登入驗證")
        print("3. 查看使用者列表")
        print("4. 設定門檻")
        print("0. 離開")
        
        choice = input("請選擇功能：").strip()

        if choice == "1": enroll_user(rec, db)
        elif choice == "2": verify_user(rec, asr_model, db, cfg["threshold"])
        elif choice == "3": list_speakers(db)
        elif choice == "4": 
            try:
                print("目前門檻：", cfg["threshold"])
                cfg["threshold"] = float(input("新門檻："))
                save_cfg(cfg)
            except: pass
        elif choice == "0": 
            print("再見 Bye!") 
            break
        else:
            print("無效輸入。 請重新輸入。")

if __name__ == "__main__":
    main_menu()