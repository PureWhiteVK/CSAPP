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

typedef struct ListNode *node_ptr;

struct ListNode {
  node_ptr prev;
  node_ptr next;
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define ALIGNMENT_MASK (~0x7)

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ALIGNMENT_MASK)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define WSIZE SIZE_T_SIZE
#define DSIZE (2 * SIZE_T_SIZE)
#define NODE_SIZE (sizeof(struct ListNode))
#define MIN_BLOCK_SIZE (NODE_SIZE + DSIZE)
#define EPILOGUE_BLOCK_SIZE (NODE_SIZE + WSIZE)

#define CHUNK_SIZE (mem_pagesize())

#define ALLOC_BIT 0x1

#define PACK(size, alloc) ((size) | (alloc))
#define GET(p) (*(size_t *)(p))
#define SET(p, val) (GET(p) = (val))
#define GET_SIZE(p) (GET(p) & ALIGNMENT_MASK)
#define GET_ALLOC(p) (GET(p) & ALLOC_BIT)
#define SET_SIZE(p, size)                                                      \
  (GET(p) = (((size)&ALIGNMENT_MASK) | (GET(p) & ALLOC_BIT)))
#define SET_ALLOC(p, alloc)                                                    \
  (GET(p) = ((GET(p) & ALIGNMENT_MASK) | ((alloc)&ALLOC_BIT)))
#define HEADER(bp) ((char *)(bp)-WSIZE)
#define FOOTER(bp) ((char *)(bp) + GET_SIZE(HEADER(bp)) - DSIZE)
#define NEXT_BLOCK(bp) ((char *)(bp) + GET_SIZE(HEADER(bp)))
#define PREV_BLOCK(bp) ((char *)(bp)-GET_SIZE((HEADER(bp) - WSIZE)))
#define NODE(bp) ((node_ptr)(bp))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static node_ptr freelist_header;

// #define DEBUG

#ifdef DEBUG
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)                                                               \
  {}
#endif

#define ATTACH(prev_node, node)                                                \
  do {                                                                         \
    node_ptr next = prev_node->next;                                           \
    prev_node->next = node;                                                    \
    node->prev = prev_node;                                                    \
    node->next = next;                                                         \
    next->prev = node;                                                         \
  } while (0)

#define DETACH(node)                                                           \
  do {                                                                         \
    node_ptr prev = node->prev;                                                \
    node_ptr next = node->next;                                                \
    prev->next = next;                                                         \
    next->prev = prev;                                                         \
  } while (0)

static void *coalesce(void *bp);
static void *extend_heap(size_t size);
static void *find_fit(size_t size);
static void place(void *bp, size_t size);
static const char *to_string(void *bp);
static void print_freelist();
static void print_heaplist();

