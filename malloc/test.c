#include <stddef.h>
#include <stdio.h>
#include "mc.h"

struct test {
    int list[3][3];
    long mm;
    char *n;
};

struct test2 {
    int list[1][1];
    long mm;
    char *n;
};

int main() {
    struct test **list;
    for (int i=0; i<50; i++) {
        block_list_size();
        list[i] = mc_malloc(sizeof(struct test));
    }
    mc_free(list[15]);
    block_list_size();
    mc_free(list[32]);
    block_list_size();
    mc_free(list[33]);
    block_list_size();
    mc_free(list[14]);
    block_list_size();
    for (int j=0; j<50; j++) {
        block_list_size();
        list[j] = mc_malloc(sizeof(struct test));
    }
}

