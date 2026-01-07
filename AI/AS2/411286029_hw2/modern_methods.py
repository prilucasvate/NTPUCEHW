"""## Part B: 現代 AI 方法 (30分)

任務說明：使用 OpenAI API 完成相同的任務。**請勿把金鑰硬編碼在程式中**。
"""


import os
import json
import openai
from dotenv import load_dotenv
from getpass import getpass
import numpy as np
import pandas as pd
from tqdm.auto import tqdm # 進度條
import matplotlib.pyplot as plt
import seaborn as sns

load_dotenv() # 載入 .env 的API KEY

try:
    import openai
except Exception as e:
    print("請先安裝 openai 套件。")

# 建議使用環境變數或 getpass
api_key = os.environ.get("OPENAI_API_KEY")# put api key here

try:
    client = openai.OpenAI(api_key=api_key)
    print(" OpenAI client initialized successfully.\n")
except Exception as e:
    print(f" Error initializing OpenAI client: {e}\n")

# ==============================================================================
# B-1: 語意相似度計算
# ==============================================================================

def ai_similarity(text1, text2):
    """使用 OpenAI 模型判斷語意相似度
    要求：
    1) 設計適當 prompt
    2) 返回 0-100 的相似度分數（整數）
    3) 處理 API 錯誤
    """
    SYSTEM_PROMPT = "你是一個專業的中文語意分析助手。你的唯一任務是評估兩段文字的語意相似度。"
    USER_PROMPT = f"""
請評估以下兩段文字的語意相似度：

文字1：{text1}
文字2：{text2}

請「只」回傳一個 0 到 100 之間的「整數」，代表相似度百分比。不要有任何多餘的解釋或文字。
"""
    try:
        response = client.chat.completions.create(
            model="gpt-4o",
            messages=[
                {"role": "system", "content": SYSTEM_PROMPT},
                {"role": "user", "content": USER_PROMPT}
            ],
            temperature=0  # 設 0，讓 AI 穩定輸出
        )

        # get response from answer
        raw_response = response.choices[0].message.content

        # change to int
        similarity_score = int(raw_response.strip())

        return similarity_score

    except Exception as e:
        # API error
        print(f"呼叫相似度 API 時錯誤: {e}")
        return -1
    # TODO: 呼叫 OpenAI API，解析回傳結果並處理可能的錯誤
    # 提示: 使用 try-except 捕捉錯誤；回傳的結果轉為 int
    raise NotImplementedError("請完成 ai_similarity() 的 API 呼叫與解析")

# 測試資料
text_a = "人工智慧是未來科技的趨勢"
text_b = "機器學習引領了AI的發展"
text_c = "今天天氣真好"

# 範例：完成後可取消註解
score1 = ai_similarity(text_a, text_b)
score2 = ai_similarity(text_a, text_c)
print("--- AI 語意相似度計算 ---")
print(f'“{text_a}” 和 “{text_b}” 的相似度: {score1}')
print(f'“{text_a}” 和 “{text_c}” 的相似度: {score2}')



# 文章
documents = [
    "人工智慧正在改變世界，機器學習是其核心技術",
    "深度學習推動了人工智慧的發展，特別是在圖像識別領域",
    "今天天氣很好，適合出去運動",
    "機器學習和深度學習都是人工智慧的重要分支",
    "運動有益健康，每天都應該保持運動習慣"
]

n_docs = len(documents)

# 相似度矩陣
ai_similarity_matrix = np.zeros((n_docs, n_docs), dtype=float)

print(f"\n--- B-1: AI 相似度矩陣 ({n_docs}x{n_docs}) ---")

for i in tqdm(range(n_docs), desc="Calculating Rows"):
    for j in range(i, n_docs):

        if i == j:
            # 自己相似度滿分
            score = 100.0
        else:
            # 呼叫 B-1 AI方法
            score = ai_similarity(documents[i], documents[j])

        # 填入矩陣
        ai_similarity_matrix[i, j] = score / 100.0
        ai_similarity_matrix[j, i] = score / 100.0


# 標籤設為 1~5
ai_sim_df = pd.DataFrame(ai_similarity_matrix,
                         index=range(1, n_docs + 1),
                         columns=range(1, n_docs + 1))
print(ai_sim_df)

# 畫出 AI 矩陣的熱圖
plt.figure(figsize=(8, 6))
# 小數
sns.heatmap(ai_sim_df, annot=True, cmap='viridis', fmt='.2f')
plt.title('AI Text Similarity Matrix (GPT-4o)')
plt.xlabel('Document ID')
plt.ylabel('Document ID')
# 存檔
os.makedirs('results', exist_ok=True)
plt.savefig('results/ai_similarity_matrix.png', dpi=150, bbox_inches='tight')
plt.show()
print("已儲存到 'results/ai_similarity_matrix.png'\n")

# ==============================================================================
# B-2: AI 文本分類
# ==============================================================================

# 測試資料
test_texts = [
    "這家餐廳的牛肉麵真的太好吃了，湯頭濃郁，麵條Q彈，下次一定再來！",
    "最新的AI技術突破讓人驚豔，深度學習模型的表現越來越好",
    "這部電影劇情空洞，演技糟糕，完全是浪費時間",
    "每天慢跑5公里，配合適當的重訓，體能進步很多"
]
import json

