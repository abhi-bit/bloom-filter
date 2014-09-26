#ifndef _BLOOM_H
#define _BLOOM_H

struct bloom {
    int entries;
    double error;
    int bits;
    int bytes;
    int hashes;

    double bpe;
    unsigned char *bf;
    int ready;
};

int bloom_init(struct bloom *bloom, int entries, double error);
int bloom_check(struct bloom *bloom, const void *buffer, int len);
int bloom_add(struct bloom *bloom, const void *buffer, int len);
void bloom_print(struct bloom *bloom);
void bloom_free(struct bloom *bloom);

#endif
