#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logger.h"

// max symbol length (UTF-8 or Big5)
#define MAX_SYMB_LEN 4

// huffman tree node structure
typedef struct Node {
    struct Node *left;    // left child (bit 0)
    struct Node *right;   // right child (bit 1)
    int is_leaf;          // is leaf node
    
    unsigned char chr[MAX_SYMB_LEN]; // symbol bytes
    int useLen;           // symbol byte length
} Node;

// huffman tree root
Node *Root = NULL;

// ------------------ create a new node -------------------------
Node* create_node() {
    Node *node = (Node*)calloc(1, sizeof(Node));
    return node;
}

// ------------------- insert code to huffman tree ------------------------
//  root, code, chr(symbol), len(symbol length)
void insert_code(Node *root, const char *code, const unsigned char *chr, int len) {
    Node *curr = root;
    const char *p = code;
    
    // follow the code path 0 or 1 until the end, then build the tree
    while (*p != '\0') {
        if (*p == '0') {
            if (curr->left == NULL) {
                curr->left = create_node();
            }
            curr = curr->left;
        } else if (*p == '1') {
            if (curr->right == NULL) {
                curr->right = create_node();
            }
            curr = curr->right;
        }
        p++;
    }
    // go to leaf node, set symbol
    curr->is_leaf = 1;
    memcpy(curr->chr, chr, len);
    curr->useLen = len;
}

// --------------------- build tree with codebook ------------------------
// search codebook line backwards to parse fields
// CSV : "Symbol",count,prob,code,info
void parse_and_build(char *line) {
    // remove newline characters
    int len = strlen(line);
    while(len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
        line[--len] = '\0';
    }
    if (len == 0) return;

    // find code and symbol fields by scanning backwards
    // find last 4 commas, second last is code, fourth last is symbol
    char *ptr = line + len - 1;
    int comma_cnt = 0;
    char *code_start = NULL;
    char *symbol_end = NULL; // address of the end of symbol field

    while (ptr >= line) {
        if (*ptr == ',') {
            comma_cnt++;
            if (comma_cnt == 2) {
                // code, at the second last comma
                code_start = ptr + 1;
                // code starts after this comma (ptr +1)
            } 
            else if (comma_cnt == 4) {
                // symbol end, at the fourth last comma
                symbol_end = ptr;
                *symbol_end = '\0'; // cut off symbol field to string
                break; 
            }
        }
        ptr--;
    }

    // check if wrong format
    if (comma_cnt < 4 || code_start == NULL) return;

    // cut the code field to string before next comma (e.g. 10,3.5)
    char *p = code_start;
    while(*p){
        if(*p == ',') { *p = '\0'; break; }
        p++;
    } // (e.g. code = "10")

    // handle symbol field from line start to symbol_end
    char *raw_sym = line;
    
    // remove surrounding quotes  (e.g. "A" -> A)
    if (raw_sym[0] == '"') raw_sym++; // skip first quote
    int rlen = strlen(raw_sym);
    if (rlen > 0 && raw_sym[rlen-1] == '"') raw_sym[rlen-1] = '\0'; // skip last quote
    // special symbol handling
    unsigned char symbol[4] = {0};
    int symLen = 0;

    if (strcmp(raw_sym, "EOF") == 0) {
        // special EOF symbol
        strcpy((char*)symbol, "EOF");
        symLen = 3; 
    } else if (strcmp(raw_sym, "\\n") == 0) {
        symbol[0] = '\n'; symLen = 1;
    } else if (strcmp(raw_sym, "\\r") == 0) {
        symbol[0] = '\r'; symLen = 1;
    } else if (strcmp(raw_sym, "\\t") == 0) {
        symbol[0] = '\t'; symLen = 1;
    } else if (strcmp(raw_sym, "\\\"") == 0) { // \" -> "
        symbol[0] = '\"'; symLen = 1;
    } else if (strcmp(raw_sym, "\\\\") == 0) { // \\ -> 
        symbol[0] = '\\'; symLen = 1;
    } else {
        // copy normal symbol & "" -> "
        int i = 0, j = 0;
        while (raw_sym[i] != '\0') {
            // if "" -> "
            if (raw_sym[i] == '"' && raw_sym[i+1] == '"') {
                symbol[j++] = '"';
                i += 2;
            } else {
                symbol[j++] = raw_sym[i++];
            }
        }
        symbol[j] = '\0'; // end
        symLen = j;       // actual length
    }
    
    // insert into Huffman Tree
    insert_code(Root, code_start, symbol, symLen);
}


// ---------------------- main ---------------------------
int main(int argc, char *argv[]) {
    if (argc != 4) {
        log_warn("decoder", "Usage: %s <input> <codebook> <output>", argv[0]);
        return -1;
    }

    FILE *fout = fopen(argv[1], "wb");
    FILE *fcsv = fopen(argv[2], "r");
    FILE *fin  = fopen(argv[3], "rb");

    if (!fout || !fcsv || !fin) {
        log_error("decoder", "Failed to open input file: %s", argv[1]);
        return -1;
    }

    // intialize Huffman Tree
    Root = create_node();
    log_info("decoder", "starting building Huffman Tree ...");
    // read codebook and build Huffman Tree
    char lineBuf[1024];
    while (fgets(lineBuf, sizeof(lineBuf), fcsv)) {
        parse_and_build(lineBuf);
    }
    
    log_info("decoder", "Huffman Tree built successfully.");
    fclose(fcsv);

    // decode the file
    Node *curr = Root;
    int c;
    long total_bytes = 0;
    int eof_found = 0;
    log_info("decoder", "write symbol into the file ...");
    // read encoded file byte by byte
    while ((c = fgetc(fin)) != EOF) {
        if (eof_found) break; 

        // read each bit (7 -> 0) in the byte
        for (int i = 7; i >= 0; i--) {
            int bit = (c >> i) & 1;

            if (bit == 0) {
                curr = curr->left;
            } else {
                curr = curr->right;
            }

            // error check for invalid path
            if (curr == NULL) {
                fprintf(stderr, "Error: Invalid path (code not found in tree).\n");
                return -1;
            }
            
            // reach leaf node (is symbol)
            if (curr->is_leaf) {
                // check is EOF
                if (curr->useLen == 3 && strncmp((char*)curr->chr, "EOF", 3) == 0) {
                    eof_found = 1;
                    break; // break loop
                }

                // write symbol to output file
                fwrite(curr->chr, 1, curr->useLen, fout);
                total_bytes++;

                // reset to root
                curr = Root;
            }
        }
    }

    log_info("decoder", "decoding complete. Output saved to %s", argv[1]);
    fclose(fin);
    fclose(fout);
    
    return 0;
} 