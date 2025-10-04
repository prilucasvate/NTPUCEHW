// mixcount.c  （混編檔：UTF-8 + Big5 → 合併為同一碼點統計、UTF-8 輸出）
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <iconv.h>

/* -------------- 參數 -------------- */
#define MAX_CP 0x110000  /* Unicode 上限 (0..0x10FFFF) */

/* -------------- 結構 -------------- */
typedef struct { uint32_t cp; int cnt; } Item;

/* -------------- UTF-8 嚴格解碼 -------------- */
static int utf8_len(unsigned char b0){
    if ((b0 & 0x80) == 0x00) return 1;
    if ((b0 & 0xE0) == 0xC0) return 2;
    if ((b0 & 0xF0) == 0xE0) return 3;
    if ((b0 & 0xF8) == 0xF0) return 4;
    return 0;
}
static int utf8_follow_ok(unsigned char b){ return (b & 0xC0) == 0x80; }

static int decode_utf8_strict(const unsigned char *s, int avail,
                              uint32_t *out_cp, int *used){
    *used = 0;
    if (avail <= 0) return 0;
    int len = utf8_len(s[0]);
    if (len == 0 || len > avail) return 0;

    uint32_t cp = 0;
    if (len == 1){
        cp = s[0];
    }else{
        for (int i = 1; i < len; i++){
            if (!utf8_follow_ok(s[i])) return 0;
        }
        if (len == 2){
            cp = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
            if (cp < 0x80) return 0;                 // overlong
        }else if (len == 3){
            cp = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
            if (cp < 0x800) return 0;                // overlong
            if (cp >= 0xD800 && cp <= 0xDFFF) return 0; // surrogate
        }else{ // len == 4
            cp = ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12) |
                 ((s[2] & 0x3F) << 6)  |  (s[3] & 0x3F);
            if (cp < 0x10000) return 0;              // overlong
            if (cp > 0x10FFFF) return 0;
        }
    }
    *out_cp = cp;
    *used = len;
    return 1;
}

/* -------------- Big5 形狀判斷 -------------- */
/* Big5 lead: 0x81–0xFE ; trail: 0x40–0x7E, 0xA1–0xFE */
static int is_big5_lead(unsigned char b){ return (b >= 0x81 && b <= 0xFE); }
static int is_big5_trail(unsigned char b){ return ( (b >= 0x40 && b <= 0x7E) || (b >= 0xA1 && b <= 0xFE) ); }

/* -------------- 用 iconv 轉 Big5→UTF-32LE 拿碼點 -------------- */
static iconv_t g_cd = (iconv_t)-1;

static int big5_to_codepoint_iconv(unsigned char b1, unsigned char b2, uint32_t *out_cp){
    if (g_cd == (iconv_t)-1) return 0;

    char inbuf[2];  inbuf[0] = (char)b1; inbuf[1] = (char)b2;
    size_t inleft = 2;
    char outbuf[8]; memset(outbuf, 0, sizeof(outbuf));
    char *pin = inbuf, *pout = outbuf;
    size_t outleft = sizeof(outbuf);

    // reset shift state for stateless encodings (Big5 是 stateless，這行可省，但保險一點)
    iconv(g_cd, NULL, NULL, NULL, NULL);

    size_t r = iconv(g_cd, &pin, &inleft, &pout, &outleft);
    if (r == (size_t)-1) return 0;

    size_t produced = sizeof(outbuf) - outleft; // bytes of UTF-32LE
    if (produced < 4) return 0;

    // UTF-32LE → 取前 4 bytes（小端序）
    uint32_t cp = (unsigned char)outbuf[0]
                | ((uint32_t)(unsigned char)outbuf[1] << 8)
                | ((uint32_t)(unsigned char)outbuf[2] << 16)
                | ((uint32_t)(unsigned char)outbuf[3] << 24);

    if (cp > 0x10FFFF) return 0; // 防呆
    *out_cp = cp;
    return 1;
}

/* -------------- 把碼點編回 UTF-8 -------------- */
static int encode_utf8(uint32_t cp, unsigned char out[4]){
    if (cp <= 0x7F){ out[0] = (unsigned char)cp; return 1; }
    if (cp <= 0x7FF){
        out[0] = 0xC0 | (cp >> 6);
        out[1] = 0x80 | (cp & 0x3F);
        return 2;
    }
    if (cp <= 0xFFFF){
        out[0] = 0xE0 | (cp >> 12);
        out[1] = 0x80 | ((cp >> 6) & 0x3F);
        out[2] = 0x80 | (cp & 0x3F);
        return 3;
    }
    out[0] = 0xF0 | (cp >> 18);
    out[1] = 0x80 | ((cp >> 12) & 0x3F);
    out[2] = 0x80 | ((cp >> 6)  & 0x3F);
    out[3] = 0x80 | (cp & 0x3F);
    return 4;
}

