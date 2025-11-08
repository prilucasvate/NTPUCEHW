# -*- coding: utf-8 -*-
"""411286029_hw2.ipynb

### 學習目標
在生成式 AI 蓬勃發展的今天，許多人直接使用 ChatGPT 等工具，卻不了解其背後的基礎原理。本作業將帶您：
1. 親手實作傳統 NLP 方法，理解文本處理的數學基礎。
2. 使用現代 AI 完成相同任務，體驗技術進步帶來的便利。
3. 深入比較兩種方法，培養選擇適當工具的判斷力。
4. 了解技術演進脈絡，建立扎實的 NLP 知識體系。
"""


"""## Part A: 傳統方法實作 (50分)
### A-1: TF-IDF 文本相似度計算 (20分)
任務說明：實作 TF-IDF 算法，並利用它來計算文本間的相似度。您需要：
1. 手動計算 TF-IDF (10分)
2. 使用 scikit-learn 實作 (5分)
3. 視覺化成果（計入分數）
"""

import jieba
import numpy as np
import pandas as pd
from collections import Counter
import math

# 測試資料
documents = [
    "人工智慧正在改變世界，機器學習是其核心技術",
    "深度學習推動了人工智慧的發展，特別是在圖像識別領域",
    "今天天氣很好，適合出去運動",
    "機器學習和深度學習都是人工智慧的重要分支",
    "運動有益健康，每天都應該保持運動習慣"
]

# 中文斷詞 jieba
tokenized_documents = [list(jieba.cut(doc)) for doc in documents]
print("斷詞結果:")
for i, doc in enumerate(tokenized_documents, 1):
    print(f"Document {i}: {doc}")

"""#### 1. 手動實作 TF-IDF"""

def calculate_tf(word_dict, total_words):
    """計算詞頻 (Term Frequency)
    Args:
        word_dict: 詞彙計數字典 (e.g., {'人工智慧': 2, '世界': 1})
        total_words: 該文件的總詞數
    Returns:
        tf_dict: TF 值字典
    """
    # TODO: 實作 TF 計算
    # 提示：TF = (該詞在文件中出現的次數) / (文件總詞數)
    #dict{k:v}
    return {word: cnt / total_words for word, cnt in word_dict.items()}
    raise NotImplementedError("請在此處完成 TF 計算")

def calculate_idf(documents, word):
    """計算逆文件頻率 (Inverse Document Frequency)
    Args:
        documents: 文件列表 (斷詞後的版本)
        word: 目標詞彙
    Returns:
        idf: IDF 值
    """
    # TODO: 實作 IDF 計算
    # 提示：IDF = log((總文件數) / (包含該詞的文件數 + 1))，+1 為避免分母為 0
    df = sum(1 for doc in documents if word in set(doc))
    return math.log(len(documents) / (df + 1))
    raise NotImplementedError("請在此處完成 IDF 計算")

def calculate_tfidf(tokenized_documents):
    """計算 TF-IDF 主函數
    回傳：pandas.DataFrame，列為文件，欄為詞彙
    """
    # TODO:
    # 1) 遍歷所有文件，計算每個文件的 TF
    # 2) 建立詞彙庫 (vocabulary)
    vocabulary = set(word for doc in tokenized_documents for word in doc) # a vocab set

    # 3) 對詞彙庫中的每個詞，計算其 IDF
    idf_dict = {word: calculate_idf(tokenized_documents, word) for word in vocabulary} # dict{word : idf_value}
    tfidf_matrix = []

    for doc in tokenized_documents:
        word_counts = Counter(doc) #each word count in doc
        total_words = len(doc) # total word in this doc
        tf_dict = calculate_tf(word_counts, total_words) # tf value
# 4) 結合 TF 和 IDF 計算每個文件中每個詞的 TF-IDF 值
        tfidf_dict = {word: tf_dict.get(word, 0) * idf_dict[word] for word in vocabulary} #tfidf_dict{word : tfidf} ; if target not found, tfidf = 0
        tfidf_matrix.append(tfidf_dict)
# 5) 回傳 TF-IDF 矩陣 (pandas DataFrame)
    return pd.DataFrame(tfidf_matrix)
    raise NotImplementedError("請在此處完成 TF-IDF 主流程")

# 範例：完成後可取消註解
tfidf_matrix = calculate_tfidf(tokenized_documents)
print("\n-------------- TF-IDF 矩陣 --------------\n")
print(tfidf_matrix.head())

