#ifdef malloc
#undef malloc
#endif

#ifdef calloc
#undef calloc
#endif

#ifdef realloc
#undef realloc
#endif

#ifdef free
#undef free
#endif

#include <stdlib.h>
#include <stdio.h>
#include "parametrized_output.h"

static int mallocs_counter = 0;
static int callocs_counter = 0;
static int reallocs_counter = 0;
static int free_counter = 0;
static int allocs_counter = 0;
static int max = 0;
static int current = 0;
static size_t memory = 0;
static size_t max_memory = 0;

#ifdef TRACK_ALLOCS
#define MAX_ALLOCS_COUNT 1000
static void *allocs[MAX_ALLOCS_COUNT];
static size_t sizes[MAX_ALLOCS_COUNT];
#endif

int find_alloc(void *ptr) {
#ifdef TRACK_ALLOCS
  int pos=0;
  while ((pos < current) && (allocs[pos] != ptr)) {
    pos++;
  }
  if (pos < current) {
    return pos;
  } else
    return -1;
#else
  return 0;
#endif
}

void remove_alloc(int pos) {
#ifdef TRACK_ALLOCS
  memory -= sizes[pos];
  debug("Alloc %p removed from %d (replaced by %d), size is %lu\n", allocs[pos], pos, current-1, memory);
  sizes[pos] = sizes[current-1];
  allocs[pos] = allocs[current-1];
#endif
  free_counter++;
  current--;
}

int add_alloc(void *ptr, size_t size) {
#ifdef TRACK_ALLOCS
  if (current < MAX_ALLOCS_COUNT) {
    allocs[current] = ptr;
    sizes[current] = size;
    memory += size;
    debug("Alloc %p added at %d, size is %lu\n", ptr, current, memory);
    if (memory > max_memory)
      max_memory = memory;
#endif
    allocs_counter++;
    current++;
    if (current > max)
      max = current;
    return 1;
#ifdef TRACK_ALLOCS
  } else {
    alert("Maximum number of allocations reached, %d\n", MAX_ALLOCS_COUNT);
    return 0;
  }
#endif
}

void *my_malloc(size_t size) {
#ifdef MALLOCS_FAIL_AFTER
  if (mallocs_counter == MALLOCS_FAIL_AFTER) {
    debug("Malloc artificial failure on %p\n", p);
    return NULL;
  }
#endif
  mallocs_counter++;
  void *result = malloc(size);
  if (result != NULL) {
    add_alloc(result, size);
  }
  return result;
}

void *my_calloc(size_t count, size_t size) {
#ifdef CALLOCS_FAIL_AFTER
  if (callocs_counter == CALLOCS_FAIL_AFTER) {
    debug("Calloc artificial failure on %p\n", p);
    return NULL;
  }
#endif
  callocs_counter++;
  void *result = calloc(count, size);
  if (result != NULL) {
    add_alloc(result, count*size);
  }
  return result;
}

void *my_realloc(void *p, size_t size) {
#ifdef REALLOCS_FAIL_AFTER
  if (reallocs_counter == REALLOCS_FAIL_AFTER) {
    debug("Realloc artificial failure on %p\n", p);
    return NULL;
  }
#endif
  reallocs_counter++;
  if (p == NULL) {
    void *result = realloc(p, size);
    if (result != NULL) {
      add_alloc(result, size);
    }
    return result;
  } else {
    int pos = find_alloc(p);
    if (pos != -1) {
      void *result = realloc(p, size);
      if (result != NULL) {
        remove_alloc(pos);
        add_alloc(result, size);
      }
      return result;
    } else {
      alert("Realloc called on an unknown pointer, %p\n", p);
      return NULL;
    }
  }
}

void my_free(void *ptr) {
  if (ptr != NULL) {
    int pos = find_alloc(ptr);
    if (pos != -1) {
      remove_alloc(pos);
      free(ptr);
    } else {
      alert("Free called on an unknown pointer, %p\n", ptr);
    }
  }
}

int current_allocs() {
  return current;
}

int total_allocs() {
  return allocs_counter;
}

size_t memory_used() {
  return memory;
}

size_t max_memory_used() {
  return max_memory;
}

int mallocs_done() {
  return mallocs_counter;
}

int callocs_done() {
  return callocs_counter;
}

int reallocs_done() {
  return reallocs_counter;
}
