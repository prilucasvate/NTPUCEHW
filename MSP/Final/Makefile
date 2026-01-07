CC = gcc
CFLAGS = -Wall -O2

# compile both encoder and decoder
all: encoder decoder

# Encoder 
encoder: encoder.o bmp.o transform.o huffman.o
	$(CC) $(CFLAGS) -o encoder encoder.o bmp.o transform.o huffman.o -lm

# Decoder 
decoder: decoder.o bmp.o transform.o huffman.o
	$(CC) $(CFLAGS) -o decoder decoder.o bmp.o transform.o huffman.o -lm

# Common compilation rules (depend on headers)
%.o: %.c bmp.h transform.h huffman.h
	$(CC) $(CFLAGS) -c $<

# clean 
clean:
	rm -f *.o encoder decoder *.txt *.bin *.raw Res*.bmp QRes*.bmp code_*.txt