#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define WRITE_MASK(x) ((1 << (8 - ((x) % 9))) - 1)
#define READ_MASK(x) (~(WRITE_MASK(x)))

static uint8_t output[1000], input[1000];

static long timespec_diff(struct timespec *a, struct timespec *b) {
  return ((b->tv_sec - a->tv_sec) * 1000000000) + (b->tv_nsec - a->tv_nsec);
}

void bitcpy(void *_dest,      /* Address of the buffer to write to */
            size_t _write,    /* Bit offset to start writing to */
            const void *_src, /* Address of the buffer to read from */
            size_t _read,     /* Bit offset to start reading from */
            size_t _count) {
  uint8_t data, original, mask;
  size_t bitsize;
  size_t read_lhs = _read & 7;
  size_t read_rhs = 8 - read_lhs;
  const uint8_t *source = _src + (_read / 8);
  size_t write_lhs = _write & 7; /* KK1 */
  size_t write_rhs = 8 - write_lhs;
  uint8_t *dest = _dest + (_write / 8);

#if 0
  static const uint8_t read_mask[] = {
      0x00, /*	== 0	00000000b	*/
      0x80, /*	== 1	10000000b	*/
      0xC0, /*	== 2	11000000b	*/
      0xE0, /*	== 3	11100000b	*/
      0xF0, /*	== 4	11110000b	*/
      0xF8, /*	== 5	11111000b	*/
      0xFC, /*	== 6	11111100b	*/
      0xFE, /*	== 7	11111110b	*/
      0xFF  /*	== 8	11111111b	*/
  };

  static const uint8_t write_mask[] = {
      0xFF, /*	== 0	11111111b	*/
      0x7F, /*	== 1	01111111b	*/
      0x3F, /*	== 2	00111111b	*/
      0x1F, /*	== 3	00011111b	*/
      0x0F, /*	== 4	00001111b	*/
      0x07, /*	== 5	00000111b	*/
      0x03, /*	== 6	00000011b	*/
      0x01, /*	== 7	00000001b	*/
      0x00  /*	== 8	00000000b	*/
  };
#endif

  while (_count > 0) {
    data = *source++;
    bitsize = (_count > 8) ? 8 : _count;
    if (read_lhs > 0) {
      data <<= read_lhs;
      if (bitsize > read_rhs)
        data |= (*source >> read_rhs);
    }

    if (bitsize < 8)
      data &= READ_MASK(bitsize);

    original = *dest;
    if (write_lhs > 0) {
      mask = READ_MASK(write_lhs);
      if (bitsize > write_rhs) {
        *dest++ = (original & mask) | (data >> write_lhs);
        original = *dest & WRITE_MASK(bitsize - write_rhs);
        *dest = original | (data << write_rhs);
      } else {
        if ((bitsize - write_lhs) > 0)
          mask = mask | WRITE_MASK(8 - (bitsize - write_lhs));
        *dest++ = (original & mask) | (data >> write_lhs);
      }
    } else {
      if (bitsize < 8)
        data = data | (original & WRITE_MASK(bitsize));
      *dest++ = data;
    }

    _count -= bitsize; /* KK2 */
  }
}

void bitcpy_branch_less(
    void *_dest,      /* Address of the buffer to write to */
    size_t _write,    /* Bit offset to start writing to */
    const void *_src, /* Address of the buffer to read from */
    size_t _read,     /* Bit offset to start reading from */
    size_t _count) {
  uint8_t data, original, mask;
  size_t bitsize;
  size_t read_lhs = _read & 7;
  size_t read_rhs = 8 - read_lhs;
  const uint8_t *source = _src + (_read / 8);
  size_t write_lhs = _write & 7; /* KK1 */
  size_t write_rhs = 8 - write_lhs;
  uint8_t *dest = _dest + (_write / 8);

  while (_count > 0) {
    bitsize = (_count > 8) ? 8 : _count;

    /* Don't care if read_lhs > 0 and
     * bitsize > read_rhs are matched or not.
     * Just read 8 bits data. */
    data = (*source++ << read_lhs) | (*source >> read_rhs);

    /* Don't care if bitsize < 8 or not.
     * data doesn't changed when it AND 11111111b */
    data &= READ_MASK(bitsize);

    original = *dest;
    /* Handle left part of data:
     * Mask original value out (*dest), and OR data with it. */
    mask = READ_MASK(write_lhs);
    *dest++ = (original & mask) | (data >> write_lhs);
    /* Handle right part of data: */
    if ((bitsize - write_rhs) >= 0) {
      *dest &= WRITE_MASK(bitsize - write_rhs);
      *dest |= (data << write_rhs);
    }

    _count -= bitsize; /* KK2 */
  }
}

static inline void dump_8bits(uint8_t _data)
{
    for (int i = 0; i < 8; ++i)
        printf("%d", (_data & (0x80 >> i)) ? 1 : 0);
}
static inline void dump_binary(uint8_t *_buffer, size_t _length)
{
    for (int i = 0; i < _length; ++i)
        dump_8bits(*_buffer++);
}

int main(int _argc, char **_argv)
{
  FILE *fptr = NULL;
  struct timespec t1, t2;

  fptr = fopen("bitcpy_time_comp.txt", "w+");

  memset(&input[0], 0xFF, sizeof(input));

  for (int i = 1; i <= 7900; ++i) {
    memset(&output[0], 0x00, sizeof(output));
    clock_gettime(CLOCK_REALTIME, &t1);
    bitcpy(&output[0], 1, &input[0], 2, i);
    clock_gettime(CLOCK_REALTIME, &t2);
    fprintf(fptr, "%d %ld", i, timespec_diff(&t1, &t2));

    memset(&output[0], 0x00, sizeof(output));
    clock_gettime(CLOCK_REALTIME, &t1);
    bitcpy_branch_less(&output[0], 1, &input[0], 2, i);
    clock_gettime(CLOCK_REALTIME, &t2);
    fprintf(fptr, " %ld\n", timespec_diff(&t1, &t2));
  }

  fclose(fptr);

  return 0;
}
