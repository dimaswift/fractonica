#include "Ephemeris.h"

#if defined(ARDUINO_ARCH_AVR)
#include <avr/pgmspace.h>
// Safest on AVR for int64_t: copy 8 bytes from flash
static inline int64_t read_int64_progmem(const int64_t *p)
{
  int64_t v;
  memcpy_P(&v, p, sizeof(v));
  return v;
}
#define READ_INT64(ptr) read_int64_progmem((const int64_t *)(ptr))

#elif defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
#include <pgmspace.h>
#define READ_INT64(ptr) ((int64_t)pgm_read_dword((const void *)(ptr)))

#else
#define READ_INT64(ptr) (*(const int64_t *)(ptr))
#endif

void *fractonica_file_open(const char *filepath, fractonica_header_t *header)
{
  if (!filepath || !header)
  {
    return NULL;
  }

  FILE *f = fopen(filepath, "rb");
  if (!f)
  {
    return NULL;
  }

  uint8_t buf[FRACTONICA_HEADER_SIZE];
  if (fread(buf, 1, FRACTONICA_HEADER_SIZE, f) != FRACTONICA_HEADER_SIZE)
  {
    fclose(f);
    return NULL;
  }

  header->magic = (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) |
                  ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);

  header->entry_count = (uint32_t)buf[4] | ((uint32_t)buf[5] << 8) |
                        ((uint32_t)buf[6] << 16) | ((uint32_t)buf[7] << 24);

  header->reserved = (uint64_t)buf[8] | ((uint64_t)buf[9] << 8) |
                      ((uint64_t)buf[10] << 16) | ((uint64_t)buf[11] << 24) |
                      ((uint64_t)buf[12] << 32) | ((uint64_t)buf[13] << 40) |
                      ((uint64_t)buf[14] << 48) | ((uint64_t)buf[15] << 56);

  return f;
}

void fractonica_file_close(void *handle)
{
  if (handle)
  {
    fclose((FILE *)handle);
  }
}

int64_t fractonica_file_get_timestamp(void *handle,
                                      const fractonica_header_t *header,
                                      uint32_t index)
{
  if (!handle || !header || index >= header->entry_count)
  {
    return (int64_t)FRACTONICA_INVALID;
  }

  FILE *f = (FILE *)handle;
  long offset = FRACTONICA_HEADER_SIZE + (index * 8);

  if (fseek(f, offset, SEEK_SET) != 0)
  {
    return (int64_t)FRACTONICA_INVALID;
  }

  uint8_t buf[8];
  if (fread(buf, 1, 8, f) != 8)
  {
    return (int64_t)FRACTONICA_INVALID;
  }

  /* Little-endian parse */
  uint64_t val = (uint64_t)buf[0] | ((uint64_t)buf[1] << 8) |
                  ((uint64_t)buf[2] << 16) | ((uint64_t)buf[3] << 24) |
                  ((uint64_t)buf[4] << 32) | ((uint64_t)buf[5] << 40) |
                  ((uint64_t)buf[6] << 48) | ((uint64_t)buf[7] << 56);

  return (int64_t)val;
}

void fractonica_mem_init(fractonica_mem_source_t *source, uint32_t count,
                          const int64_t *timestamps)
{
  if (source)
  {
    source->entry_count = count;
    source->timestamps = timestamps;
  }
}

int64_t fractonica_mem_get_timestamp(const fractonica_mem_source_t *source,
                                      uint32_t index)
{
  if (!source || !source->timestamps || index >= source->entry_count)
  {
    return (int64_t)FRACTONICA_INVALID;
  }
  return READ_INT64(source->timestamps + index);
}

