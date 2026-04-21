# Manual Memory Management in C — A Guide for This Project

This guide explains stack and heap memory in C and maps every concept directly to code you have already written or are about to implement based on the code review.

---

## 1. The Stack

### What is it?

The stack is a region of memory managed automatically by the CPU. Every time you call a function, a **stack frame** is pushed: it holds that function's local variables, its parameters, and the return address. When the function returns, the frame is popped and that memory is instantly reclaimed — no manual work required.

Key properties:
- Fixed, limited size (typically 1–8 MB on modern systems).
- Allocation and deallocation are a single CPU instruction (moving the stack pointer).
- Variables live only as long as the function they are declared in.

### Where you already use the stack

Every local variable in your project lives on the stack. Some concrete examples:

**`square_from_position()` in [board.c](board.c)**
```c
Square square_from_position(int position)
{
    int file = position % 8;         // stack
    int row  = (position - file) / 8; // stack
    return (Square){file, row};      // copied out of the frame on return
}
```
Both `file` and `row`, and the returned `Square`, live on the stack. Returning a `Square` by value copies its two `int` fields — totally fine and efficient for small structs.

**`find_diagonal_moves()` in [pieces.c](pieces.c)**
```c
int directions[4] = {7, 9, -7, -9};  // stack array, 4 ints = 16 bytes
int counter = 1;                       // stack
int next_pos = position + directions[i]; // stack
```
The `directions` array is created on the stack every time this function is called and disappears when it returns.

**`is_check()` in [board.c](board.c)**
```c
int short_castle_allowed = 1;   // stack
int long_castle_allowed  = 1;   // stack
int is_check = 0;               // stack
int castle_directions[4] = {-2, -1, 1, 2}; // stack
```

### Where new features will use the stack

#### Compact move representation (code review §1.5)

When you implement packed `uint16_t` moves, every `Move` you create in a function is a stack variable:

```c
typedef uint16_t Move;

Move create_move(int from, int to, int flags) {
    Move m = (from & 0x3F) | ((to & 0x3F) << 6) | ((flags & 0xF) << 12);
    return m; // 2-byte value copied off the stack on return
}
```

Because `uint16_t` is 2 bytes, you can freely create, copy, and pass hundreds of `Move` values on the stack without concern for heap overhead.

#### `can_castle()` validation (code review §1.3)

The review recommends moving castle validation into its own function. All intermediate results are purely local — perfect stack usage:

```c
int can_castle(char color, char side, GameState *state) {
    int rights_ok    = /* check TeamState flags */;   // stack int
    int path_clear   = /* check empty squares */;     // stack int
    int not_in_check = /* check current check */;     // stack int
    int path_safe    = /* check transit squares */;   // stack int

    return rights_ok && path_clear && not_in_check && path_safe;
}
```

None of these need to outlive the function call, so none need the heap.

#### `is_square_attacked()` (code review §1.2)

The review's efficient check detection approach generates attack masks inside the function and immediately tests them:

```c
int is_square_attacked(int square, char by_color, uint64_t occupancy) {
    uint64_t knight_attacks = knight_attack_table[square]; // stack uint64_t
    uint64_t pawn_attacks   = pawn_attack_table[by_color][square]; // stack
    // ...
    return (knight_attacks & enemy_knights) || (pawn_attacks & enemy_pawns);
}
```

All intermediate bitboards are stack variables — they are computed, tested, and thrown away within one function call.

---

## 2. The Heap

### What is it?

The heap is a large pool of memory that your program can request at runtime using `malloc()` / `calloc()` and release with `free()`. Unlike the stack:

- Size is limited only by available system RAM.
- Lifetime is controlled entirely by you — data persists until you call `free()`.
- Allocation is slower (the allocator must find a free block and track it).
- Forgotten `free()` calls cause **memory leaks**; using memory after `free()` causes **undefined behaviour**.

The three calls to know:

```c
#include <stdlib.h>

void *malloc(size_t size);           // allocate uninitialised bytes
void *calloc(size_t n, size_t size); // allocate n * size zero-initialised bytes
void  free(void *ptr);               // release allocation
```

Always check that `malloc`/`calloc` did not return `NULL` before using the pointer.

### Where the current code misses the heap (and causes problems)

**Global mutable state — [pieces.c](pieces.c)**

Right now, every bitboard and the `pieces` array are global variables:

```c
uint64_t WHITE_PAWNS = 0x000000000000FF00ULL;
// ...
Piece pieces[12] = { ... };
```

The code review flags three concrete problems with this:
1. You cannot run two games simultaneously (e.g. for AI self-play).
2. Resetting / unit-testing state requires manually writing every global back.
3. Nothing is thread-safe.

The fix is to move this state onto the heap inside a `Board` struct.

### Where new features will use the heap

#### `Board` struct (code review §2.1)

This is the most impactful change. A heap-allocated `Board` encapsulates all game state and makes everything above go away:

