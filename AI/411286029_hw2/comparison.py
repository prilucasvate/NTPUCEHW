
import os
import json
import time
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import tiktoken
from tqdm.auto import tqdm
import jieba

# 現代、AI 方法
import traditional_methods as tm
import modern_methods as mm


from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.metrics.pairwise import cosine_similarity

print("=== Part C 完整比較 ===")

# --- 儲存資訊 ---
os.makedirs('results', exist_ok=True)
performance_data = {
    'similarity': {},
    'classification': {},
    'summarization': {}
}

# --- 測試資料 ---
documents = [
    "人工智慧正在改變世界，機器學習是其核心技術",
    "深度學習推動了人工智慧的發展，特別是在圖像識別領域",
    "今天天氣很好，適合出去運動",
    "機器學習和深度學習都是人工智慧的重要分支",
    "運動有益健康，每天都應該保持運動習慣"
]
test_texts = [
    "這家餐廳的牛肉麵真的太好吃了，湯頭濃郁，麵條Q彈，下次一定再來！",
    "最新的AI技術突破讓人驚豔，深度學習模型的表現越來越好",
    "這部電影劇情空洞，演技糟糕，完全是浪費時間",
    "每天慢跑5公里，配合適當的重訓，體能進步很多"
]
article = (
    "人工智慧（AI）的發展正深刻改變我們的生活方式。從早上起床時的智慧鬧鐘，到通勤時的路線規劃，再到工作中的各種輔助工具，AI無處不在。\n"
    "在醫療領域，AI協助醫生進行疾病診斷，提高了診斷的準確率和效率。透過分析大量的醫療影像和病歷資料，AI能夠發現人眼容易忽略的細節，為患者提供更好的治療方案。\n"
    "教育方面，AI個人化學習系統能夠根據每個學生的學習進度和特點，提供客製化的教學內容。這種因材施教的方式，讓學習變得更加高效和有趣。\n"
    "然而，AI的快速發展也帶來了一些挑戰。首先是就業問題，許多傳統工作可能會被AI取代。其次是隱私和安全問題，AI系統需要大量數據來訓練，如何保護個人隱私成為重要議題。最後是倫理問題，AI的決策過程往往缺乏透明度，可能會產生偏見或歧視。\n"
    "面對這些挑戰，我們需要在推動AI發展的同時，建立相應的法律法規和倫理準則。只有這樣，才能確保AI技術真正為人類福祉服務，創造一個更美好的未來。\n"
)

# --- 相似度比較 A-1 vs B-1 ---
print("\n[任務 1/4] 相似度比較...")

# A-1 (Traditional)
tokenized_documents = [list(jieba.cut(doc)) for doc in documents]
processed_docs = [' '.join(doc) for doc in tokenized_documents]
vectorizer = TfidfVectorizer()
tfidf_matrix_sklearn = vectorizer.fit_transform(processed_docs)
similarity_matrix = cosine_similarity(tfidf_matrix_sklearn)
plt.figure(figsize=(8, 6))
sns.heatmap(similarity_matrix, annot=True, cmap='viridis', fmt='.2f', xticklabels=range(1,6), yticklabels=range(1,6))
plt.title('A-1: Text Similarity Matrix (TF-IDF + Cosine Similarity)')
plt.xlabel('Document ID')
plt.ylabel('Document ID')
plt.savefig('results/tfidf_similarity_matrix.png', dpi=150, bbox_inches='tight')
print("A-1 熱圖已儲存到 'results/tfidf_similarity_matrix.png'")

# B-1 (AI)
n_docs = len(documents)
ai_similarity_matrix = np.zeros((n_docs, n_docs), dtype=float)
for i in tqdm(range(n_docs), desc="B-1 (AI) Similarity"):
    for j in range(i, n_docs):
        if i == j:
            score = 100.0
        else:
            score = mm.ai_similarity(documents[i], documents[j]) # 呼叫 modern_methods
        ai_similarity_matrix[i, j] = score / 100.0
        ai_similarity_matrix[j, i] = score / 100.0
ai_sim_df = pd.DataFrame(ai_similarity_matrix, index=range(1, n_docs + 1), columns=range(1, n_docs + 1))
plt.figure(figsize=(8, 6))
sns.heatmap(ai_sim_df, annot=True, cmap='viridis', fmt='.2f', xticklabels=range(1,6), yticklabels=range(1,6))
plt.title('B-1: AI Text Similarity Matrix (GPT-4o)')
plt.xlabel('Document ID')
plt.ylabel('Document ID')
plt.savefig('results/ai_similarity_matrix.png', dpi=150, bbox_inches='tight')
print("B-1 熱圖已儲存到 'results/ai_similarity_matrix.png'")

# --- 分類比較 (A-2 vs B-2) ---
print("\n[任務 2/4] 分類比較...")
sentiment_classifier = tm.RuleBasedSentimentClassifier()
topic_classifier = tm.TopicClassifier()
results_list = []

for text in tqdm(test_texts, desc="A-2 (Traditional) Classify"):
    results_list.append({
        'method': 'Traditional (Rule-Based)',
        'text': text,
        'sentiment': sentiment_classifier.classify(text), # 呼叫 traditional_methods
        'topic': topic_classifier.classify(text)
    })
for text in tqdm(test_texts, desc="B-2 (AI) Classify"):
    result_dict = mm.ai_classify(text) # 呼叫 modern_methods
    results_list.append({
        'method': 'Modern (AI GPT-4o)',
        'text': text,
        'sentiment': result_dict.get('sentiment', '錯誤'),
        'topic': result_dict.get('topic', '錯誤')
    })

