#include <stdint.h>

int is_bit_set(uint64_t bb, int position);
void set_bit(uint64_t *piece_bb, int position);
void unset_bit(uint64_t *piece_bb, int position);
int get_lowest_bit_index(uint64_t bb);