// only used in debugger
static void *header(void *bp) { return HEADER(bp); }
static void *footer(void *bp) { return FOOTER(bp); }
static size_t get_size(void *bp) { return GET_SIZE(HEADER(bp)); }
static size_t get_alloc(void *bp) { return GET_ALLOC(HEADER(bp)); }

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
  LOG("\nin %s\n", __func__);
  /* Create the initial empty heap */
  size_t initial_size = MIN_BLOCK_SIZE + EPILOGUE_BLOCK_SIZE;
  char *bp;
  if ((bp = (char *)mem_sbrk(initial_size)) == (char *)-1) {
    return -1;
  }
  char *prologue_block = bp + WSIZE;
  SET(HEADER(prologue_block), PACK(MIN_BLOCK_SIZE, 1));
  SET(FOOTER(prologue_block), PACK(MIN_BLOCK_SIZE, 1));
  char *epilogue_block = NEXT_BLOCK(prologue_block);
  SET(HEADER(epilogue_block), PACK(0, 1));
  node_ptr header = NODE(prologue_block);
  node_ptr tail = NODE(epilogue_block);
  header->prev = NULL;
  header->next = tail;
  tail->prev = header;
  tail->next = NULL;
  freelist_header = header;

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
  void *bp;

  /* Ignore spurious requests */
  if (size == 0) {
    return NULL;
  }

  /* Adjust block size to include overhead and alignment reqs */
  asize = ALIGN(size + DSIZE);

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
  if (ptr > mem_heap_hi() || ptr < mem_heap_lo()) {
    return;
  }
  size_t size = GET_SIZE(HEADER(ptr));
  LOG("in %s, with block = %s,size: %zu\n ", __func__, to_string(ptr), size);
  print_heaplist();
  print_freelist();
  SET(HEADER(ptr), PACK(size, 0));
  SET(FOOTER(ptr), PACK(size, 0));
  ATTACH(freelist_header, NODE(ptr));
  print_heaplist();
  print_freelist();
  coalesce(ptr);
  print_heaplist();
  print_freelist();
  LOG("end %s\n", __func__);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
  LOG("in %s, with block = %s, size = %zu\n", __func__, to_string(ptr), size);
  if (ptr == NULL) {
    LOG("original ptr is NULL, perform mm_malloc\n");
    return mm_malloc(size);
  }
  if (size == 0) {
    LOG("size is 0, perform mm_free\n");
    mm_free(ptr);
    return NULL;
  }

  size_t curr_size = GET_SIZE(HEADER(ptr));
  size_t total_size = curr_size;
  size_t payload_size = ALIGN(size + DSIZE);
  if (total_size >= payload_size) {
    LOG("original size is big enough, try shrink\n");
    print_freelist();
    print_heaplist();
    // shrink curr block to size (or should we leave some )
    if (total_size >= payload_size + MIN_BLOCK_SIZE) {
      LOG("shrink from %06zu to %06zu\n", total_size, payload_size);
      // create free block
      SET(HEADER(ptr), PACK(payload_size, 1));
      SET(HEADER(ptr), PACK(payload_size, 1));
      void *free_block = NEXT_BLOCK(ptr);
      SET(HEADER(free_block), PACK(total_size - payload_size, 0));
      SET(FOOTER(free_block), PACK(total_size - payload_size, 0));
      ATTACH(freelist_header, NODE(free_block));
    }
    print_freelist();
    print_heaplist();
    return ptr;
  }
  void *next_block = NEXT_BLOCK(ptr);
  size_t next_alloc = GET_ALLOC(HEADER(next_block));
  size_t next_size = GET_SIZE(HEADER(next_block));
  if (!next_alloc && curr_size + next_size >= payload_size) {
    LOG("next block is not allocated and total is big enough, expand to it\n");
    total_size += next_size;
    print_freelist();
    print_heaplist();
    DETACH(NODE(next_block));
    if (total_size >= payload_size + MIN_BLOCK_SIZE) {
      SET(HEADER(ptr), PACK(payload_size, 1));
      SET(FOOTER(ptr), PACK(payload_size, 1));
      void *free_block = NEXT_BLOCK(ptr);
      SET(HEADER(free_block), PACK(total_size - payload_size, 0));
      SET(FOOTER(free_block), PACK(total_size - payload_size, 0));
      ATTACH(freelist_header, NODE(free_block));
    } else {
      SET(HEADER(ptr), PACK(total_size, 1));
      SET(FOOTER(ptr), PACK(total_size, 1));
    }
    print_freelist();
    print_heaplist();
    return ptr;
  }
  void *prev_block = PREV_BLOCK(ptr);
  size_t prev_alloc = GET_ALLOC(HEADER(prev_block));
  size_t prev_size = GET_SIZE(HEADER(prev_block));
  if (!prev_alloc && prev_size + curr_size >= payload_size) {
    LOG("prev block is not allocated and total is big enough, expand to it\n");
    total_size += prev_size;
    print_freelist();
    print_heaplist();
    DETACH(NODE(prev_block));
    // the memory may be overlapped
    // 必须先 move 再 set header 和 footer，否则可能会把数据给替换掉
    memmove(prev_block, ptr, curr_size - DSIZE);
    if (total_size >= payload_size + MIN_BLOCK_SIZE) {
      SET(HEADER(prev_block), PACK(payload_size, 1));
      SET(FOOTER(prev_block), PACK(payload_size, 1));
      void *free_block = NEXT_BLOCK(prev_block);
      SET(HEADER(free_block), PACK(total_size - payload_size, 0));
      SET(FOOTER(free_block), PACK(total_size - payload_size, 0));
      ATTACH(freelist_header, NODE(free_block));
    } else {
      SET(HEADER(prev_block), PACK(total_size, 1));
      SET(FOOTER(prev_block), PACK(total_size, 1));
    }
    print_freelist();
    print_heaplist();
    return prev_block;
  }

  if (!prev_alloc && !next_alloc &&
      prev_size + curr_size + next_size >= payload_size) {
    LOG("prev block is not allocated, next block is not allocated and total is "
        "big enough, expand to it\n");
    total_size += prev_size + next_size;
    print_freelist();
    print_heaplist();
    DETACH(NODE(prev_block));
    DETACH(NODE(next_block));
    memmove(prev_block, ptr, curr_size - WSIZE);
    if (total_size >= payload_size + MIN_BLOCK_SIZE) {
      SET(HEADER(prev_block), PACK(payload_size, 1));
      SET(FOOTER(prev_block), PACK(payload_size, 1));
      void *free_block = NEXT_BLOCK(prev_block);
      SET(HEADER(free_block), PACK(total_size - payload_size, 0));
      SET(FOOTER(free_block), PACK(total_size - payload_size, 0));
      ATTACH(freelist_header, NODE(free_block));
    } else {
      SET(HEADER(prev_block), PACK(total_size, 1));
      SET(FOOTER(prev_block), PACK(total_size, 1));
    }
    print_freelist();
    print_heaplist();
    return prev_block;
  }

  LOG("no adjacent free block available, perform mm_malloc\n");
  void *newptr;
  newptr = mm_malloc(size);
  if (newptr == NULL)
    return NULL;
  memcpy(newptr, ptr, curr_size - DSIZE);
  mm_free(ptr);
  return newptr;
}

//////////////////////////////////////////////////////////////////////////////////////