"""#### 2. 使用 scikit-learn 實作"""

from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.metrics.pairwise import cosine_similarity

# TfidfVectorizer 以空格分隔的字串，所以我們先把斷詞結果接起來
processed_docs = [' '.join(doc) for doc in tokenized_documents]

# TODO: 使用 TfidfVectorizer 和 cosine_similarity 計算相似度矩陣
# 1) 初始化 TfidfVectorizer
vectorizer = TfidfVectorizer()
# 2) fit_transform 文本資料
tfidf_matrix_sklearn = vectorizer.fit_transform(processed_docs)
#print(tfidf_matrix_sklearn)
# 3) 使用 cosine_similarity 計算向量相似度
similarity_matrix = cosine_similarity(tfidf_matrix_sklearn)
print("\n-------------- A-1 相似度矩陣 --------------\n")
print(similarity_matrix)
#raise NotImplementedError("請完成：scikit-learn 的 TF-IDF 與相似度計算")

"""#### 3. 視覺化（熱圖）"""

import matplotlib.pyplot as plt
import seaborn as sns
import os
os.makedirs('results', exist_ok=True)

# just use english

plt.figure(figsize=(8, 6))
sns.heatmap(similarity_matrix, annot=True, cmap='viridis', xticklabels=range(1,6), yticklabels=range(1,6))
plt.title('Text Similarity Matrix (TF-IDF + Cosine Similarity)')
plt.xlabel('Document ID')
plt.ylabel('Document ID')
plt.savefig('results/tfidf_similarity_matrix.png', dpi=150, bbox_inches='tight')
plt.show()

"""### A-2: 基於規則的文本分類 (15分)
任務說明：建立規則式分類器，不使用機器學習，純粹基於關鍵詞和規則。
1. 情感分類器 (8分)
2. 主題分類器 (7分)
"""

# 測試資料
test_texts = [
    "這家餐廳的牛肉麵真的太好吃了，湯頭濃郁，麵條Q彈，下次一定再來！",
    "最新的AI技術突破讓人驚豔，深度學習模型的表現越來越好",
    "這部電影劇情空洞，演技糟糕，完全是浪費時間",
    "每天慢跑5公里，配合適當的重訓，體能進步很多"
]

"""#### 1. 情感分類器"""

import jieba
class RuleBasedSentimentClassifier:
    def __init__(self):
        # 建立正負面詞彙庫（已擴充）
        self.positive_words = ['好', '棒', '優秀', '喜歡', '推薦', '滿意', '開心', '值得', '精彩', '完美', '好吃', '濃郁', 'Q彈','驚豔', '進步']
        self.negative_words = ['差', '糟', '失望', '討厭', '不推薦', '浪費', '無聊', '爛', '糟糕', '差勁', '空洞']
        #self.negation_words = ['不', '沒', '無', '非', '別']
        #加權詞
        #利用乘法倍率調整
        self.multiplier_dict = {
            # 否定 (反轉)
            '不': -1, '沒': -1, '無': -1, '非': -1, '別': -1,
            # 強烈 (加權)
            '很': 1.5, '非常': 1.5, '太': 1.5, '超級': 2.0, '極度': 2.0,
            '真的': 1.5, '很多': 1.5,
            # 稍微 (減弱)
            '有點': 0.5, '稍稍': 0.5, '略': 0.5, '稍微': 0.5
        }

    def classify(self, text):
        """
        分類邏輯（請自行實作）：
        1) 計算正負詞數量
        2) 處理否定詞（否定 + 正面 → 轉負；否定 + 負面 → 轉正）
        3) （可選）程度副詞加權
        回傳：'正面' / '負面' / '中性'
        """
        #initial emotion score
        score = 0
        tokens = list(jieba.cut(text))

        for i, token in enumerate(tokens):

            base_score = 0

            # basic word
            if token in self.positive_words:
                base_score = 1
            elif token in self.negative_words:
                base_score = -1

            # if this word not target
            if base_score == 0:
                continue

            # multiplier
            multiplier = 1.0

            # previous word ("太" 好吃)
            if i > 0 and tokens[i-1] in self.multiplier_dict:
                multiplier *= self.multiplier_dict[tokens[i-1]]

                # The first two words ("不" "太" 好吃)
                if i > 1 and tokens[i-2] in self.multiplier_dict:
                    multiplier *= self.multiplier_dict[tokens[i-2]]

            # --- total score ---
            score += (base_score * multiplier)
        #print(f'--- 文本: "{text[:10]}..." | 原始分數: {score} ---')
        #final
        if score > 0:
            return '正面'
        elif score < 0:
            return '負面'
        else:
            return '中性'
        # TODO: 實作情感分類邏輯
        raise NotImplementedError("請完成情感分類器 classify()")

