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
#define ALIGNMENT_MASK (~0x7) /* the coresponding alignemtn bit mask */

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ALIGNMENT_MASK)

#define WSIZE                                                                  \
  (ALIGN(sizeof(size_t)))    /* Word and header/footer size (bytes)            \
                              */
#define DSIZE (WSIZE * 2)    /* Doubld word size (bytes) */
#define CHUNK_SIZE (1 << 12) /* Extend heap by this amount (bytes) */

/* Pack a size , previous allocated bit and allocated bit into header / footer
 */
#define PREV_ALLOCATED 0x2
#define NOT_ALLOCATED 0x0
#define ALLOCATED 0x1
#define PACK(size, prev_alloc, alloc) ((size) | (prev_alloc) | (alloc))
/* Read and write a size_t at address p */
#define GET(p) (*(size_t *)(p))
#define PUT(p, val) (*(size_t *)(p) = (val))
/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ALIGNMENT_MASK)
#define GET_PREV(p) (GET(p) & 0x2)
#define GET_ALLOC(p) (GET(p) & 0x1)
/* Given block ptr bp, compute address of its header and footer */
#define HEADER(bp) ((char *)(bp)-WSIZE)
#define FOOTER(bp) ((char *)(bp) + GET_SIZE(HEADER(bp)) - DSIZE)
/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLOCK(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLOCK(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static void *heap_listp;

// #define DEBUG 1

#ifdef DEBUG
#define LOG(...) printf(__VA_ARGS__)

static void print_freelist() {
  // loop through heap_listp
  void *bp = heap_listp;
  void *header = HEADER(bp);
  printf("Implicit Free Lists [\n");
  printf("  %p~%p Header(Size=%04zu,Prev=%zu,Alloc=%zu)\n", header, FOOTER(bp),
         GET_SIZE(header), GET_PREV(header) >> 1, GET_ALLOC(header));
  for (bp = NEXT_BLOCK(bp); GET_SIZE(HEADER(bp)) > 0; bp = NEXT_BLOCK(bp)) {
    header = HEADER(bp);
    if (GET_ALLOC(header)) {
      // only header
      printf("  %p~%p Header(Size=%04zu,Prev=%zu,Alloc=%zu)\n", header,
             FOOTER(bp), GET_SIZE(header), GET_PREV(header) >> 1,
             GET_ALLOC(header));
    } else {
      // header + footer
      void *footer = FOOTER(bp);
      printf("  %p~%p Header(Size=%04zu,Prev=%zu,Alloc=%zu) "
             "Footer(Size=%04zu,Prev=%zu,Alloc=%zu)\n",
             header, FOOTER(bp), GET_SIZE(header), GET_PREV(header) >> 1,
             GET_ALLOC(header), GET_SIZE(footer), GET_PREV(footer) >> 1,
             GET_ALLOC(footer));
      fflush(stdout);
      assert(GET_SIZE(footer) == GET_SIZE(header));
    }
  }
  header = HEADER(bp);
  printf("  %p~%p Header(Size=%04zu,Prev=%zu,Alloc=%zu)\n", header, header,
         GET_SIZE(header), GET_PREV(header) >> 1, GET_ALLOC(header));
  printf("]\n");
}
#else
#define LOG(...) 0
static void print_freelist() {}
#endif

/*
 * mm_init - initialize the malloc package.
 */

static void *coalesce(void *bp) {
  LOG("in %s\n", __func__);
  print_freelist();

  size_t prev_alloc = GET_PREV(HEADER(bp));
  size_t next_alloc = GET_ALLOC(HEADER(NEXT_BLOCK(bp)));
  size_t size = GET_SIZE(HEADER(bp));

  if (prev_alloc && next_alloc) {
    LOG("prev is allocated, next is allocated\n");
    /* no-op */
  } else if (prev_alloc && !next_alloc) {
    LOG("prev is allocated, next is not allocated\n");
    size += GET_SIZE(HEADER(NEXT_BLOCK(bp)));
    PUT(HEADER(bp), PACK(size, PREV_ALLOCATED, NOT_ALLOCATED));
    PUT(FOOTER(bp), PACK(size, PREV_ALLOCATED, NOT_ALLOCATED));
  } else if (!prev_alloc && next_alloc) {
    LOG("prev is not allocated, next is allocated\n");
    void *prev_header = HEADER(PREV_BLOCK(bp));
    size += GET_SIZE(prev_header);
    PUT(prev_header, PACK(size, GET_PREV(prev_header), NOT_ALLOCATED));
    PUT(FOOTER(bp), PACK(size, GET_PREV(prev_header), NOT_ALLOCATED));
    bp = PREV_BLOCK(bp);
  } else {
    LOG("prev is not allocated, next is not allocated\n");
    void *prev_header = HEADER(PREV_BLOCK(bp));
    size += GET_SIZE(prev_header) + GET_SIZE(HEADER(NEXT_BLOCK(bp)));
    PUT(HEADER(PREV_BLOCK(bp)),
        PACK(size, GET_PREV(prev_header), NOT_ALLOCATED));
    PUT(FOOTER(NEXT_BLOCK(bp)),
        PACK(size, GET_PREV(prev_header), NOT_ALLOCATED));
    bp = PREV_BLOCK(bp);
  }
  print_freelist();
  LOG("end %s\n", __func__);
  return bp;
}

static void *extend_heap(size_t size) {
  LOG("in %s with size = %zu\n", __func__, size);
  void *bp;
  size = ALIGN(size);
  if ((bp = mem_sbrk(size)) == (void *)-1) {
    return NULL;
  }

  print_freelist();

  /* Initialize free block header/footer and the epilogue header */
  PUT(HEADER(bp), PACK(size, GET_PREV(HEADER(bp)), NOT_ALLOCATED));
  PUT(FOOTER(bp), PACK(size, GET_PREV(HEADER(bp)), NOT_ALLOCATED));
  PUT(HEADER(NEXT_BLOCK(bp)), PACK(0, NOT_ALLOCATED, ALLOCATED));

  print_freelist();
  LOG("end %s\n", __func__);
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
  LOG("in %s with size = %zu\n", __func__, size);
  print_freelist();
  size_t block_size = GET_SIZE(HEADER(bp));
  if ((block_size - size) > DSIZE) {
    PUT(HEADER(bp), PACK(size, GET_PREV(HEADER(bp)), ALLOCATED));
    bp = NEXT_BLOCK(bp);
    PUT(HEADER(bp), PACK(block_size - size, PREV_ALLOCATED, NOT_ALLOCATED));
    PUT(FOOTER(bp), PACK(block_size - size, PREV_ALLOCATED, NOT_ALLOCATED));
  } else {
    PUT(HEADER(bp), PACK(block_size, GET_PREV(HEADER(bp)), ALLOCATED));
    void *next_header = HEADER(NEXT_BLOCK(bp));
    // 还需要同步更新next block的信息
    PUT(next_header,
        PACK(GET_SIZE(next_header), PREV_ALLOCATED, GET_ALLOC(next_header)));
    if (!GET_ALLOC(next_header)) {
      PUT(FOOTER(NEXT_BLOCK(bp)),
          PACK(GET_SIZE(next_header), PREV_ALLOCATED, GET_ALLOC(next_header)));
    }
  }

  print_freelist();
  LOG("end %s\n", __func__);
}

int mm_init(void) {
  LOG("\nin %s\n", __func__);
  /* Create the initial empty heap */
  if ((heap_listp = mem_sbrk(DSIZE)) == (void *)-1) {
    return -1;
  }
  /* Prologue header */
  PUT(heap_listp, PACK(WSIZE, PREV_ALLOCATED, ALLOCATED));
  // /* Prologue footer */
  // PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
  /* Epilogue header */
  PUT(heap_listp + WSIZE, PACK(0, PREV_ALLOCATED, ALLOCATED));
  // 设置 heap_listp 指向 prologue 的data部分（虽然实际上没有任何数据）
  heap_listp += WSIZE;

  LOG("init Implicit Free Lists\n");
  print_freelist();

  /* Extend the empty heap with a free block of CHUNK_SIZE bytes */
  if (extend_heap(CHUNK_SIZE) == NULL) {
    return -1;
  }

  LOG("end %s\n", __func__);
  return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
  LOG("in %s with size = %zu\n", __func__, size);
  size_t asize;       /* Adjusted block size */
  size_t extend_size; /* Amount to extend heap if no fit */
  char *bp;

  /* Ignore spurious requests */
  if (size == 0) {
    return NULL;
  }

  /* Adjust block size to include overhead and alignment reqs */
  asize = ALIGN(size + WSIZE);

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
  LOG("end %s\n", __func__);
  return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
  // we have to check whether the ptr is valid ?
  if (ptr > mem_heap_hi() || ptr < mem_heap_lo()) {
    return;
  }
  size_t size = GET_SIZE(HEADER(ptr));
  LOG("in %s, free %p~%p,size: %zu\n ", __func__, HEADER(ptr), FOOTER(ptr),
      size);
  print_freelist();
  PUT(HEADER(ptr), PACK(size, GET_PREV(HEADER(ptr)), NOT_ALLOCATED));
  PUT(FOOTER(ptr), PACK(size, GET_PREV(HEADER(ptr)), NOT_ALLOCATED));
  // set next prev
  void *next_header = HEADER(NEXT_BLOCK(ptr));
  PUT(next_header,
      PACK(GET_SIZE(next_header), NOT_ALLOCATED, GET_ALLOC(next_header)));
  if (!GET_ALLOC(next_header)) {
    PUT(FOOTER(NEXT_BLOCK(ptr)),
        PACK(GET_SIZE(next_header), NOT_ALLOCATED, GET_ALLOC(next_header)));
  }
  coalesce(ptr);
  LOG("end %s\n", __func__);
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
