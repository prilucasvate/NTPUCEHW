# 作業二：傳統與現代 NLP 方法 使用說明

## 作業說明

這次作業實作並比較了傳統 NLP 方法 (TF-IDF, 規則分類, 統計摘要)與現代 AI 方法 (GPT-4o API) 在文本相似度、文本分類和自動摘要，三個任務上的表現。

所有程式的比較結果可以由 `comparison.py` 程式自動生成並儲存在 `results/` 資料夾中。

## 檔案結構
```text
411286029_hw2/ 
├── .env (本地 API Key 不會上傳) 
├── .gitignore (忽略 .env) 
├── comparison.py (Part C 比較程式) 
├── modern_methods.py  (Part B AI 實作) 
├── traditional_methods.py (Part A 傳統實作)
├── report.md (Part C 分析報告) 
├── requirements.txt (Python 依賴套件) 
└── myresults/ (我測試的結果)
│   ├── ai_similarity_matrix.png  (AI 相似度熱圖)
│   ├── classification_results.csv (傳統、AI 分類比較)
│   ├── performance_metrics.json (傳統、AI 時間成本比較)
│   ├── summarization_comparison.txt (傳統、AI 摘要比較)
│   └── tfidf_similarity_matrix.png (TF-IDF 相似度熱圖)
└── results/ (comparison.py執行後的結果)
    ├── ai_similarity_matrix.png 
    ├── classification_results.csv 
    ├── performance_metrics.json 
    ├── summarization_comparison.txt 
    └── tfidf_similarity_matrix.png

```

## 執行方法 
### 1. 設定環境
**安裝依賴套件**

本專案需要 Python 3。執行以下指令安裝所有必要的套件 ：

```
pip3 install -r requirements.txt
```

**設定 API 金鑰**

本專案需要 OpenAI API key才能執行 Part B (AI 方法) 。  
在本專案的根目錄 (411286029_hw2/) 建立一個名為 .env 的檔案。  
在 .env 檔案中加入以下內容 (換成你自己的 API Key)：  
```
OPENAI_API_KEY="sk-YourApiKey"
```

### 2.執行比較程式
完成設定後，只需要執行 comparison.py
```
python3 comparison.py
```
程式會：
* 依序執行 Part A 和 Part B 的所有任務。
* 在終端機顯示進度條和測試結果。
* 在 results/ 資料夾中生成所有 5 個比較檔案

### 3. 查看分析報告
量化分析、質性分析報告請參閱 : report.md
相關數據存於 : results/

## 其他說明
可單獨執行、但會覆蓋 results/ 結果 :  
traditional_methods.py 為 Part A 實作  
modern_methods.py  為 Part B 實作  
comparison.py 為Part C 比較程式 會呼叫 Part A、B 方法  