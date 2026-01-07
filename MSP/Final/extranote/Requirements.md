# MMSP Final Project 2025

* 基礎知識參考:
    * **重要的參考書籍 pdf 檔，請必看: https://burg.cs.wfu.edu/TheScienceOfDigitalMedia/Chapter3/Ch3ScienceOfDigitalMedia.pdf** 。
    * https://github.com/cychiang-ntpu/ntpu-ce-mmsp-2023/tree/master/Chapter-5 ：
        * 簡單來講就是把 matlab code 改成 C:
            *   https://github.com/cychiang-ntpu/ntpu-ce-mmsp-2023/blob/master/Chapter-5/simple_jpeg/simple_jpeg.m 。
            *   但要小心，matlab 的 code 只有處理灰階圖，這個作業要求處理 RBG 以及 YCbCr 色域。
    * DCT 講義：https://hackmd.io/@cychiang-ntpu/DCT 。
* **注意：使用方法題目裡面 (a), (b) 都要實現** 。
* **同學僅要在數位學苑繳交 GitHub link 即可**，GitHub 裡面只要放 `encoder.c`、`decoder.c`、workflow 相關檔案、`README.md` 裡面要包含：
   *  `README.md` 裡面要包含：
       * 系統實作的進度，可以用 block diagram 配合工作日誌來記錄呈現，不用放圖，圖只看 artifacts (納入評分) 。
       * 心得及感想 (納入評分) 。
* **請撰寫 GitHub workflow，將你覺得重要的內容放到 artifact，評分的時候會依據你放的 artifact 內容是否具有意義來評分** 。
* 有準時繳交，就有基本分 40% 的分數，
    * 剩下的 60% 分數就是 「程式使用方法 - 0/1/2/3」四個功能各自 15%。
    * 這個剩下的 60% 會在口試完之後細評。
* 圖檔在 https://github.com/cychiang-ntpu/mmsp-2025-final-jpeg/blob/main/Kimberly.bmp ，但是要用以下指令才可以在 workflow 裡面抓下來：`curl -o Kimberly.bmp https://raw.githubusercontent.com/cychiang-ntpu/mmsp-2025-final-jpeg/main/Kimberly.bmp`
* 可以用 AI 工具輔助撰寫，口試的時候會問你們寫的 code，驗證同學們了解的程度。
* 此份作業沒有補交的開放，口試完就結束本學期。




## 程式使用方法 - 0 

### 目的
希望同學使用這第零個使用方來檢查 bmp 檔有沒有讀正確！

### encoder 程式使用方法 - 0:
```
encoder 0 Kimberly.bmp R.txt G.txt B.txt dim.txt
```
**輸入**
1. `0`: 代表第零個使用方法
2. `Kimberly.bmp`: 代表輸入的 bmp 檔

**輸出**
1. `R.txt:` 以 ascii 儲存的 channel R 的值，`Kimberly.bmp` 裏面 channel R 的每一個 row 的值存成一個 `R.txt` 的一個 row。
2. `G.txt`: 以 ascii 儲存的 channel G 的值，`Kimberly.bmp` 裏面 channel G 的每一個 row 的值存成一個 `G.txt` 的一個 row。
3. `B.txt`: 以 ascii 儲存的 channel B 的值，`Kimberly.bmp` 裏面 channel B 的每一個 row 的值存成一個 `B.txt` 的一個 row。
4. dim.txt: 裡面儲存圖的 size

### decoder 程式使用方法 - 0:
```
decoder 0 ResKimberly.bmp R.txt G.txt B.txt dim.txt
```
**輸入**
1. `0`: 代表第零個使用方法
2. `R.txt`: 由 encoder 使用第零個使用方法以 ascii 儲存的 channel R 的值。
3. `G.txt`: 由 encoder 使用第零個使用方法以 ascii 儲存的 channel G 的值。
4. `B.txt`: 由 encoder 使用第零個使用方法以 ascii 儲存的 channel B 的值。
5. `dim.txt`: 由 encoder 使用第零個使用方法得到圖的 size

**輸出**
1. `ResKimberly.bmp`，是從輸入的 `R/G/B.txt` 以及 `dim.txt` 讀取資料，然後再儲存成 bmp 格式