fractonica_search_result_t
fractonica_find_closest(const fractonica_mem_source_t *source,
                        int64_t timestamp)
{
  fractonica_search_result_t result = {0, 0, false, false};

  if (!source || !source->timestamps || source->entry_count == 0)
  {
    return result;
  }

  uint32_t left = 0;
  uint32_t right = source->entry_count - 1;

  int64_t first = READ_INT64(source->timestamps);
  int64_t last = READ_INT64(source->timestamps + right);

  if (timestamp < first)
  {
    result.future_index = 0;
    result.found_future = true;
    result.found_past = false;
    return result;
  }

  if (timestamp > last)
  {
    result.past_index = right;
    result.found_past = true;
    result.found_future = false;
    return result;
  }

  while (left <= right)
  {
    uint32_t mid = left + (right - left) / 2;
    int64_t val = READ_INT64(source->timestamps + mid);

    if (val < timestamp)
    {
      left = mid + 1;
    }
    else if (val > timestamp)
    {
      if (mid == 0)
      {
        right = 0;
        break;
      }
      right = mid - 1;
    }
    else
    {
      result.past_index = mid;
      result.future_index = mid;
      result.found_past = true;
      result.found_future = true;
      return result;
    }
  }

  if (left < source->entry_count)
  {
    result.future_index = left;
    result.found_future = true;
  }

  if (left > 0)
  {
    result.past_index = left - 1;
    result.found_past = true;
  }

  return result;
}

uint32_t convert_decimal_to_octal(uint32_t decimalNumber)
{
  uint32_t octalNumber = 0;
  uint32_t i = 1;
  uint32_t tempDecimal = decimalNumber;
  while (tempDecimal != 0)
  {
    uint32_t remainder = tempDecimal % 8;
    octalNumber += remainder * i;
    tempDecimal /= 8;
    i *= 10;
  }
  return (uint32_t)octalNumber;
}

fractonica_ephemeris_fraction_t
fractonica_ephemeris_fraction_at(const fractonica_mem_source_t *source,
                                  int64_t unix_ts,
                                  uint32_t resolution,
                                  int64_t *io_window_start,
                                  int64_t *io_window_end)
{
  fractonica_ephemeris_fraction_t out;
  memset(&out, 0, sizeof(out));
  out.valid = false;

  if (!source || !source->timestamps || source->entry_count < 2 || resolution == 0)
  {
    return out;
  }

  // Use a cached window if provided and valid and unix_ts inside [start, end]
  bool have_window = (io_window_start && io_window_end);
  int64_t t0 = have_window ? *io_window_start : 0;
  int64_t t1 = have_window ? *io_window_end : 0;

  bool window_ok = have_window && (t1 > t0) && (unix_ts >= t0) && (unix_ts <= t1);

  if (!window_ok)
  {
    // Find a new surrounding window and store it back (if caller provided pointers)
    fractonica_search_result_t sr = fractonica_find_closest(source, unix_ts);
    out.past_index = sr.past_index;
    out.future_index = sr.future_index;

    if (!sr.found_past || !sr.found_future)
      return out;

    t0 = fractonica_mem_get_timestamp(source, sr.past_index);
    t1 = fractonica_mem_get_timestamp(source, sr.future_index);
    if (t0 == (int64_t)FRACTONICA_INVALID || t1 == (int64_t)FRACTONICA_INVALID)
      return out;
    if (t1 <= t0)
      return out;

    if (have_window)
    {
      *io_window_start = t0;
      *io_window_end = t1;
    }
  }

  // Now compute within a window [t0, t1]
  if (unix_ts < t0)
    unix_ts = t0;
  if (unix_ts > t1)
    unix_ts = t1;

  const double period = (double)(t1 - t0);
  const double dt = (double)(unix_ts - t0);
  double normalized = dt / period;
  if (normalized < 0.0)
    normalized = 0.0;
  if (normalized > 1.0)
    normalized = 1.0;
  out.normalized = normalized;

  double pos = normalized * (double)resolution;

  // Ceil bin mapping (1..resolution) then to 0-based (0..resolution-1)
  uint32_t ceiled = (uint32_t)ceil(pos);
  if (ceiled == 0)
    ceiled = 1;
  if (ceiled > resolution)
    ceiled = resolution;
  out.bin = (ceiled - 1);

  // progress toward next bin boundary (fractional part within the current bin step)
  double nextBoundary = (double)ceiled;
  double prevBoundary = nextBoundary - 1.0;
  double frac;
  if (pos <= prevBoundary)
    frac = 0.0;
  else if (pos >= nextBoundary)
    frac = 1.0;
  else
    frac = (pos - prevBoundary); // 0..1
  out.progress = frac;

  out.bin_octal = convert_decimal_to_octal(out.bin);
  out.valid = true;
  return out;
}
