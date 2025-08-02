/* Enhanced memory checking macros for WaoN
 * Copyright (C) 2007-2024 Kengo Ichiki <kichiki@users.sourceforge.net>
 * $Id: memory-check.h,v 1.2 2024/01/01 00:00:00 kichiki Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef	_MEMORY_CHECK_H_
#define	_MEMORY_CHECK_H_

#include <stdio.h>
#include <stdlib.h>

/* Basic malloc check with file and line info */
#define CHECK_MALLOC(PTR, FUNC) \
if (PTR == NULL) { \
  fprintf (stderr, "%s:%d: %s: malloc error for %s\n", \
           __FILE__, __LINE__, FUNC, #PTR); \
  exit (1); \
}

/* Enhanced malloc check that returns error code instead of exiting */
#define CHECK_MALLOC_RET(PTR, FUNC, RET) \
if (PTR == NULL) { \
  fprintf (stderr, "%s:%d: %s: malloc error for %s\n", \
           __FILE__, __LINE__, FUNC, #PTR); \
  return RET; \
}

/* Check with custom error handler */
#define CHECK_MALLOC_HANDLER(PTR, FUNC, HANDLER) \
if (PTR == NULL) { \
  fprintf (stderr, "%s:%d: %s: malloc error for %s\n", \
           __FILE__, __LINE__, FUNC, #PTR); \
  HANDLER; \
}

/* Debug mode memory tracking (only enabled if DEBUG_MEMORY is defined) */
#ifdef DEBUG_MEMORY

static size_t total_allocated = 0;
static size_t total_freed = 0;
static int allocation_count = 0;

#define DEBUG_MALLOC(SIZE) \
  (total_allocated += (SIZE), \
   allocation_count++, \
   fprintf(stderr, "[MEM] Allocated %zu bytes (total: %zu, count: %d)\n", \
           (SIZE), total_allocated, allocation_count), \
   malloc(SIZE))

#define DEBUG_FREE(PTR) \
  do { \
    if (PTR) { \
      free(PTR); \
      allocation_count--; \
      fprintf(stderr, "[MEM] Freed memory (count: %d)\n", allocation_count); \
    } \
  } while(0)

#define PRINT_MEMORY_STATS() \
  fprintf(stderr, "[MEM] Total allocated: %zu bytes, Active allocations: %d\n", \
          total_allocated, allocation_count)

#else /* !DEBUG_MEMORY */

#define DEBUG_MALLOC(SIZE) malloc(SIZE)
#define DEBUG_FREE(PTR) free(PTR)
#define PRINT_MEMORY_STATS() ((void)0)

#endif /* DEBUG_MEMORY */

/* Safe allocation wrappers */
static inline void* safe_malloc(size_t size, const char *func, const char *file, int line)
{
  void *ptr = malloc(size);
  if (ptr == NULL && size > 0) {
    fprintf(stderr, "%s:%d: %s: malloc(%zu) failed\n", file, line, func, size);
    exit(1);
  }
  return ptr;
}

static inline void* safe_realloc(void *old_ptr, size_t size, const char *func, const char *file, int line)
{
  void *ptr = realloc(old_ptr, size);
  if (ptr == NULL && size > 0) {
    fprintf(stderr, "%s:%d: %s: realloc(%zu) failed\n", file, line, func, size);
    exit(1);
  }
  return ptr;
}

#define SAFE_MALLOC(SIZE) safe_malloc(SIZE, __FUNCTION__, __FILE__, __LINE__)
#define SAFE_REALLOC(PTR, SIZE) safe_realloc(PTR, SIZE, __FUNCTION__, __FILE__, __LINE__)

#endif /* !_MEMORY_CHECK_H_ */