### 使用 diff/cmp 指令檢查
要使用 linux 指令 diff/cmp 證明 `Kimberly.bmp` 以及重建的圖檔 `ResKimberly.bmp` 是一樣的！
diff/cmp 的用法可以參考  [diff/cmp用法](https://www.796t.com/content/1547457511.html)

## 程式使用方法 - 1

### 目的
希望同學使用這第壹個使用方來檢查：
1. 經過RGB2YCbCr、2D-DCT、Quantization 之後是否動作正確？ 
2. 練習計算量化誤差。

### encoder 程式使用方法 - 1:
```
encoder 1 Kimberly.bmp Qt_Y.txt Qt_Cb.txt Qt_Cr.txt dim.txt qF_Y.raw qF_Cb.raw qF_Cr.raw eF_Y.raw eF_Cb.raw eF_Cr.raw
```
**輸入**
1. `1`: 代表第壹個使用方法
2. `Kimberly.bmp`: 代表輸入的 bmp 檔

**輸出**
1.	`Qt_Y.txt`: 以 ascii 儲存的channel Y 的 quantization table，8x8 quantization table 的每一個 row 的值存成一個 `Qt_Y.txt` 的一個 row，方便人觀察數值。
2.	`Qt_Cb.txt`: 以 ascii 儲存的channel Cb 的 quantization table，儲存方法同 `Qt_Y`.txt。
3.	`Qt_Cr.txt`: 以 ascii 儲存的channel Cr 的 quantization table，儲存方法同 `Qt_Y.txt`。
4.	`dim.txt`: 圖的 size
5.	`qF_Y.raw`: 以 binary 儲存的 channel Y 經過量化後的值，也就是 `qF_Y=round(F_Y/Qt_Y)`， 其中 `F_Y` 就是經過 2D-DCT 轉換後的頻率軸值，而 `Qt_Y` 就是量化表裡的數值，因為圖檔有 504 x 378 個 8x8 的 block，每一個 block 有64個值，請依 [row-major order](https://en.wikipedia.org/wiki/Row-_and_column-major_order) 順序將每一個量化值以 short 資料型態儲存。
6.	`qF_Cb.raw`: 以 binary 儲存的 channel Cb 經過量化後的值，儲存方式如同 `qF_Y.raw`。
7.	`qF_Cr.raw`: 以 binary 儲存的 channel Cr 經過量化後的值，儲存方式如同 `qF_Y.raw`。
8.	`eF_Y.raw`: 以 binary 儲存的 Channel Y 量化誤差值，也就是 `eF_Y = F_Y - qF_Y * Qt_Y`，每一個誤差值以 float 資料型態儲存，儲存順序如同 `qF_Y.raw`。
9.	`eF_Cb.raw`: 以 binary 儲存的 channel Cb 量化誤差值，儲存順序與方法如同 `eF_Y.raw`。
10.	`eF_Cr.raw`: 以 binary 儲存的 channel Cr量化誤差值，儲存順序與方法如同 `eF_Y.raw`。
11.	螢幕列印出以 ascii 表示的 3x64 頻率軸上的 SQNR (dB)，其中 3 代表 Y/Cb/Cr，64代表 8x8 的頻率。

### decoder 程式使用方法 - 1(a):
```
decoder 1 QResKimberly.bmp Kimberly.bmp Qt_Y.txt Qt_Cb.txt Qt_Cr.txt dim.txt qF_Y.raw qF_Cb.raw qF_Cr.raw
```
**輸入**
1.	`1`: 代表第壹個使用方法
2.	`Kimberly.bmp`: 原本的圖檔，將用來計算以 pixel 為單位的分 R/G/B 三個 channel 的 SQNR (dB)
3.	`Qt_Y.txt:` 以 ascii 儲存的channel Y 的 quantization table。
4.	`Qt_Cb.txt`: 以 ascii 儲存的channel Cb 的 quantization table。
5.	`Qt_Cr.txt`: 以 ascii 儲存的channel Cr 的 quantization table。
6.	`dim.txt`: encoder 輸出的圖 size
7.	`qF_Y.raw`: 以 binary 儲存的 channel Y 經過量化後的值。
8.	`qF_Cb.raw`: 以 binary 儲存的 channel Cb 經過量化後的值。
9.	`qF_Cr.raw`: 以 binary 儲存的 channel Cr 經過量化後的值。

**輸出**
1.	`QResKimberly.bmp`，是由 `Qt_Y/Cb/Cr.txt` 以及 `qF_Y/Cb.Cr.raw` 重建的 bmp 檔，會和原本的 `Kimberly.bmp` 有誤差。
2.	螢幕列印出以 ascii 表示的 `3x1` pixel 為單位的 SQNR，其中 `3` 代表 R/G/B 的三個 channel 。也就是用 `QResKimberly.bmp` 和 `Kimberly.bmp` 之間的差值來計算。

### decoder 程式使用方法 - 1(b):
```
decoder 1 ResKimberly.bmp Qt_Y.txt Qt_Cb.txt Qt_Cr.txt dim.txt qF_Y.raw qF_Cb.raw qF_Cr.raw eF_Y.raw eF_Cb.raw eF_Cr.raw
```
**輸入**
1.	`1`: 代表第壹個使用方法
2.	`Qt_Y.txt`: 以 ascii 儲存的channel Y 的 quantization table。
3.	`Qt_Cb.txt`: 以 ascii 儲存的channel Cb 的 quantization table。
4.	`Qt_Cr.txt`: 以 ascii 儲存的channel Cr 的 quantization table。
5.	`dim.txt`: encoder 輸出的圖 size
6.	`qF_Y.raw`: 以 binary 儲存的 channel Y 經過量化後的值。
7.	`qF_Cb.raw`: 以 binary 儲存的 channel Cb 經過量化後的值。
8.	`qF_Cr.raw`: 以 binary 儲存的 channel Cr 經過量化後的值。
9.	`eF_Y.raw`: 以 binary 儲存的 Channel Y 量化誤差值。
10.	`eF_Cb.raw`: 以 binary 儲存的 channel Cb 量化誤差值。
11.	`eF_Cr.raw`: 以 binary 儲存的 channel Cr 量化誤差值。

**輸出**
1.	`ResKimberly.bmp`，是由 `Qt_Y/Cb/Cr.txt`、`qF_Y/Cb/Cr.raw`、以及 `eF_Y/Cb/Cr.raw`，重建的 bmp 檔，應該要和原本的 `Kimberly.bmp` 一模一樣。

### 使用 diff/cmp 指令檢查
要使用 linux 指令 diff/cmp 證明 `Kimberly.bmp` 以及重建的圖檔 `ResKimberly.bmp` 是一樣的！

## 程式使用方法 - 2
### 目的
希望同學使用這第貳個使用方法來檢查：
1. 經過 RGB2YCbCr、2D-DCT、Quantization、DPCM、ZigZag、RLE 之後是否動作正確？ 
2. 練習計算壓縮率。

### encoder 程式使用方法 - 2(a):
```
encoder 2 Kimberly.bmp ascii rle_code.txt 
```
**輸入**
1. `2`: 代表第貳個使用方法
2. `Kimberly.bmp`: 代表輸入的 bmp 檔
3. ascii: 代表將 Y/Cb/Cr 三個 channel 的 RLE 以 ascii 儲存 (讓人看得懂)

**輸出**：
1. `rle_code.txt`: 包含圖檔 size 以及 Y/Cb/Cr channel 所有 8x8 的 RLE

`rle_code.txt` 的格式要求：
1. 第一個 row 儲存圖檔的 size
2. 第二個 row 開始，每三個 row 為一組儲存每一個 8x8 的 RLE，格式如下：
```
($m,$n, Y) $skip1 $value1 $skip2 $value2 ...
($m,$n, Cb) $skip1 $value1 $skip2 $value2 ...
($m,$n, Cr) $skip1 $value1 $skip2 $value2 ...
```

其中 `$m` 代表第 `m` 個 row 的 8x8 block，`$n` 代表第 `n` 個 column 的 8x8 block，
`$skip{%d} $value{%d}` 代表 RLE

### encoder 程式使用方法 - 2(b):
```
encoder 2 Kimberly.bmp binary rle_code.bin
```
**輸入**
1. `2`: 代表第貳個使用方法
2. `Kimberly.bmp`: 代表輸入的 bmp 檔
3. binary: 代表將 Y/Cb/Cr 三個 channel 的 RLE 以 binary 儲存 

**輸出**
1. `rle_code.bin`: 包含圖檔 size 以及 Y/Cb/Cr channel 所有 8x8 的 RLE，格式自訂，decoder 看得懂就好。
2. 螢幕印出 Y/Cb/Cr 三個 channel 各自的壓縮率、以及整體的壓縮率 (`rle_code.bin` 和原本 `Kimberly.bmp` 的壓縮比值或比率)


注意! 如果你已經在 encoder 有指定 8x8 的排列順序，或許在 `rle_code.bin` 可以不用特別儲存 `m/n`。

### decoder 程式使用方法 - 2(a):
```
decoder 2 QResKimberly.bmp ascii rle_code.txt 
```
輸入輸出不再贅述。

請同學檢查 `QResKimberly.bmp` 是否和 "第壹個使用方法" 所解碼的 `QResKimberly.bmp` 有沒有一模一樣 (by using diff/cmp)

### decoder 程式使用方法 - 2(b):
```
decoder 2 QResKimberly.bmp binary rle_code.bin 
```
輸入輸出不再贅述。

請同學檢查 `QResKimberly.bmp` 是否和 "第壹個使用方法" 所解碼的 `QResKimberly.bmp` 有沒有一模一樣 (by using diff/cmp)


## 程式使用方法 - 3
### 目的
希望同學使用這第參個使用方法來檢查：
1. 經過RGB2YCbCr、2D-DCT、Quantization、DPCM、ZigZag、RLE、Huffman coding 之後是否動作正確？ 
2. 練習計算壓縮率。

### encoder 程式使用方法 - 3(a):
```
encoder 3 Kimberly.bmp ascii codebook.txt huffman_code.txt 
```
**輸入**
1. `3`: 代表第參個使用方法
2. `Kimberly.bmp`: 代表輸入的 bmp 檔
3. ascii: 代表將 Huffman code 以 ascii 儲存 (讓人看得懂)

**輸出**
1. `codebook.txt`: 以 ascii 表示方便人看懂的 codebook，裡面包含 symbol、symbol 對應的 count、symbol 對應的 codeword。
2. `huffman_code.txt`: 以 ascii 表示方便人看懂的 encoding，裡面應該要包含圖片的 size 資訊、以及 symbol串對應的 bitstream。

請注意：
1. 請自訂 `codebook.txt` 以及 `huffman_code.txt` 的格式，要讓人可以看得懂。
2. `codebook.txt` 可以考慮把 Y/Cb/Cr 以及 DC/AC 成份全部合在一起建立一本 codebook，這樣子是簡易版的。
3. `codebook.txt` 也可以做成接近真實 jpeg 標準的四本 codebooks，包含：
   a) codebook for DC component of channel Y
   b) codebook for DC component of channels Cb and Cr
   c) codebook for AC component of channel Y
   d) codebook for AC component of channels Cb and Cr
   建立此版本可以加分。

