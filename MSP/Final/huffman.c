// huffman.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"

// initialize Huffman Table
void init_huffman_table(HuffmanTable *table) {
    table->size = 0;
    table->capacity = 256; // initial capacity
    table->nodes = (HuffNode**)malloc(sizeof(HuffNode*) * table->capacity);
}

// find if symbol exists (Linear Search)
HuffNode* find_symbol(HuffmanTable *table, int run, int val) {
    for (int i = 0; i < table->size; i++) { 
        if (table->nodes[i]->sym.run == run && table->nodes[i]->sym.val == val) {
            return table->nodes[i]; // return pointer to the node
        }
    }
    return NULL;
}

// add symbol or increase count
void add_symbol(HuffmanTable *table, int run, int val) {
    HuffNode *node = find_symbol(table, run, val);
    if (node) {
        node->count++; // found, count++
    } else {
        // size exceed capacity, reallocate
        if (table->size >= table->capacity) {
            table->capacity *= 2; // double capacity
            table->nodes = (HuffNode**)realloc(table->nodes, sizeof(HuffNode*) * table->capacity);
        }
        // add new node
        HuffNode *new_node = (HuffNode*)calloc(1, sizeof(HuffNode));
        new_node->sym.run = run; // set run
        new_node->sym.val = val; // set val
        new_node->count = 1; 
        new_node->is_leaf = 1;
        table->nodes[table->size++] = new_node; 
    }
}

// DFS to generate codes
void generate_codes_recursive(HuffNode *root, char *current_code, int depth) {
    if (!root) return; // safety check

    // if leaf, assign code
    if (root->is_leaf) {
        root->code = (char*)malloc(depth + 1);
        // if depth is 0 (single node tree), assign "0"
        if (depth == 0) {
            strcpy(root->code, "0");
        } else {
            strncpy(root->code, current_code, depth); // copy code to node
            root->code[depth] = '\0'; // end string
        }
        return;
    }

    // DFS -> Node, left (0), right (1) (pre-order)
    current_code[depth] = '0';
    generate_codes_recursive(root->left, current_code, depth + 1);

    current_code[depth] = '1';
    generate_codes_recursive(root->right, current_code, depth + 1);
}

// build Huffman Tree (find two min, merge, repeat)
void build_huffman_tree(HuffmanTable *table) {
    if (table->size == 0) return;

    // copy nodes to a temporary heap array
    int heap_size = table->size;
    HuffNode **heap = (HuffNode**)malloc(sizeof(HuffNode*) * heap_size);
    for(int i=0; i<heap_size; i++) heap[i] = table->nodes[i]; // copy pointers

    // start merging until only one tree remains
    while (heap_size > 1) {
        // find two minimum nodes (min1, min2)
        int min1_idx = -1, min2_idx = -1;
        long min1_val = -1, min2_val = -1;

        for (int i = 0; i < heap_size; i++) {
            long c = heap[i]->count;
            if (min1_idx == -1 || c < min1_val) { // find smallest
                // old min1 becomes min2
                min2_val = min1_val; min2_idx = min1_idx;
                // update min1
                min1_val = c; min1_idx = i;
            } else if (min2_idx == -1 || c < min2_val) {
                min2_val = c; min2_idx = i; // find second smallest
            }
        }

        // get the two minimum nodes from heap
        HuffNode *left = heap[min1_idx];
        HuffNode *right = heap[min2_idx];

        // build new parent node
        HuffNode *parent = (HuffNode*)calloc(1, sizeof(HuffNode));
        parent->count = left->count + right->count; // sum counts
        parent->left = left; // min1 as left
        parent->right = right; // min2 as right
        parent->is_leaf = 0; // not a leaf

        // remove min1 and min2 from heap and insert parent
        // put parent back to min1 position, replace min2 with last element
        heap[min1_idx] = parent;
        heap[min2_idx] = heap[heap_size - 1];
        heap_size--;
    }

    HuffNode *root = heap[0]; // root of the Huffman tree
    
    // generate codes
    char buffer[65536]; 
    generate_codes_recursive(root, buffer, 0);

    free(heap); 
}

// output codebook to file
void write_codebook(FILE *f, HuffmanTable *table, const char *name) {
    //fprintf(f, "--- Codebook: %s ---\n", name);
    // format: run val count code
    for (int i = 0; i < table->size; i++) {
        HuffNode *n = table->nodes[i];
        if (n->count > 0) { // only output symbols with count > 0
            fprintf(f, "%d %d %ld %s\n", n->sym.run, n->sym.val, n->count, n->code);
        }
    }
    fprintf(f, "\n");
}

// write bits to file
void write_huffman_bits(FILE *f, const char *code, uint8_t *bit_buf, int *bit_cnt) {
    for (int i = 0; code[i] != '\0'; i++) { // read each bit one by one
        char bit = code[i];
        
        // push bit into buffer (left to right)
        // shift left by << 1 and add new bit using OR
        // (bit - '0') converts char '0'/'1' to int 0/1
        *bit_buf = (*bit_buf << 1) | (bit - '0');
        (*bit_cnt)++; // increase bit count

        // write to file when buffer is full (8 bits)
        if (*bit_cnt == 8) { // 8 bits full, write to file
            fwrite(bit_buf, 1, 1, f);
            *bit_cnt = 0; // reset bit count
            *bit_buf = 0; // reset buffer
        }
    }
}

// flush last remaining bits in buffer (pad with 0s)
void flush_huffman_bits(FILE *f, uint8_t *bit_buf, int *bit_cnt) {
    if (*bit_cnt > 0) {
        // shift (8 - *bit_cnt) to left to pad with 0s (8 - bit count)
        *bit_buf = (*bit_buf << (8 - *bit_cnt)); // pad with 0 by shift left
        fwrite(bit_buf, 1, 1, f);
        *bit_cnt = 0; // reset bit count
        *bit_buf = 0; // reset buffer
    }
}

void free_huffman_table(HuffmanTable *table) {
    if(table->nodes) free(table->nodes);
}