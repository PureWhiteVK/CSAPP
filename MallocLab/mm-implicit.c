/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "No Team",
    /* First member's full name */
    "PureWhiteVK",
    /* First member's email address */
    "yl_xiao@outlook.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define ALIGNMENT_MASK                                                         \
  (~0x7) /* the corresponding alignment bit mask (~0x3 for 4 bytes alignment   \
            and ~0x7 for 8 bytes alignment) */

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ALIGNMENT_MASK)

#define WSIZE                                                                  \
  (ALIGN(sizeof(size_t)))    /* Word and header/footer size (bytes)            \
                              */
#define DSIZE (WSIZE * 2)    /* Doubld word size (bytes) */
#define CHUNK_SIZE (1 << 12) /* Extend heap by this amount (bytes) */

/* Pack a size and allocated bit into a size_t */
#define PACK(size, alloc) ((size) | (alloc))
/* Read and write a size_t at address p */
#define GET(p) (*(size_t *)(p))
#define PUT(p, val) (*(size_t *)(p) = (val))
/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ALIGNMENT_MASK)
#define GET_ALLOC(p) (GET(p) & 0x1)
/* Given block ptr bp, compute address of its header and footer */
#define HEADER(bp) ((char *)(bp)-WSIZE)
#define FOOTER(bp) ((char *)(bp) + GET_SIZE(HEADER(bp)) - DSIZE)
/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLOCK(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLOCK(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void *heap_listp;

#ifdef DEBUG
#define LOG(...) printf(__VA_ARGS__)

static void print_freelist() {
  // loop through heap_listp
  void *bp = heap_listp;
  printf("heap list [\n");
  printf("  %p~%p [size(header): %04zu,alloc bit(header): %zu], [size(footer): "
         "%04zu,alloc bit(footer): %zu] (prologue)\n",
         HEADER(bp), FOOTER(bp), GET_SIZE(HEADER(bp)), GET_ALLOC(HEADER(bp)),
         GET_SIZE(FOOTER(bp)), GET_ALLOC(FOOTER(bp)));
  for (bp = NEXT_BLOCK(bp); GET_SIZE(HEADER(bp)) > 0; bp = NEXT_BLOCK(bp)) {
    printf(
        "  %p~%p [size(header): %04zu,alloc bit(header): %zu], [size(footer): "
        "%04zu,alloc bit(footer): %zu]\n",
        HEADER(bp), FOOTER(bp), GET_SIZE(HEADER(bp)), GET_ALLOC(HEADER(bp)),
        GET_SIZE(FOOTER(bp)), GET_ALLOC(FOOTER(bp)));
  }
  printf("  %p~%p size: %04zu,alloc bit: %zu (epilogue)\n", HEADER(bp),
         HEADER(bp), GET_SIZE(HEADER(bp)), GET_ALLOC(HEADER(bp)));
  printf("]\n");
}
#else
#define LOG(...) {}
static void print_freelist() {}
#endif

/*
 * mm_init - initialize the malloc package.
 */

static void *coalesce(void *bp) {
  void *prev_block = PREV_BLOCK(bp);
  void *next_block = NEXT_BLOCK(bp);
  size_t prev_alloc = GET_ALLOC(HEADER(prev_block));
  size_t next_alloc = GET_ALLOC(HEADER(next_block));
  size_t size = GET_SIZE(HEADER(bp));

  if (prev_alloc && next_alloc) {
    LOG("prev is allocated, next is allocated\n");
    /* no-op */
  } else if (prev_alloc && !next_alloc) {
    LOG("prev is allocated, next is not allocated\n");
    size += GET_SIZE(HEADER(next_block));
    PUT(HEADER(bp), PACK(size, 0));
    PUT(FOOTER(next_block), PACK(size, 0));
  } else if (!prev_alloc && next_alloc) {
    LOG("prev is not allocated, next is allocated\n");
    size += GET_SIZE(HEADER(prev_block));
    PUT(HEADER(prev_block), PACK(size, 0));
    PUT(FOOTER(bp), PACK(size, 0));
    bp = prev_block;
  } else {
    LOG("prev is not allocated, next is not allocated\n");
    size += GET_SIZE(HEADER(prev_block)) + GET_SIZE(HEADER(next_block));
    PUT(HEADER(prev_block), PACK(size, 0));
    PUT(FOOTER(next_block), PACK(size, 0));
    bp = prev_block;
  }
  return bp;
}

static void *extend_heap(size_t size) {
  char *bp;
  size = ALIGN(size);
  if ((bp = mem_sbrk(size)) == (void *)-1) {
    return NULL;
  }

  /* Initialize free block header/footer and the epilogue header */
  PUT(HEADER(bp), PACK(size, 0));
  PUT(FOOTER(bp), PACK(size, 0));
  PUT(HEADER(NEXT_BLOCK(bp)), PACK(0, 1));

  /* Coalesce if the previous block was free */
  return coalesce(bp);
}

static void *find_fit(size_t size) {
  /* First-fit search */
  void *bp;

  for (bp = heap_listp; GET_SIZE(HEADER(bp)) > 0; bp = NEXT_BLOCK(bp)) {
    if (!GET_ALLOC(HEADER(bp)) && (size <= GET_SIZE(HEADER(bp)))) {
      return bp;
    }
  }
  return NULL; /* No fit */
}

static void place(void *bp, size_t size) {
  size_t block_size = GET_SIZE(HEADER(bp));
  if ((block_size - size) >= (2 * DSIZE)) {
    PUT(HEADER(bp), PACK(size, 1));
    PUT(FOOTER(bp), PACK(size, 1));
    bp = NEXT_BLOCK(bp);
    PUT(HEADER(bp), PACK(block_size - size, 0));
    PUT(FOOTER(bp), PACK(block_size - size, 0));
  } else {
    PUT(HEADER(bp), PACK(block_size, 1));
    PUT(FOOTER(bp), PACK(block_size, 1));
  }
  print_freelist();
}

int mm_init(void) {
  LOG("\nin %s\n", __func__);
  /* Create the initial empty heap */
  if ((heap_listp = mem_sbrk(3 * WSIZE)) == (void *)-1) {
    return -1;
  }
  /* Prologue header */
  PUT(heap_listp, PACK(DSIZE, 1));
  /* Prologue footer */
  PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
  /* Epilogue header */
  PUT(heap_listp + DSIZE, PACK(0, 1));
  // 设置 heap_listp 指向 prologue 的data部分（虽然实际上没有任何数据）
  heap_listp += WSIZE;

  print_freelist();

  /* Extend the empty heap with a free block of CHUNK_SIZE bytes */
  // if (extend_heap(CHUNK_SIZE) == NULL) {
  //   return -1;
  // }

  print_freelist();

  return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
  LOG("in %s, allocate %zu\n", __func__, size);
  size_t asize;       /* Adjusted block size */
  size_t extend_size; /* Amount to extend heap if no fit */
  char *bp;

  /* Ignore spurious requests */
  if (size == 0) {
    return NULL;
  }

  /* Adjust block size to include overhead and alignment reqs */
  asize = ALIGN(size + 2 * WSIZE);

  /* Searech the free list for a fit */
  if ((bp = find_fit(asize)) != NULL) {
    place(bp, asize);
    return bp;
  }

  /* No fit found. Get more memory and place the block */
  extend_size = MAX(asize, CHUNK_SIZE);
  if ((bp = extend_heap(extend_size)) == NULL) {
    return NULL;
  }
  place(bp, asize);
  return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
  if (ptr > mem_heap_hi() || ptr < mem_heap_lo()) {
    return;
  }
  size_t size = GET_SIZE(HEADER(ptr));
  LOG("in %s, free %p~%p,size: %zu\n ", __func__, HEADER(ptr), FOOTER(ptr),
      size);
  print_freelist();
  PUT(HEADER(ptr), PACK(size, 0));
  PUT(FOOTER(ptr), PACK(size, 0));
  print_freelist();
  coalesce(ptr);
  print_freelist();
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
  void *oldptr = ptr;
  void *newptr;
  size_t copySize;

  newptr = mm_malloc(size);
  if (newptr == NULL)
    return NULL;
  copySize = *(size_t *)((char *)oldptr - WSIZE);
  if (size < copySize)
    copySize = size;
  memcpy(newptr, oldptr, copySize);
  mm_free(oldptr);
  return newptr;
}
