#include <stdint.h>

int is_bit_set(uint64_t bb, int position)
{
    uint64_t mask = (uint64_t) 1 << position;
    return (bb & mask) ? 1 : 0;
}

void set_bit(uint64_t *piece_bb, int position)
{
    uint64_t mask = (uint64_t) 1 << position;
    *piece_bb |= mask;
}

void unset_bit(uint64_t *piece_bb, int position)
{
    uint64_t mask = (uint64_t) 1 << position;
    *piece_bb &= ~mask;
}

int get_lowest_bit_index(uint64_t bb)
{
    // Ensure that atleast 1 bit is set
    if (bb == 0) {
        return -1;
    }
    int index = 0;

    // Check bitboard against one (1ULL = ... 0001)
    while ((bb & 1ULL) == 0) {
        bb >>= 1;
        index++;
    }
    return index;
}