df_combined = pd.DataFrame(results_list)
df_combined.to_csv('results/classification_results.csv', index=False, encoding='utf-8-sig')
print("A-2 & B-2 分類結果已儲存到 'results/classification_results.csv'")

# --- 摘要比較 (A-3 vs B-3) ---
print("\n[任務 3/4] 正在執行摘要比較...")
summarizer = tm.StatisticalSummarizer()
traditional_summary = summarizer.summarize(article, ratio=0.7) # 呼叫 traditional_methods
ai_summary = mm.ai_summarize(article, max_length=150) # 呼叫 modern_methods

comparison_content = f"""
================================
傳統統計式摘要
================================
(原文長度: {len(article)} 字, 摘要長度: {len(traditional_summary)} 字)

{traditional_summary}

================================
現代 AI 摘要
================================
(原文長度: {len(article)} 字, 摘要長度: {len(ai_summary)} 字)

{ai_summary}
"""
with open('results/summarization_comparison.txt', 'w', encoding='utf-8') as f:
    f.write(comparison_content.strip())
print("A-3 & B-3 摘要比較已儲存到 'results/summarization_comparison.txt'")

# --- 效能與成本 (C-1) ---
print("\n[任務 4/4] 效能與成本分析...")
num_runs = 5 # 執行 5 次取平均

# 5.1 時間
tqdm.write(f"--- 開始測試 每項測試執行 {num_runs} 次後平均 ---")
# --- 相似度 ---
total_time_a1 = 0
for _ in range(num_runs):
    start_time = time.perf_counter()
    _ = cosine_similarity(tfidf_matrix_sklearn) 
    total_time_a1 += (time.perf_counter() - start_time)
performance_data['similarity']['traditional_time_sec'] = total_time_a1 / num_runs

total_time_b1 = 0
for _ in range(num_runs):
    start_time = time.perf_counter()
    for i in range(n_docs):
        for j in range(i + 1, n_docs):
            _ = mm.ai_similarity(documents[i], documents[j]) # 10 次 (n_docs=5 -> 4+3+2+1=10)
    total_time_b1 += (time.perf_counter() - start_time)
performance_data['similarity']['ai_time_sec'] = total_time_b1 / num_runs
tqdm.write("Similarity time test Done.")
# --- 分類 ---
total_time_a2 = 0
for _ in range(num_runs):
    start_time = time.perf_counter()
    for text in test_texts:
        _ = sentiment_classifier.classify(text)
        _ = topic_classifier.classify(text)
    total_time_a2 += (time.perf_counter() - start_time)
performance_data['classification']['traditional_time_sec'] = total_time_a2 / num_runs

total_time_b2 = 0
for _ in range(num_runs):
    start_time = time.perf_counter()
    for text in test_texts:
        _ = mm.ai_classify(text) # 4 次
    total_time_b2 += (time.perf_counter() - start_time)
performance_data['classification']['ai_time_sec'] = total_time_b2 / num_runs
tqdm.write("Classification time test Done.")
# --- 摘要 ---
total_time_a3 = 0
for _ in range(num_runs):
    start_time = time.perf_counter()
    _ = summarizer.summarize(article, ratio=0.4)
    total_time_a3 += (time.perf_counter() - start_time)
performance_data['summarization']['traditional_time_sec'] = total_time_a3 / num_runs

total_time_b3 = 0
for _ in range(num_runs):
    start_time = time.perf_counter()
    _ = mm.ai_summarize(article, max_length=150) 
    total_time_b3 += (time.perf_counter() - start_time)
performance_data['summarization']['ai_time_sec'] = total_time_b3 / num_runs
tqdm.write("Summarization time test Done.")
# --- B-1 成本 ---
encoding = tiktoken.get_encoding("o200k_base")
PRICE_INPUT_PER_1M = 2.50  # gpt-4o 價格
PRICE_OUTPUT_PER_1M = 10.00 

# B-1 成本
total_input_tokens_b1, total_output_tokens_b1 = 0, 0
n_calls_b1 = 0
B1_SYSTEM_PROMPT = "你是一個專業的中文語意分析助手。你的唯一任務是評估兩段文字的語意相似度。"
B1_USER_PROMPT_TEMPLATE = """
請評估以下兩段文字的語意相似度：
文字1：{text1}
文字2：{text2}
請「只」回傳一個 0 到 100 之間的「整數」，代表相似度百分比。不要有任何多餘的解釋或文字。
"""

for i in range(n_docs):
    for j in range(i + 1, n_docs):
        n_calls_b1 += 1
        prompt = B1_USER_PROMPT_TEMPLATE.format(text1=documents[i], text2=documents[j])
        total_input_tokens_b1 += len(encoding.encode(B1_SYSTEM_PROMPT))
        total_input_tokens_b1 += len(encoding.encode(prompt))
        total_output_tokens_b1 += 2 # 分數估 2 token
cost_b1 = (total_input_tokens_b1 * PRICE_INPUT_PER_1M / 1_000_000) + (total_output_tokens_b1 * PRICE_OUTPUT_PER_1M / 1_000_000)
performance_data['similarity']['ai_cost_usd'] = cost_b1


tqdm.write("Cost analysis (B-1) Done.")

# 儲存
json_path = 'results/performance_metrics.json'
with open(json_path, 'w', encoding='utf-8') as f:
    json.dump(performance_data, f, ensure_ascii=False, indent=4)
print(f"效能與成本數據已儲存到 '{json_path}'")

print("\n=== 完整比較 ===")