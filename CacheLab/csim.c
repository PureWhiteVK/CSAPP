#include "cachelab.h"

#include <assert.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PATH_LEN 1024
#define BUFFER_SIZE 1024

int verbose = 0;

void print_usage(FILE *f, const char *exe_name) {
  fprintf(f, "Usgae: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n",
          exe_name);
  fprintf(f, "Options:\n");
  fprintf(f, "  -h         Print this help message.\n");
  fprintf(f, "  -v         Optional verbose flag.\n");
  fprintf(f, "  -s <num>   Number of set index bits.\n");
  fprintf(f, "  -E <num>   Number of lines per set.\n");
  fprintf(f, "  -b <num>   Number of block offset bits.\n");
  fprintf(f, "  -t <file>  Trace file.\n");
  fprintf(f, "\n");
  fprintf(f, "Examples:\n");
  fprintf(f, "  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", exe_name);
  fprintf(f, "  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", exe_name);
  fflush(f);
}

// implement LRU cache here

/*
Each line denotes one or two memory accesses.

The format of each line is [space]operation address,size

The operation field denotes the type of memory access:
“I” denotes an instruction load, “L” a data load,“S” a data store, and “M” a
data modify (i.e., a data load followed by a data store). There is never a space
before each “I”. There is always a space before each “M”, “L”, and “S”. The
address field specifies a 64-bit hexadecimal memory address. The size field
specifies the number of bytes accessed by the operation.
*/

typedef struct {
  size_t valid;
  size_t tag;
  size_t timestamp;
  // data size is specified in b
  // void *data;
} cache_block;

typedef struct {
  size_t count;
  size_t n_hits;
  size_t n_misses;
  size_t n_evictions;
  size_t s;
  size_t E;
  size_t b;
  size_t n_sets;
  size_t n_blocks;
  size_t block_size;
  // 对于每一个 block 我们需要存储一个 valid bit、tag
  // field、timestamp（供LRU使用）、数据字段
  cache_block **sets;
} lru_cache;

void create_lru_cache(lru_cache *cache, size_t s, size_t E, size_t b) {
  // using malloc to create blocks
  // 直接计算出所需要的 block 个数
  cache->s = s;
  cache->E = E;
  cache->b = b;
  cache->n_sets = 1 << s;
  cache->n_blocks = E;
  cache->block_size = 1 << b;
  cache->count = 0;
  cache->n_evictions = 0;
  cache->n_hits = 0;
  cache->n_misses = 0;
  // 分配空间
  cache->sets = malloc(cache->n_sets * sizeof(cache_block *));
  for (size_t i = 0; i < cache->n_sets; i++) {
    cache->sets[i] = calloc(cache->n_blocks, sizeof(cache_block));
  }
}

void destroy_lru_cache(lru_cache *cache) {
  for (size_t i = 0; i < cache->n_sets; i++) {
    free(cache->sets[i]);
  }
  free(cache->sets);
}

const char *to_binary_string(uintptr_t data) {
  static char buffer[BUFFER_SIZE];
  memset(buffer, '\0', BUFFER_SIZE);
  sprintf(buffer, "0b");
  ssize_t bit = 64;
  size_t i = 2;
  while (bit-- > 0) {
    buffer[i++] = ((data >> bit) & 0x1) + '0';
  }
  return buffer;
}

