#ifndef __MC_H__
#define __MC_H__
/*字符对齐大小*/
#define DSIZE 0x20
/*每次扩展内存空间大小*/
#define CHUNKSIZE (1<<12)
extern void *mc_malloc(size_t);
extern void mc_free(void *);
extern void block_list_size();
#endif