# 範例：完成後可取消註解
sentiment_classifier = RuleBasedSentimentClassifier()
print("\n-------------- A-2 基於規則的文本分類 --------------")
print("\n------- 情感分類結果 -------")
for text in test_texts:
  sentiment = sentiment_classifier.classify(text)
  print(f'文本: "{text[:20]}..." -> 情感: {sentiment}')

"""#### 2. 主題分類器"""

class TopicClassifier:
    def __init__(self):
        self.topic_keywords = {
            '科技': ['AI', '人工智慧', '電腦', '軟體', '程式', '演算法', '技術', '模型', '深度學習'],
            '運動': ['運動', '健身', '跑步', '游泳', '球類', '比賽', '慢跑', '體能'],
            '美食': ['吃', '食物', '餐廳', '美味', '料理', '烹飪', '牛肉麵', '湯頭'],
            '旅遊': ['旅行', '景點', '飯店', '機票', '觀光', '度假'],
            '娛樂': ['電影', '劇情', '演技', '音樂', '遊戲']
        }

    def classify(self, text):
        """返回最可能的主題（請實作關鍵詞計分）"""
        # TODO: 計算每個主題關鍵詞在文本中出現次數，回傳分數最高主題
        #e.g. {'科技': 3, '運動': 0, '美食': 1, '旅遊': 0, '娛樂': 0}
        topic_scores = {}
        #iterate all topic
        for topic, keywords in self.topic_keywords.items():
          score = 0

          # iterate all keyword
          for keyword in keywords:
            score += text.count(keyword) #a topic keyword in sentence
          topic_scores[topic] = score #save score
        if not topic_scores or all(s == 0 for s in topic_scores.values()):
          return '其他' # others

        best_topic = max(topic_scores, key=topic_scores.get)

        return best_topic
        raise NotImplementedError("請完成主題分類器 classify()")

# 範例：完成後可取消註解
topic_classifier = TopicClassifier()
print("\n-------- 主題分類結果 --------")
for text in test_texts:
  topic = topic_classifier.classify(text)
  print(f'文本: "{text[:20]}..." -> 主題: {topic}')

"""### 儲存A-2結果"""

import pandas as pd
import os



os.makedirs('results', exist_ok=True)

# 收集結果
traditional_results = []

# 執行兩分類
for text in test_texts:
    sentiment = sentiment_classifier.classify(text)
    topic = topic_classifier.classify(text)

    traditional_results.append({
        'method': 'Traditional (Rule-Based)',
        'text': text,
        'sentiment': sentiment,
        'topic': topic
    })

# 轉換成 DataFrame
df_traditional = pd.DataFrame(traditional_results)

# 儲存到 CSV
df_traditional.to_csv('results/classification_results.csv', index=False, encoding='utf-8-sig')

print("Part A-2 的分類結果已儲存到 'results/classification_results.csv'\n")
print(df_traditional)

"""### A-3: 統計式自動摘要 (15分)
任務說明：使用統計方法實作摘要系統，不依賴現代生成式 AI。
"""

# 測試文章（可自行替換）
article = (
    "人工智慧（AI）的發展正深刻改變我們的生活方式。從早上起床時的智慧鬧鐘，到通勤時的路線規劃，再到工作中的各種輔助工具，AI無處不在。\n"
    "在醫療領域，AI協助醫生進行疾病診斷，提高了診斷的準確率和效率。透過分析大量的醫療影像和病歷資料，AI能夠發現人眼容易忽略的細節，為患者提供更好的治療方案。\n"
    "教育方面，AI個人化學習系統能夠根據每個學生的學習進度和特點，提供客製化的教學內容。這種因材施教的方式，讓學習變得更加高效和有趣。\n"
    "然而，AI的快速發展也帶來了一些挑戰。首先是就業問題，許多傳統工作可能會被AI取代。其次是隱私和安全問題，AI系統需要大量數據來訓練，如何保護個人隱私成為重要議題。最後是倫理問題，AI的決策過程往往缺乏透明度，可能會產生偏見或歧視。\n"
    "面對這些挑戰，我們需要在推動AI發展的同時，建立相應的法律法規和倫理準則。只有這樣，才能確保AI技術真正為人類福祉服務，創造一個更美好的未來。\n"
)

