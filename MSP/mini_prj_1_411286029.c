#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#define ASCII_MAX     128
#define MAX_SYMB     2000

typedef struct {
    unsigned char chr[4];     // bytes of symbol
    int useLen;               //size: 1~4 bytes
    int count;                //number of this symbol
    double prob;              //probability 
} Symb;

static int utf8_len(unsigned char b){ //use first byte to check UTF-8 using length
    if ((b & 0x80)==0x00) return 1; //0xxxxxxx
    if ((b & 0xE0)==0xC0) return 2; //110xxxxx
    if ((b & 0xF0)==0xE0) return 3; //1110xxxx
    if ((b & 0xF8)==0xF0) return 4; //11110xxx
    return 0;  
}
static int follow_ok(unsigned char b){ //check is UTF-8 follow byte legal (10xxxxxx)
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
        fputc(s[i],stdout); 
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
    for(int i=0;i<ASCII_MAX;i++){
        symb[i].chr[0]=(unsigned char)i; 
        symb[i].useLen=1; 
        symb[i].count=0; 
    }
    int used = ASCII_MAX;  // count used symb[], start from 128
    int total = 0; //count total symbol

    int c;
    while((c=fgetc(stdin))!=EOF){
        unsigned char b0 =(unsigned char)c;
        int chrLen=utf8_len(b0); //b0  (first byte)
        if(chrLen<=1){ // is ASCII (one byte)
            symb[b0].count++; 
            total++; 
            continue;
        }
        //below is not ascii
        unsigned char tmp[4]; 
        tmp[0]=b0;

        int read=1, ok=1; //check byte is legal & add in tmp[]
        for(int i=1;i<chrLen;i++){
            int d=fgetc(stdin);
            if(d==EOF){ ok=0; break; }
            tmp[read++]=(unsigned char)d;
            if(!follow_ok(tmp[read-1])){ ok=0; break; } //following is illegal
        }
        if(!ok){ //illegal
            for(int j=read-1;j>=1;--j) ungetc(tmp[j],stdin); // put illegal byte back
            symb[b0].count++; 
            total++; 
            continue;
        }
        // legal UTF-8 add to symb
        // check UTF-8 exist & +1
        int found=0;
        for(int i=ASCII_MAX;i<used;i++){ //linear search 
            if(symb[i].useLen==chrLen && memcmp(symb[i].chr,tmp,chrLen)==0){ //check same
            symb[i].count++; 
            found=1; 
            break; 
            }
        }
        //new UTF-8 , extend symb[] and set count=1
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
    printf("\ntotal: %d",total);
    return 0;
}