void access_cache(lru_cache *cache, char operation, uintptr_t address,
                  size_t size) {
  cache->count++;
  uintptr_t tag = (address >> (cache->s + cache->b)) &
                  ((1 << (64 - (cache->s + cache->b))) - 1);
  uintptr_t set_index = (address >> cache->b) & ((1 << cache->s) - 1);

  // 读取一下数据
  cache_block *set = cache->sets[set_index];
  int miss = 0;
  int hit = 0;
  int eviction = 0;
  // search in sets
  for (size_t i = 0; i < cache->n_blocks; i++) {
    if (set[i].valid && set[i].tag == tag) {
      // cache hit
      hit = 1;
      set[i].timestamp = cache->count;
      break;
    }
  }
  if (!hit) {
    // cache miss
    miss = 1;
    eviction = 1;
    size_t i = 0;
    size_t min_timestamp = -1;
    size_t min_i = 0;
    while (i < cache->n_blocks) {
      if (!set[i].valid) {
        // no need to perform eviction
        eviction = 0;
        min_i = i;
        break;
      }
      // find minimum timestamp
      if (min_timestamp > set[i].timestamp) {
        min_i = i;
        min_timestamp = set[i].timestamp;
      }
      i++;
    }
    // update data
    set[min_i].valid = 1;
    set[min_i].tag = tag;
    set[min_i].timestamp = cache->count;
  }
  cache->n_hits += hit;
  cache->n_misses += miss;
  cache->n_evictions += eviction;
  if (operation == 'M') {
    // one more cache hit for modify
    cache->n_hits++;
  }
  if (verbose) {
    switch (operation) {
    case 'L':
    case 'S':
      printf("%c %lx,%zu%s%s\n", operation, address, size,
             miss ? " miss" : " hit", eviction ? " eviction" : "");
      break;
    case 'M':
      printf("M %lx,%zu%s%s hit\n", address, size, miss ? " miss" : " hit",
             eviction ? "eviction" : "");
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  int opt;
  int s = -1, E = -1, b = -1;
  const char *trace_file_path;
  // max file lenthgh is 1024, last one for '\0'
  while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
    switch (opt) {
    case 'h':
      print_usage(stdout, argv[0]);
      exit(EXIT_SUCCESS);
    case 'v':
      verbose = 1;
      break;
    case 's':
      s = atoi(optarg);
      break;
    case 'E':
      E = atoi(optarg);
      break;
    case 'b':
      b = atoi(optarg);
      break;
    case 't':
      // 判断一下是否拷贝完毕
      trace_file_path = optarg;
      break;
    default: /* '?' */
      print_usage(stderr, argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  // check missint command line argument
  if (s == -1 || E == -1 || b == -1 || strlen(trace_file_path) == 0) {
    fprintf(stderr, "%s: Missing required command line argument\n", argv[0]);
    print_usage(stderr, argv[0]);
    exit(EXIT_FAILURE);
  }

  FILE *f = fopen(trace_file_path, "r");
  if (f == NULL) {
    fprintf(stderr, "%s: No such file or directory\n", trace_file_path);
    exit(EXIT_FAILURE);
  }

  char buffer[BUFFER_SIZE];
  size_t bytes_read;
  uintptr_t address = 0;
  size_t size = 0;
  char operation = '\0';

  enum State { init, data_op, sp, addr, sz, err } state = init;

  size_t lineno = 1;

  lru_cache cache;

  create_lru_cache(&cache, s, E, b);

  while ((bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, f))) {
    // run state machine
    for (size_t i = 0; i < bytes_read; i++) {
      char ch = buffer[i];
      switch (state) {
      case init:
        if (ch == 'I') {
          operation = ch;
          state = sp;
        } else if (ch == ' ') {
          state = data_op;
        } else {
          state = err;
        }
        break;
      case data_op:
        if (ch == 'S' || ch == 'L' || ch == 'M') {
          operation = ch;
          state = sp;
        } else {
          state = err;
        }
        break;
      case sp:
        if (ch == ' ') {
          // no-op
        } else if (ch >= '0' && ch <= '9') {
          address = (ch - '0');
          state = addr;
        } else if (ch >= 'a' && ch <= 'f') {
          address = (10 + ch - 'a');
          state = addr;
        } else {
          state = err;
        }
        break;
      case addr:
        if (ch >= '0' && ch <= '9') {
          address = (address << 4) + (ch - '0');
        } else if (ch >= 'a' && ch <= 'f') {
          address = (address << 4) + (10 + ch - 'a');
        } else if (ch == ',') {
          size = 0;
          state = sz;
        } else {
          state = err;
        }
        break;
      case sz:
        if (ch >= '0' && ch <= '9') {
          size = size * 10 + (ch - '0');
        } else if (ch == '\r' || ch == ' ') {
          // no-op
        } else if (ch == '\n') {
          lineno++;
          if (operation != 'I') {
            access_cache(&cache, operation, address, size);
          }
          state = init;
        } else {
          state = err;
        }
        break;
      case err:
        fprintf(stderr, "parse error at %s:%lu\n", trace_file_path, lineno);
        destroy_lru_cache(&cache);
        exit(EXIT_FAILURE);
      }
    }
  }
  printSummary(cache.n_hits, cache.n_misses, cache.n_evictions);
  destroy_lru_cache(&cache);
  return 0;
}
