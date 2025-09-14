#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#define BYTE_MAX     256
#define MAX_SYMB     3000

typedef struct {
    unsigned char chr[4];     // bytes of symbol
    int useLen;               //size: 1~4 bytes
    int count;                //number of this symbol
    double prob;              //probability 
} Symb;

static int big5_len(unsigned char b0){
    if (b0 <= 0x7F) return 1;                 // ASCII
    if (b0 >= 0x81 && b0 <= 0xFE) return 2;   // big5 high
    return 1;                                 
}
// big5 low 0x40–0x7E , 0xA1–0xFE
static int big5_follow_ok(unsigned char b1){
    return ( (b1 >= 0x40 && b1 <= 0x7E) || (b1 >= 0xA1 && b1 <= 0xFE) );
}
static int utf8_len(unsigned char b){ //use first byte to check UTF-8 using length
    if ((b & 0x80)==0x00) return 1; //0xxxxxxx
    if ((b & 0xE0)==0xC0) return 2; //110xxxxx
    if ((b & 0xF0)==0xE0) return 3; //1110xxxx
    if ((b & 0xF8)==0xF0) return 4; //11110xxx
    return 0;  
}
static int utf8_follow_ok(unsigned char b){ //check is UTF-8 follow byte legal (10xxxxxx)
    return (b & 0xC0)==0x80; 
}

//qsort compare func
static int cmp(const void *a, const void *b){ 
    const Symb *x = (const Symb*)a;
    const Symb *y = (const Symb*)b;
    if(x->count != y->count) 
        return y->count - x->count;   // 1.count  descending
    if(x->useLen != y->useLen) 
        return x->useLen - y->useLen;     // 2.length ascending
    return x->chr[(x->useLen)-1] - y->chr[(x->useLen)-1]; //3.byte index ascending
}
//write char to output
static void csv_char(const unsigned char *s,int len){
    if (len==1 && s[0]=='\r'){ fputs("\"\\r\"",stdout); return; }
    if (len==1 && s[0]=='\n'){ fputs("\"\\n\"",stdout); return; }
    if (len==1 && s[0]=='\t'){ fputs("\"\\t\"",stdout); return; }
    fputc('"',stdout); //""
    for(int i=0;i<len;i++){
        if (s[i] == '"') {
            fputc('"', stdout);
        }else {
            fputc(s[i],stdout); 
        }
    }
    fputc('"',stdout);//""
}

int main(void){
#ifdef _WIN32
    _setmode(_fileno(stdin),  _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    Symb symb[MAX_SYMB]={0};
    // initial ascii :len=1 chr[0]=ascii
    for(int i=0;i<BYTE_MAX;i++){
        symb[i].chr[0]=(unsigned char)i; 
        symb[i].useLen=1; 
        symb[i].count=0; 
    }
    int used = BYTE_MAX;  // count used symb[], start from 128
    int total = 0; //count total symbol

    int c;
    while((c=fgetc(stdin))!=EOF){
        unsigned char b0 =(unsigned char)c;//b0  (first byte)
        if(b0 <= 0x7F){ // is ASCII (one byte)
            symb[b0].count++; 
            total++; 
            continue;
        }
        //below is not ascii
        int chrLen=1; 
        unsigned char tmp[4]; 
        tmp[0]=b0;
        //try utf8
        int uLen = utf8_len(b0); //use length
        int read=1, ok=0; //check byte is legal & add in tmp[]
        if(uLen > 1){
            ok = 1;
            for(int i=1;i<uLen;i++){
                int d=fgetc(stdin);
                if(d==EOF){ ok=0; break; }
                tmp[read++]=(unsigned char)d;
                if(!utf8_follow_ok(tmp[read-1])){ ok=0; break; } //following is illegal
            }
        }
        if (ok){
            chrLen = uLen; // is UTF-8 
        } else {
            for (int j = read-1; j >= 1; --j) ungetc(tmp[j], stdin); //go back to first
        }
        // try BIG5
        if (chrLen == 1){
            uLen = big5_len(b0);
            if (uLen == 2){
                int d = fgetc(stdin);
                if (d != EOF && big5_follow_ok((unsigned char)d)){
                    tmp[1] = (unsigned char)d;
                    chrLen = 2;   // is BIG5 
                    read = 2;
                } else {
                    if (d != EOF) ungetc(d, stdin);
                }
            }
        }
        if (chrLen == 1){ //other byte unknown
            if (symb[b0].useLen == 0){          // 第一次用到這個索引
                symb[b0].useLen = 1;
                symb[b0].chr[0] = b0;
                if (b0 >= used) 
                    used = b0 + 1;   // 讓後面輸出走得到
            }
            symb[b0].count++;
            total++;
            continue;
        }

        // legal char add to symb
        // check exist & +1
        int found=0;
        for(int i=BYTE_MAX;i<used;i++){ //linear search 
            if(symb[i].useLen==chrLen && memcmp(symb[i].chr,tmp,chrLen)==0){ //check same
            symb[i].count++; 
            found=1; 
            break; 
            }
        }
        //new symbol , extend symb[] and set count=1
        if(!found && used<MAX_SYMB){
            memcpy(symb[used].chr,tmp,chrLen);
            symb[used].useLen=chrLen;
            symb[used].count=1;
            used++;
        }
        total++;
    }

    // filter count>0 symbol
    Symb out[MAX_SYMB];
    int n=0;
    for(int i=0;i<used;i++) {
        if(symb[i].count>0){ 
            out[n]=symb[i]; 
            n++; 
        }
    }
    // count the probability
    for(int i=0;i<n;i++){
        if(total<=0){
            out[i].prob = 0.0;
            continue;
        }
        out[i].prob = (double)out[i].count/(double)total;
    }
    // sort & output
    qsort(out,n,sizeof(Symb),cmp);
    for(int i=0;i<n;i++){
        csv_char(out[i].chr,out[i].useLen);
        printf(",%d,%.15f\n", out[i].count, out[i].prob);
    }
    return 0;
}
