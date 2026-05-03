#include <stdint.h>

int is_bit_set(uint64_t bb, int position)
{
    uint64_t mask = (uint64_t) 1 << position;
    return (bb & mask) ? 1 : 0;
}