#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "bloom.h"
#include "murmurhash2.h"


int bloom_init(struct bloom *bloom, int entries, double error)
{
    bloom->ready = 0;
    if (entries < 1 || error == 0)
        return 1;

    bloom->entries = entries;
    bloom->error = error;

    double num = log(bloom->error);
    double denom = 0.480453013918201;
    bloom->bpe = -(num / denom);

    double dentries = (double)entries;
    bloom->bits = (int)(dentries * bloom->bpe);

    if (bloom->bits % 8)
        bloom->bytes = (bloom->bytes / 8) + 1;
    else
        bloom->bytes = bloom->bits / 8;

    bloom->hashes = (int)ceil(0.693147180559945 * bloom->bpe);

    bloom->bf = (unsigned char *)calloc(bloom->bytes, sizeof(unsigned char));
    if(bloom->bf == NULL)
        return 1;

    bloom->ready = 1;
    return 0;
}


static int bloom_check_add(struct bloom *bloom,
                            const void *buffer, int len, int add)
{
    if(bloom->ready == 0) {
        (void)printf("bloom at %p not initialised\n", (void *)bloom);
        return -1;
    }

    int hits = 0;
    register unsigned int a = murmurhash2(buffer, len, 0x9747b28c);
    register unsigned int b = murmurhash2(buffer, len, a);
    register unsigned int x;
    register unsigned int i;
    register unsigned int byte;
    register unsigned int mask;
    register unsigned int c;

    for(i = 0; i < bloom->hashes; i++) {
        a = (a + i*b) % bloom->bits;
        byte = x >> 3;
        c = bloom->bf[byte];
        mask = 1 << (x % 8);

        if(c & mask)
            hits++;
        else {
            if(add)
                bloom->bf[byte] = c | mask;
        }
    }

    if(hits == bloom->hashes)
        return 1;

    return 0;
}


int bloom_check(struct bloom *bloom, const void *buffer, int len)
{
    return bloom_check_add(bloom, buffer, len, 0);
}


int bloom_add(struct bloom *bloom, const void *buffer, int len)
{
    return bloom_check_add(bloom, buffer, len, 1);
}


void bloom_print(struct bloom *bloom)
{
    (void)printf("bloom at %p\n", (void *)bloom);
    (void)printf(" ->entries = %d\n", bloom->entries);
    (void)printf(" ->error = %f\n", bloom->error);
    (void)printf(" ->bits = %d\n", bloom->bits);
    (void)printf(" ->bytes = %d\n", bloom->bytes);
    (void)printf(" ->hash functions = %d\n", bloom->hashes);
    (void)printf(" ->bits per elem = %f\n", bloom->bpe);
}


void bloom_free(struct bloom *bloom)
{
    if(bloom->ready)
        free(bloom->bf);

    bloom->ready = 0;
}