```c
// types.h — shared struct definitions
typedef struct {
    uint64_t white_pawns;
    uint64_t black_pawns;
    uint64_t white_rooks;
    uint64_t black_rooks;
    uint64_t white_knights;
    uint64_t black_knights;
    uint64_t white_bishops;
    uint64_t black_bishops;
    uint64_t white_queen;
    uint64_t black_queen;
    uint64_t white_king;
    uint64_t black_king;
    Piece pieces[12];        // now inside the struct, not global
} Board;

// board.c — constructor / destructor pattern
Board *board_create(void) {
    Board *b = calloc(1, sizeof(Board)); // zeroes all fields
    if (b == NULL) {
        return NULL; // caller must handle failure
    }
    board_init(b); // set starting position bitboards, link pieces to fields
    return b;
}

void board_destroy(Board *b) {
    free(b); // one call, no leaks
}
```

Usage in `main.c`:

```c
Board *board = board_create();
if (board == NULL) {
    fprintf(stderr, "Failed to allocate board\n");
    return 1;
}

// ... run the game ...

board_destroy(board);
```

#### `AttackMaps` struct (code review §1.1)

The review recommends caching attack maps on the board so they do not need to be recalculated from scratch every move. These maps live inside (or alongside) the heap-allocated board:

```c
typedef struct {
    uint64_t white_attacks;
    uint64_t black_attacks;
} AttackMaps;

typedef struct {
    // ... all bitboards ...
    Piece      pieces[12];
    AttackMaps attacks;      // embedded directly — no extra malloc needed
    GameState  state;
} Board;
```

Because `AttackMaps` is embedded inside `Board`, it is allocated and freed as part of the single `board_create()` / `board_destroy()` calls above. You only need a separate `malloc` when ownership is genuinely separate.

#### Move history stack for `make_move` / `unmake_move` (code review §1.4)

The review recommends a stack-based move history so the search can make and unmake thousands of moves per second. This is heap-allocated once at startup because its total size depends on the maximum search depth:

```c
#define MAX_MOVES 512 // enough for any realistic game tree depth

typedef struct {
    uint16_t    move;              // packed from/to/flags
    char        captured_piece;
    int         old_en_passant_sq;
    int         old_castle_rights; // 4-bit field: KQkq
} MoveUndo;

typedef struct {
    MoveUndo entries[MAX_MOVES]; // fixed-size array inside the Board
    int      top;                // index of next free slot (starts at 0)
} MoveHistory;
```

You can embed `MoveHistory` directly inside `Board` — no separate allocation. Then:

```c
void push_undo(Board *b, MoveUndo u) {
    assert(b->history.top < MAX_MOVES);
    b->history.entries[b->history.top++] = u;
}

MoveUndo pop_undo(Board *b) {
    assert(b->history.top > 0);
    return b->history.entries[--b->history.top];
}
```

`make_move` saves state before modifying the board; `unmake_move` pops and restores it. This replaces the confusing `update_state` / `real_move` boolean flags identified in the review (§2.2).

---

## 3. Stack vs. Heap — Decision Checklist

| Question | Stack | Heap |
|---|---|---|
| Is the size known at compile time? | yes → stack | no → heap |
| Does the data need to outlive the function? | no → stack | yes → heap |
| Will there be more than one instance (e.g. two boards)? | no → stack | yes → heap |
| Is the data very large (megabytes)? | no → stack | yes → heap |
| Do you need to pass ownership across module boundaries? | no → stack | yes → heap |

For this chess project, the practical rule is simple:
- **Temporaries inside a function** (moves, squares, bitboard masks, loop variables) → stack.
- **Game state that the whole program shares and that needs explicit lifetime control** (`Board`, `AttackMaps`, `MoveHistory`) → heap, allocated once in `main` or `board_create`, freed once on exit.

---

## 4. Common Mistakes to Avoid

### Dangling pointer — returning a pointer to a local variable

```c
// WRONG — stack frame is gone after return
Square *bad_square(void) {
    Square sq = {3, 4};
    return &sq; // undefined behaviour: sq no longer exists
}

// CORRECT — return by value for small structs
Square good_square(void) {
    return (Square){3, 4};
}

// CORRECT — heap-allocate if caller needs to keep it alive
Square *heap_square(void) {
    Square *sq = malloc(sizeof(Square));
    if (sq) { sq->file = 3; sq->row = 4; }
    return sq; // caller must free()
}
```

Your current `square_from_position()` already does the right thing — it returns by value.

### Memory leak — forgetting `free()`

```c
Board *b = board_create();
// ... forgot board_destroy(b) before returning
// leak: the Board lives in memory forever
```

Use a single `board_destroy()` at every exit path, or `goto cleanup` patterns for complex initialisation.

### Use-after-free

```c
board_destroy(b);
printf("%d\n", b->state.total_moves); // undefined behaviour
b = NULL; // set to NULL immediately after free to catch this in debugger
```

### NULL dereference — already present in your code (code review §2.3)

```c
// pieces.c — is_enemy()
Piece *other_piece = find_piece_by_position(position);
if (other_piece == NULL) {
    return 0; // guard BEFORE dereferencing
}
if (piece->color != other_piece->color) { ... }
```

Every pointer returned by `malloc`, `calloc`, or a function that can fail must be checked for `NULL` before use.