from stopwordsiso import stopwords
import re
from collections import Counter
import jieba

class StatisticalSummarizer:
    def __init__(self):
        # 載入停用詞（繁體）
        self.stop_words = set(stopwords('traditional'))

    def _split_sentences(self, text):
        # 中文分句
        sents = re.split(r"[。！？\n]+", text)
        return [s.strip() for s in sents if s.strip()]
#句子重要性分數 我利用句子中每個詞出現在文章的頻率和句長平衡、首尾句加成、短句扣分來給分
    def sentence_score(self, sentence, word_freq, idx, n_sent):
        """計算句子重要性分數（請自行設計）
        可考慮：高頻詞數量、句子位置(首尾加權)、句長懲罰、是否含數字／專有名詞等
        """
        #intial score
        score = 0
        tokens = list(jieba.cut(sentence))

        if not tokens: # empty sentence
            return 0

        # 1. score sentence by  word frequence
        for word in tokens:
            if word in word_freq:
              #sum the all "total word count" in this sentence (by word fruq)
                score += word_freq[word]  # score : origin sentence score

        # 2. normalize for long sentence (all weights are just guess)
        #    0.8 ease the long sentence punish
        # avoid long sentence higher score
        if len(tokens) > 0:
            score = score / (len(tokens)**0.8)

        # 3. sentence position
        if idx == 0:
            # first sentence x 1.5
            score *= 1.5
        elif idx == (n_sent - 1):
            # last sentence x 1.2
            score *= 1.2

        # 4. short sentence
        if len(tokens) < 5:
            score *= 0.3 # sentence less than 5 word might not important

        return score

        # TODO: 實作句子評分邏輯
        raise NotImplementedError("請完成 sentence_score() 設計")

    def summarize(self, text, ratio=0.4):
        """
        生成摘要步驟建議：
        1) 分句
        2) 分詞並計算詞頻（過濾停用詞與標點）
        3) 計算每句分數
        4) 依 ratio 選取 Top-K 句子
        5) 依原文順序輸出摘要
        """
        # 1) spilt sentence
        sentences = self._split_sentences(text)
        n_sent = len(sentences) # total sentence count

        if n_sent == 0:
            return " (空文章)"

        # 2) spilt word
        punctuation = set("。！？，、；：「」（）《》\n \t") # symbol filter

        all_words = []
        for s in sentences:
            tokens = jieba.cut(s)
            for word in tokens:
                # filter words
                if word not in self.stop_words and word not in punctuation:
                    all_words.append(word) #all_words words list

        # caculate total word frequency
        word_freq = Counter(all_words)

        # 3) sentence score !! most important func
        sentence_scores = []
        for i, sentence in enumerate(sentences):
            # caculate each sentence score
            score = self.sentence_score(sentence, word_freq, i, n_sent)
            sentence_scores.append( (score, sentence, i) )
            # (score, sentence, index) need index to sort sentences later

        # 4) get Top-K sentence by ratio
        k = int(n_sent * ratio) # how many sentence to be left
        if k < 1:
            k = 1 # at least 1 sentence

        # sort by score , high -> low
        top_sentences = sorted(sentence_scores, key=lambda x: x[0], reverse=True)

        # get top-k sentence
        top_sentences = top_sentences[:k]

        # 5) sort by index
        top_sentences_sorted = sorted(top_sentences, key=lambda x: x[2])

        # concatenate the sentence and 。
        summary = "。".join([s[1] for s in top_sentences_sorted]) + "。"

        return summary
        # TODO: 實作摘要主流程
        raise NotImplementedError("請完成 summarize() 主流程")

# 範例：完成後可取消註解
summarizer = StatisticalSummarizer()
summary = summarizer.summarize(article, ratio=0.7)
print("\n---------- A-3 統計式摘要結果 ----------")
print("原文長度:", len(article))
print("摘要內容:\n", summary)

os.makedirs('results', exist_ok=True)

# 寫入新檔案
with open('results/summarization_comparison.txt', 'w', encoding='utf-8') as f:
    f.write("========== Part A: Traditional Summary ==========\n")
    f.write(f"原文長度: {len(article)}\n")
    f.write(f"摘要:\n{summary}\n\n")

print("\nPart A-3 摘要結果已儲存到 'results/summarization_comparison.txt'")
