/**
 * Fractonica Celestial Ephemeris Library
 *
 * Efficient reader for compact binary ephemeris files.
 * Supports both file-based reading (seek) and embedded static arrays.
 *
 * Binary format (little-endian):
 *   uint32  magic           - "FRAC" (0x43415246)
 *   uint32  entry_count     - Number of timestamps
 *   uint64  reserved        - Reserved
 *   int64[] timestamps      - Absolute timestamps
 */

#ifndef FRACTONICA_EPHEMERIS_H
#define FRACTONICA_EPHEMERIS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define FRACTONICA_MAGIC 0x43415246
#define FRACTONICA_HEADER_SIZE 16
/* Error value returned for invalid queries */
#define FRACTONICA_INVALID UINT64_MAX

/* Binary file header structure */
typedef struct
{
  uint32_t magic;
  uint32_t entry_count;
  uint64_t reserved;
} fractonica_header_t;



typedef struct
{
  uint32_t entry_count;
  const int64_t *timestamps; /* Pointer to static int64 array */
} fractonica_mem_source_t;

typedef struct
{
  int64_t start;
  int64_t period;
  uint16_t resolution; /* Power of 2, e.g., 4096 */
} fractonica_period_config_t;

typedef struct
{
  uint32_t bin;       /* 0..resolution-1 */
  uint32_t bin_octal; /* decimal bin rendered as octal digits */
  double normalized;  /* 0..1 within current period window */
  double progress;    /* 0..1 progress to next bin boundary */
  bool valid;
  uint32_t past_index;
  uint32_t future_index;
} fractonica_ephemeris_fraction_t;

typedef struct
{
  uint32_t past_index;   /* Index <= timestamp */
  uint32_t future_index; /* Index >= timestamp */
  bool found_past;
  bool found_future;
} fractonica_search_result_t;

/**
 * Open an ephemeris binary file and read its header.
 */
void *fractonica_file_open(const char *filepath, fractonica_header_t *header);

/**
 * Close an ephemeris file.
 */
void fractonica_file_close(void *handle);

/**
 * Get the timestamp of period N from a file.
 */
int64_t fractonica_file_get_timestamp(void *handle,
                                      const fractonica_header_t *header,
                                      uint32_t index);
/**
 * Initialize a memory source from static data.
 */
void fractonica_mem_init(fractonica_mem_source_t *source, uint32_t count,
                          const int64_t *timestamps);

/**
 * Get the timestamp of period N from memory.
 */
int64_t fractonica_mem_get_timestamp(const fractonica_mem_source_t *source,
                                      uint32_t index);

/**
 * Find closest period indices (past and future) for a given timestamp.
 * If timestamp is exact match, past_index == future_index.
 */
fractonica_search_result_t
fractonica_find_closest(const fractonica_mem_source_t *source,
                        int64_t timestamp);

/**
 * Compute bin/progress from ephemeris periods around a unix timestamp.
 *
 * - Finds surrounding timestamps (past/future)
 * - Computes normalized position in that window
 * - Maps to discrete bins of given resolution
 * - progress = fractional part toward next bin
 *
 * resolution must be > 0.
 */
fractonica_ephemeris_fraction_t
fractonica_ephemeris_fraction_at(const fractonica_mem_source_t *source,
                                 int64_t unix_ts,
                                 uint32_t resolution,
                                 int64_t *io_window_start, /* optional in/out */
                                 int64_t *io_window_end);  /* optional in/out */


#ifdef __cplusplus
}
#endif

#endif /* FRACTONICA_EPHEMERIS_H */