### encoder 程式使用方法 - 3(b):
```
encoder 3 Kimberly.bmp binary codebook.txt huffman_code.bin 
```

**輸入**
1. `3`: 代表第參個使用方法
2. `Kimberly.bmp`: 代表輸入的 bmp 檔
3. binary: 代表將 Huffman code 以 binary 儲存 (真的才有壓縮效果)

**輸出**
1. `codebook.txt`: 以 ascii 表示方便人看懂的 codebook，裡面包含 symbol、symbol 對應的 count、symbol 對應的 codeword。
2. `huffman_code.bin`: 以 binary 表示的 encoding，裡面應該要包含圖片的 size 資訊、以及 symbol 串對應的 bitstream。
3. 螢幕印出整體的壓縮率 (`huffman_code.bin` 和原本 `Kimberly.bmp` 的壓縮比值或比率)

請注意：
1. 請自訂 `codebook.txt` 以及 `huffman_code.bin` 的格式。
2. `codebook.txt` 的格式設計可以和 encoder 程式使用方法 - 3(a) 一樣。

### decoder 程式使用方法 - 3(a):
```
decoder 3 QResKimberly.bmp ascii codebook.txt huffman_code.txt 
```
輸入輸出不再贅述。

請同學檢查 `QResKimberly.bmp` 是否和 "第壹個使用方法" 所解碼的 `QResKimberly.bmp` 有沒有一模一樣 (by using diff/cmp)

### decoder 程式使用方法 - 3(b):
```
decoder 3 QResKimberly.bmp binary codebook.txt huffman_code.bin 
```
輸入輸出不再贅述。

請同學檢查 `QResKimberly.bmp` 是否和 "第壹個使用方法" 所解碼的 `QResKimberly.bmp` 有沒有一模一樣 (by using diff/cmp)
