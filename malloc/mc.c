#include <unistd.h>
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include "mc.h"

/*扩展内存空间*/
#define MMAP (mmap(NULL, CHUNKSIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0))

/*head节点数据*/
#define BDSIZE (sizeof(BLOCK))
#define BSIZE(ptr) ((ptr)->head & ~0x7)
#define BSTATE(ptr) ((ptr)->head & 0x1)
#define SETSIZE(ptr, size) (ptr->head = PACK(size, BSTATE(ptr)))
#define SETSTATE(ptr, state) (ptr->head = PACK(BSIZE(ptr), state))
#define PACK(size, state) ((size) | (state))
#define BASIZE(ptr) (BSIZE(ptr) - BDSIZE)

/*设置一个块结构*/
#define PUT_BLOCK(ptr, s, st, p, n) SETSIZE(ptr, (s));\
SETSTATE(ptr, (st));\
(ptr)->pred = (p);\
(ptr)->next = (n);

struct block {
    size_t head; /*块大小+状态*/
    struct block *pred;
    struct block *next;
    size_t ext; /*占位符，保证字符对齐*/
};

typedef struct block BLOCK;

/*初始块指针*/
static unsigned int *global_ptr;
static BLOCK *block_list = NULL;

/*统计块数量，测试*/
void block_list_size() {
    int free_count = 0;
    int alloc_count = 0;
    int count = 0;
    BLOCK *current_ptr = block_list;
    while(current_ptr != NULL) {
        if (BSTATE(current_ptr)==0) {
            printf("free_size = %lu, index = %d\n", (BSIZE(current_ptr)), count);
            free_count++;
        }
        if (BSTATE(current_ptr)==1)
            alloc_count++;
        current_ptr = current_ptr->next;
        count++;
    }
    printf("free_count = %d, alloc_count = %d, all_count = %d\n", free_count, alloc_count, free_count+alloc_count);
}

/*块链表增删操作*/
static
BLOCK *remove_next(BLOCK *ptr) {
    BLOCK *tmp_ptr = ptr->next;
    ptr->next = tmp_ptr->next;

    if (ptr->next != NULL)
        ptr->next->pred = ptr;

    tmp_ptr->pred = NULL;
    tmp_ptr->next = NULL;
    return tmp_ptr;
}

static
BLOCK *remove_pred(BLOCK *ptr) {
    BLOCK *tmp_ptr = ptr->pred;
    ptr->pred = tmp_ptr->pred;

    if (ptr->pred != NULL)
        ptr->pred->next = ptr;

    tmp_ptr->pred = NULL;
    tmp_ptr->next = NULL;
    return tmp_ptr;
}

static
void remove_self(BLOCK *ptr) {
    if (ptr->pred != NULL)
        ptr->pred->next = ptr->next;

    if (ptr->next != NULL)
        ptr->next->pred = ptr->pred;

    ptr->pred = NULL;
    ptr->next = NULL;
}

static
void insert_next(BLOCK *ptr, BLOCK *i_ptr) {
    i_ptr->next = ptr->next;
    i_ptr->pred = ptr;
    ptr->next = i_ptr;

    if (i_ptr->next != NULL)
        i_ptr->next->pred = i_ptr;
}

static
void insert_pred(BLOCK *ptr, BLOCK *i_ptr) {
    i_ptr->next = ptr;
    i_ptr->pred = ptr->pred;
    ptr->pred = i_ptr;

    if (i_ptr->pred != NULL)
        i_ptr->pred->next = i_ptr;
}
/*块链表增删操作结束*/

/*合并空闲块*/
static
void mc_merge(BLOCK *ptr) {
    /*第一种情况-后块存在并且后块是空闲块*/
    if (ptr->next != NULL && BSTATE(ptr->next) == 0) {
        SETSIZE(ptr, BSIZE(ptr) + BSIZE(ptr->next));
        remove_self(ptr->next);
    }

    /*第二种情况-前块存在并且前块是空闲块*/
    if (ptr->pred != NULL && BSTATE(ptr->pred) == 0) {
        SETSIZE(ptr->pred, BSIZE(ptr) + BSIZE(ptr->pred));
        remove_self(ptr);
    }
}

/*分割空闲块*/
static
void mc_split(BLOCK *ptr, size_t size) {
    /*如果剩余空间大于一个块节点大小,进行块分割*/
    if (BASIZE(ptr) - size > BDSIZE) {
        BLOCK *split_ptr = (BLOCK *)(ptr + 1 + size/DSIZE);
        PUT_BLOCK(split_ptr, BASIZE(ptr) - size, 0, NULL, NULL);
        insert_next(ptr, split_ptr);
        SETSIZE(ptr, size + BDSIZE);
    }
    SETSTATE(ptr, 1);
}

/*扩展堆大小，返回新区域的初始地址指针*/
static
void expand_heap() {
    BLOCK *current_ptr = block_list;
    while(current_ptr->next != NULL) {
        current_ptr = current_ptr->next;
    }
    BLOCK *new_ptr = (BLOCK *)MMAP;
    PUT_BLOCK(new_ptr, CHUNKSIZE, 0, NULL, NULL);
    insert_next(current_ptr, new_ptr);
    mc_merge(new_ptr);
}

/*初始化*/
static
void mc_init() {
    global_ptr = (unsigned int *)MMAP;

    block_list = (BLOCK *)global_ptr;
    PUT_BLOCK(block_list, CHUNKSIZE, 0, NULL, NULL);
}

/*内存空间动态分配*/
/*策略：首次适配*/
void *mc_malloc(size_t size) {

    if (block_list == NULL)
        mc_init();

    /*字符对齐*/
    size = (size % DSIZE) == 0 ? size : (size - (size % DSIZE) + DSIZE);
    BLOCK *current_ptr = block_list;

    while(current_ptr != NULL) {
        /*当前块是空闲块并且可用空间大于申请空间*/
        if (BSTATE(current_ptr) == 0 && BASIZE(current_ptr) >= size) {
            /*转换分配*/
            mc_split(current_ptr, size);
            return (current_ptr + 1);
        }
        current_ptr = current_ptr->next;
    }

    /*如果没有找到合适空间，进行堆空间扩展*/
    expand_heap();

    /*递归再次进行空间分配*/
    return mc_malloc(size);
}


/*释放已分配块*/
void mc_free(void *ptr) {
    BLOCK *marker_ptr = ((BLOCK *)ptr) - 1;

    /*将当前块置为空闲块*/
    SETSTATE(marker_ptr, 0);

    /*合并*/
    mc_merge(marker_ptr);
}