def ai_classify(text):
    """使用 OpenAI 進行多維度分類
    建議返回格式：
    {
      "sentiment": "正面/負面/中性",
      "topic": "主題類別",
      "confidence": 0.95
    }
    """
    SYSTEM_PROMPT = """
你是一個專業的中文文本分類器。你的任務是分析使用者提供的文本，並「只」回傳一個 JSON 物件。
這個 JSON 物件「必須」包含 'sentiment', 'topic', 'confidence' 這三個鍵。
"""

    USER_PROMPT = f"""
請分析以下文本：
    {text}
    請根據以下 JSON 格式回傳你的分析結果，主題類別請自行判斷（例如：科技、美食、娛樂、運動等）：
{{
  "sentiment": "正面/負面/中性",
  "topic": "主題類別",
  "confidence": 0.0
}}
"""
    # 2. 處理 API 錯誤
    try:
        response = client.chat.completions.create(
            model="gpt-4o",
            # JSON Mode
            response_format={"type": "json_object"},

            messages=[
                {"role": "system", "content": SYSTEM_PROMPT},
                {"role": "user", "content": USER_PROMPT}
            ],
            temperature=0
        )

        # 4. 解析回傳的 JSON 字串
        raw_json_string = response.choices[0].message.content

        # 5. 使用 json.loads() 轉成 Python 字典
        result_dict = json.loads(raw_json_string)

        return result_dict

    except Exception as e:
        print(f"呼叫 AI 分類 API 時發生錯誤: {e}")
        # 如果出錯，回傳一個錯誤字典
        return {"sentiment": "錯誤", "topic": "錯誤", "confidence": 0.0}
    # TODO: 設計 prompt，呼叫 API，並解析回傳 JSON
    # 提示：在 prompt 明確要求模型回傳 JSON 字串，再用 json.loads() 解析
    raise NotImplementedError("請完成 ai_classify() 的 API 呼叫與解析")

# 範例：完成後可取消註解
print("--- B-2: AI 文本分類 ---")
for text in test_texts:
    result = ai_classify(text)
    print(f'文本: "{text[:20]}..." -> 分類結果: {result}')

# ==============================================================================
# B-3: AI 自動摘要
# ==============================================================================
# 測試文章（可自行替換）
article = (
    "人工智慧（AI）的發展正深刻改變我們的生活方式。從早上起床時的智慧鬧鐘，到通勤時的路線規劃，再到工作中的各種輔助工具，AI無處不在。\n"
    "在醫療領域，AI協助醫生進行疾病診斷，提高了診斷的準確率和效率。透過分析大量的醫療影像和病歷資料，AI能夠發現人眼容易忽略的細節，為患者提供更好的治療方案。\n"
    "教育方面，AI個人化學習系統能夠根據每個學生的學習進度和特點，提供客製化的教學內容。這種因材施教的方式，讓學習變得更加高效和有趣。\n"
    "然而，AI的快速發展也帶來了一些挑戰。首先是就業問題，許多傳統工作可能會被AI取代。其次是隱私和安全問題，AI系統需要大量數據來訓練，如何保護個人隱私成為重要議題。最後是倫理問題，AI的決策過程往往缺乏透明度，可能會產生偏見或歧視。\n"
    "面對這些挑戰，我們需要在推動AI發展的同時，建立相應的法律法規和倫理準則。只有這樣，才能確保AI技術真正為人類福祉服務，創造一個更美好的未來。\n"
)
def ai_summarize(text, max_length=100):
    """使用 OpenAI 生成摘要
    要求：
    1) 可控制摘要長度
    2) 保留關鍵資訊
    3) 語句通順
    """

    """使用 OpenAI 生成摘要"""

    # 1.設計 Prompt (系統指令 + 使用者指令)
    SYSTEM_PROMPT = "你是一個專業的中文文本摘要助手。你的任務是閱讀使用者提供的長篇文章，並生成一份流暢、精準、保留核心資訊的摘要。"

    # 我們把 max_length 變數 和要求 都放進 prompt
    USER_PROMPT = f"""
請將以下文章摘要成一篇「大約 {max_length} 字」的摘要。

要求：
1. 摘要必須語句通順。
2. 摘要必須保留文章的核心論點和關鍵資訊。
3. 摘要的長度應盡量接近 {max_length} 字，不要過長或過短。

文章：
{text}
    請直接回傳摘要內容，不要包含「這是一篇摘要：」或任何多餘的開頭。
"""

    try:
        response = client.chat.completions.create(
            model="gpt-4o",
            messages=[
                {"role": "system", "content": SYSTEM_PROMPT},
                {"role": "user", "content": USER_PROMPT}
            ],
            temperature=0.3  # 讓 AI 能流暢重組句子
        )

        # 解析回傳結果
        summary_text = response.choices[0].message.content.strip()

        return summary_text

    except Exception as e:
        print(f"呼叫 摘要 API 時發生錯誤: {e}")
        return "AI 摘要失敗。"

    # TODO: 設計 prompt，呼叫 API，並回傳摘要結果
    raise NotImplementedError("請完成 ai_summarize() 的 API 呼叫與解析")

# 範例：完成後可取消註解
print("\n--- B-3: AI 自動摘要 ---")
ai_summary_text = ai_summarize(article, max_length=150)
print("原文長度:", len(article))
print("摘要長度:", len(ai_summary_text))
print("AI 摘要內容\n:", ai_summary_text)

