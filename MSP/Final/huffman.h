// huffman.h
#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdint.h>
#include <stdio.h>

// 1. define a Symbol (run-length pair) (run, val)
typedef struct {
    int run;
    int val;
} Symbol;

// 2. Huffman tree node
typedef struct HuffNode {
    Symbol sym;           //  (Run, Val)
    long count;           // frequency
    double prob;          // probability (for codebook info)
    int is_leaf;          // is leaf node
    struct HuffNode *left;
    struct HuffNode *right;
    struct HuffNode *parent; // for upward traversal during tree construction 
    char *code;           // final generated code string (e.g., "1101")
} HuffNode;

// 3. define a Codebook (Symbol Table)
// Because we have 4 books, encapsulating them in a structure is better for management
typedef struct {
    HuffNode **nodes;     // array of pointers, storing all symbol nodes
    int size;             // current number of different symbols
    int capacity;         // array capacity
} HuffmanTable;

// --- function declarations ---

// initialization and cleanup
void init_huffman_table(HuffmanTable *table);
void free_huffman_table(HuffmanTable *table);

// core functions
void add_symbol(HuffmanTable *table, int run, int val); // for frequency counting
void build_huffman_tree(HuffmanTable *table);           // build tree & generate codes
HuffNode* find_symbol(HuffmanTable *table, int run, int val); // for lookup

// I/O functions
void write_codebook(FILE *f, HuffmanTable *table, const char *name); // w codebook.txt
void write_huffman_bits(FILE *f, const char *code, uint8_t *bit_buf, int *bit_cnt); // write bits
void flush_huffman_bits(FILE *f, uint8_t *bit_buf, int *bit_cnt); // write remaining bits

#endif  