void *coalesce(void *bp) {
  LOG("in %s, with block = %s\n", __func__, to_string(bp));

  print_heaplist();
  print_freelist();

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
    // detach next block
    DETACH(NODE(bp));
    DETACH(NODE(next_block));
    size += GET_SIZE(HEADER(next_block));
    SET(HEADER(bp), PACK(size, 0));
    SET(FOOTER(next_block), PACK(size, 0));
    // we have to detach node and
    ATTACH(freelist_header, NODE(bp));
  } else if (!prev_alloc && next_alloc) {
    LOG("prev is not allocated, next is allocated\n");
    DETACH(NODE(prev_block));
    DETACH(NODE(bp));
    size += GET_SIZE(HEADER(prev_block));
    SET(HEADER(prev_block), PACK(size, 0));
    SET(FOOTER(bp), PACK(size, 0));
    bp = prev_block;
    ATTACH(freelist_header, NODE(bp));
  } else {
    LOG("prev is not allocated, next is not allocated\n");
    DETACH(NODE(prev_block));
    DETACH(NODE(bp));
    DETACH(NODE(next_block));
    size += GET_SIZE(HEADER(prev_block)) + GET_SIZE(HEADER(next_block));
    SET(HEADER(prev_block), PACK(size, 0));
    SET(FOOTER(next_block), PACK(size, 0));
    bp = prev_block;
    ATTACH(freelist_header, NODE(bp));
  }

  print_heaplist();
  print_freelist();

  LOG("end %s\n", __func__);

  return bp;
}

void *extend_heap(size_t size) {
  LOG("in %s with size = %zu\n", __func__, size);

  print_heaplist();
  print_freelist();

  char *bp;
  size = ALIGN(size);
  if ((bp = mem_sbrk(size)) == (char *)-1) {
    return NULL;
  }

  bp = bp - NODE_SIZE;
  node_ptr epilogue_prev = NODE(bp)->prev;

  /* Initialize free block header/footer and the epilogue header */
  SET(HEADER(bp), PACK(size, 0));
  SET(FOOTER(bp), PACK(size, 0));
  // setup epilogue
  void *epilogue = NEXT_BLOCK(bp);
  SET(HEADER(epilogue), PACK(0, 1));
  epilogue_prev->next = NODE(epilogue);
  NODE(epilogue)->prev = epilogue_prev;
  NODE(epilogue)->next = NULL;

  ATTACH(freelist_header, NODE(bp));

  print_heaplist();
  print_freelist();

  /* Coalesce if the previous block was free */
  return coalesce(bp);
}

void *find_fit(size_t size) {
  void *bp = freelist_header->next;
  while (bp && GET_SIZE(HEADER(bp)) < size) {
    bp = NODE(bp)->next;
  }
  return bp;
}

void place(void *bp, size_t size) {
  LOG("in %s with block = %s, size = %zu\n", __func__, to_string(bp), size);
  print_freelist();
  print_heaplist();
  DETACH(NODE(bp));
  size_t block_size = GET_SIZE(HEADER(bp));
  if ((block_size - size) >= MIN_BLOCK_SIZE) {
    SET(HEADER(bp), PACK(size, 1));
    SET(FOOTER(bp), PACK(size, 1));
    bp = NEXT_BLOCK(bp);
    SET(HEADER(bp), PACK(block_size - size, 0));
    SET(FOOTER(bp), PACK(block_size - size, 0));
    ATTACH(freelist_header, NODE(bp));
  } else {
    SET(HEADER(bp), PACK(block_size, 1));
    SET(FOOTER(bp), PACK(block_size, 1));
  }
  print_freelist();
  print_heaplist();
}

const char *to_string(void *bp) {
  static char buffer[2048];
  if (GET_SIZE(HEADER(bp)) == 0) {
    sprintf(buffer, "%p~%p H(S=%06zu,A=%zu)", HEADER(bp), HEADER(bp),
            GET_SIZE(HEADER(bp)), GET_ALLOC(HEADER(bp)));
  } else {
    sprintf(buffer, "%p~%p H(S=%06zu,A=%zu) F(S=%06zu,A=%zu)", HEADER(bp),
            FOOTER(bp), GET_SIZE(HEADER(bp)), GET_ALLOC(HEADER(bp)),
            GET_SIZE(FOOTER(bp)), GET_ALLOC(FOOTER(bp)));
  }
  return buffer;
}

#ifdef DEBUG

void print_heaplist() {
  void *bp;
  printf("Heap List [\n");
  for (bp = freelist_header; GET_SIZE(HEADER(bp)) != 0; bp = NEXT_BLOCK(bp)) {
    printf("  %s\n", to_string(bp));
  }
  printf("  %s\n", to_string(bp));
  printf("]\n");
}

void print_freelist() {
  printf("Free List [\n");
  node_ptr curr;
  for (curr = freelist_header; curr->next != NULL; curr = curr->next) {
    printf("  %s\n", to_string(curr));
  }
  printf("  %s\n", to_string(curr));
  printf("]\n");
}

#else
static void print_heaplist() {}
static void print_freelist() {}
#endif