/* -------------- 排序：count ↓，cp ↑ -------------- */
static int cmp_item(const void *a, const void *b){
    const Item *x = (const Item*)a, *y = (const Item*)b;
    if (y->cnt != x->cnt) return y->cnt - x->cnt;
    if (x->cp  <  y->cp ) return -1;
    if (x->cp  >  y->cp ) return  1;
    return 0;
}

/* -------------- 主流程 -------------- */
int main(void){
    /* 建立 Big5→UTF-32LE 轉換器 */
    g_cd = iconv_open("UTF-32LE", "BIG5");
    if (g_cd == (iconv_t)-1){
        fprintf(stderr, "[ERR] iconv_open(BIG5->UTF-32LE) 失敗，請確認已安裝 libiconv。\n");
        return 1;
    }

    /* 以碼點統計 */
    int *counts = (int*)calloc(MAX_CP, sizeof(int));
    if (!counts){ fprintf(stderr, "OOM\n"); iconv_close(g_cd); return 1; }

    int total = 0;
    unsigned char look[4];
    while (1){
        int c0 = fgetc(stdin);
        if (c0 == EOF) break;
        look[0] = (unsigned char)c0;

        /* 先試 UTF-8：最多再多讀 3 bytes 放在 look[] 給 decoder 用 */
        int filled = 1;
        for (int k = 1; k < 4; k++){
            int d = fgetc(stdin);
            if (d == EOF) break;
            look[filled++] = (unsigned char)d;
        }

        uint32_t cp = 0;
        int used = 0;

        if (decode_utf8_strict(look, filled, &cp, &used)){
            // 把多讀未使用的退回
            for (int k = filled - 1; k >= used; k--) ungetc(look[k], stdin);
        }else{
            // 退回 UTF-8 嘗試時多讀的全部，回到只有 b0 的狀態
            for (int k = filled - 1; k >= 1; k--) ungetc(look[k], stdin);

            // 再試 Big5（需要再看一個 byte）
            int c1 = fgetc(stdin);
            if (c1 != EOF && is_big5_lead(look[0]) && is_big5_trail((unsigned char)c1)){
                if (big5_to_codepoint_iconv(look[0], (unsigned char)c1, &cp)){
                    used = 2; // 成功用掉兩個 byte
                }else{
                    // iconv 不認得 → 當單 byte
                    if (c1 != EOF) ungetc(c1, stdin);
                    cp = (uint32_t)look[0];
                    used = 1;
                }
            }else{
                // 不是合法 Big5 雙位元組 → 把第二個退回、當單 byte
                if (c1 != EOF) ungetc(c1, stdin);
                cp = (uint32_t)look[0];
                used = 1;
            }
        }

        if (cp < MAX_CP){ counts[cp]++; total++; }
    }

    /* 收集非零 */
    Item *arr = (Item*)malloc(sizeof(Item) * (size_t)MAX_CP);
    int n = 0;
    for (uint32_t cp = 0; cp < MAX_CP; cp++){
        if (counts[cp] > 0){ arr[n].cp = cp; arr[n].cnt = counts[cp]; n++; }
    }
    qsort(arr, n, sizeof(Item), cmp_item);

    /* 輸出 CSV（UTF-8），\r \n 分開顯示；加上機率 */
    for (int i = 0; i < n; i++){
        uint32_t cp = arr[i].cp;
        int cnt = arr[i].cnt;
        double p = (total > 0) ? ((double)cnt / (double)total) : 0.0;

        if (cp == '\r'){ printf("\"\\r\",%d,%.15f\n", cnt, p); continue; }
        if (cp == '\n'){ printf("\"\\n\",%d,%.15f\n", cnt, p); continue; }
        if (cp == '\t'){ printf("\"\\t\",%d,%.15f\n", cnt, p); continue; }

        unsigned char out[4];
        int L = encode_utf8(cp, out);

        putchar('"');
        for (int k = 0; k < L; k++){
            if (out[k] == '"') putchar('"'); // CSV 轉義
            putchar(out[k]);
        }
        putchar('"');
        printf(",%d,%.15f\n", cnt, p);
    }

    free(arr);
    free(counts);
    iconv_close(g_cd);
    return 0;
}
