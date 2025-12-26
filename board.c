#include "board.h"

#include "pieces.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int count_bits(uint64_t number)
{
    int count = 0;
    while (number) {
        count += number & 1;
        number >>= 1;
    }
    return count;
}

int calculate_total_piece_value(Piece pieces[], char color)
{
    int total_value = 0;
    for (int i = 0; i < 12; i++) {
        if (pieces[i].color == color) {
            Piece piece = pieces[i];
            int amount_bits_set = count_bits(*piece.pos_bb);
            total_value = total_value + (amount_bits_set * piece.value);
        }
    }
    return total_value;
}

char color_to_move(GameState *game_state)
{
    return game_state->total_moves % 2 == 0 ? 'w' : 'b';
}

void squares_to_notation(Square input_square, Square output_square,
                         char last_move[5])
{
    last_move[0] = FILE_OFFSET + input_square.file;
    last_move[1] = ROW_OFFSET + input_square.row;
    last_move[2] = FILE_OFFSET + output_square.file;
    last_move[3] = ROW_OFFSET + output_square.row;
    last_move[4] = '\0';
}

void print_bitboard(uint64_t possible_moves)
{
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d ", rank + 1);
        for (int file = 0; file < 8; file++) {
            int sq = rank * 8 + file;
            uint64_t mask = (uint64_t) 1 << sq;

            if (possible_moves & mask)
                printf("x");
            else
                printf(".");
        }
        printf("\n");
    }
    printf("  abcdefgh\n");
}

int get_position(int file, int row)
{
    return row * 8 + file;
}

Square square_from_position(int position)
{
    int file = position % 8;
    int row = (position - file) / 8;
    return (Square) {file, row};
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

int is_check(Piece pieces[], GameState *game_state, char color_moving)
{
    int king_index = color_moving == 'b' ? WHITE_KING_INDEX : BLACK_KING_INDEX;
    int king_position = get_lowest_bit_index(*(pieces[king_index].pos_bb));

    for (int i = 0; i < 12; i++) {
        if (pieces[i].color == color_moving) {
            uint64_t piece_bb = *(pieces[i].pos_bb);
            while (piece_bb) {
                int position = get_lowest_bit_index(piece_bb);
                Square input_square = square_from_position(position);
                Piece *piece = find_piece_by_position(position);

                if (piece != NULL) {
                    uint64_t pos_mov = find_possible_moves(
                        input_square, position, piece, game_state);
                    if (is_bit_set(pos_mov, king_position)) {
                        return 1;
                    }
                }
                piece_bb &= piece_bb - 1;
            }
        }
    }
    return 0;
}

void make_move(Square input_square, Square output_square, Piece pieces[],
               GameState *game_state, int update_state)
{
    int old_pos = get_position(input_square.file, input_square.row);
    Piece *piece = find_piece_by_position(old_pos);

    int new_pos = get_position(output_square.file, output_square.row);

    Piece *other_piece = find_piece_by_position(new_pos);
    char color_moving = color_to_move(game_state);

    if (game_state->play_en_passant) {
        int other_pawn_pos = new_pos;
        if (color_moving == 'w') {
            other_pawn_pos = other_pawn_pos - 8;
        } else {
            other_pawn_pos = other_pawn_pos + 8;
        }
        other_piece = find_piece_by_position(other_pawn_pos);

        if (!(other_piece == NULL)) {
            unset_bit(other_piece->pos_bb, other_pawn_pos);
            if (!(update_state)) {
                game_state->last_captured_piece = other_piece->symbol;
            }
        }
        unset_bit(piece->pos_bb, old_pos);
        set_bit(piece->pos_bb, new_pos);
    } else {
        if (!(other_piece == NULL)) {
            unset_bit(other_piece->pos_bb, new_pos);
            if (!(update_state)) {
                game_state->last_captured_piece = other_piece->symbol;
            }
        }

        unset_bit(piece->pos_bb, old_pos);
        if (game_state->promote_pawn) {
            piece = get_piece_bb(game_state->promote_to);
        }
        set_bit(piece->pos_bb, new_pos);
    }

    if (update_state) {
        game_state->promote_pawn = 0;
        game_state->promote_to = '0';
        game_state->play_en_passant = 0;
        game_state->last_moved_piece = piece->symbol;
        game_state->is_check = is_check(pieces, game_state, color_moving);
        game_state->total_moves++;
    }
}

const char *get_image_path(char symbol)
{
    switch (symbol) {
    case 'P':
        return "images/pawn_w.png";
    case 'p':
        return "images/pawn_b.png";
    case 'R':
        return "images/rook_w.png";
    case 'r':
        return "images/rook_b.png";
    case 'N':
        return "images/knight_w.png";
    case 'n':
        return "images/knight_b.png";
    case 'B':
        return "images/bishop_w.png";
    case 'b':
        return "images/bishop_b.png";
    case 'Q':
        return "images/queen_w.png";
    case 'q':
        return "images/queen_b.png";
    case 'K':
        return "images/king_w.png";
    case 'k':
        return "images/king_b.png";
    default:
        return NULL;
    }
}

void draw_possible_moves(SDL_Renderer *renderer, char board[8][8],
                         uint64_t pos_mov)
{
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int sq = rank * 8 + file;
            uint64_t mask = (uint64_t) 1 << sq;
            if (pos_mov & mask) {
                // Enable alpha blending
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

                // Set semi-transparent dark army green color (R, G, B, Alpha)
                SDL_SetRenderDrawColor(renderer, 60, 80, 50, 180);

                // Check if square has a piece (capture move)
                if (board[rank][file] != 0) {
                    // Draw triangles in corners
                    int corner_size = SQUARE_SIZE / 5;
                    int x = file * SQUARE_SIZE;
                    int y = (7 - rank) * SQUARE_SIZE;

                    // Top-left triangle (corner at top-left, legs going right
                    // and down)
                    for (int i = 0; i < corner_size; i++) {
                        for (int j = 0; j < corner_size - i; j++) {
                            SDL_RenderDrawPoint(renderer, x + j, y + i);
                        }
                    }

                    // Top-right triangle (corner at top-right, legs going left
                    // and down)
                    for (int i = 0; i < corner_size; i++) {
                        for (int j = 0; j < corner_size - i; j++) {
                            SDL_RenderDrawPoint(renderer,
                                                x + SQUARE_SIZE - 1 - j, y + i);
                        }
                    }

                    // Bottom-left triangle (corner at bottom-left, legs going
                    // right and up)
                    for (int i = 0; i < corner_size; i++) {
                        for (int j = 0; j < corner_size - i; j++) {
                            SDL_RenderDrawPoint(renderer, x + j,
                                                y + SQUARE_SIZE - 1 - i);
                        }
                    }

                    // Bottom-right triangle (corner at bottom-right, legs going
                    // left and up)
                    for (int i = 0; i < corner_size; i++) {
                        for (int j = 0; j < corner_size - i; j++) {
                            SDL_RenderDrawPoint(renderer,
                                                x + SQUARE_SIZE - 1 - j,
                                                y + SQUARE_SIZE - 1 - i);
                        }
                    }
                } else {
                    // Draw circle in center for empty squares
                    int center_x = file * SQUARE_SIZE + SQUARE_SIZE / 2;
                    int center_y = (7 - rank) * SQUARE_SIZE + SQUARE_SIZE / 2;
                    int radius = SQUARE_SIZE / 6;

                    for (int w = 0; w < radius * 2; w++) {
                        for (int h = 0; h < radius * 2; h++) {
                            int dx = radius - w;
                            int dy = radius - h;
                            if ((dx * dx + dy * dy) <= (radius * radius)) {
                                SDL_RenderDrawPoint(renderer, center_x + dx,
                                                    center_y + dy);
                            }
                        }
                    }
                }
            }
        }
    }
}

void render_board(SDL_Renderer *renderer, char board[8][8], Piece pieces[],
                  Square sel_square, uint64_t pos_mov, int render_bool,
                  GameState *game_state)
{
    for (int row = 0; row < 8; row++) {
        for (int file = 0; file < 8; file++) {
            int y = 7 - row;
            int x = file;

            SDL_Rect rect = {x * SQUARE_SIZE, y * SQUARE_SIZE, SQUARE_SIZE,
                             SQUARE_SIZE};
            if (sel_square.file == file && sel_square.row == row) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

                SDL_SetRenderDrawColor(renderer, 60, 80, 50, 180);
                SDL_RenderFillRect(renderer, &rect);
            } else if ((x + row) % 2 == 0)
                SDL_SetRenderDrawColor(renderer, 240, 217, 181, 255);
            else
                SDL_SetRenderDrawColor(renderer, 181, 136, 99, 255);

            SDL_RenderFillRect(renderer, &rect);

            if (board[row][file] != 0) {
                SDL_Texture *tex = NULL;
                for (int i = 0; i < 12; i++) {
                    char symbol = pieces[i].symbol;
                    if (symbol == board[row][file]) {
                        SDL_Surface *surface = IMG_Load(get_image_path(symbol));
                        if (!surface) {
                            printf("Failed to load image: %s\n",
                                   IMG_GetError());
                            continue;
                        }
                        tex = SDL_CreateTextureFromSurface(renderer, surface);
                        SDL_FreeSurface(surface);
                        break;
                    }
                }
                if (tex) {
                    SDL_Rect pieceRect = {x * SQUARE_SIZE, y * SQUARE_SIZE,
                                          SQUARE_SIZE, SQUARE_SIZE};
                    SDL_RenderCopy(renderer, tex, NULL, &pieceRect);
                }
            }
        }
    }
    draw_possible_moves(renderer, board, pos_mov);

    if (game_state->is_check) {
        SDL_SetRenderDrawColor(renderer, 220, 50, 50, 180);

        char color_moving = color_to_move(game_state);
        int king_index =
            color_moving == 'w' ? WHITE_KING_INDEX : BLACK_KING_INDEX;
        int king_position = get_lowest_bit_index(*(pieces[king_index].pos_bb));
        Square king_square = square_from_position(king_position);

        int center_x = king_square.file * SQUARE_SIZE + SQUARE_SIZE / 2;
        int center_y = (7 - king_square.row) * SQUARE_SIZE + SQUARE_SIZE / 2;
        int radius = SQUARE_SIZE / 2;

        for (int w = 0; w < radius * 2; w++) {
            for (int h = 0; h < radius * 2; h++) {
                int dx = radius - w;
                int dy = radius - h;
                if ((dx * dx + dy * dy) <= (radius * radius)) {
                    SDL_RenderDrawPoint(renderer, center_x + dx, center_y + dy);
                }
            }
        }
    }

    if (render_bool) {
        SDL_RenderPresent(renderer);
    }
}

void bitboards_to_board(Piece pieces[], char board[8][8])
{
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int sq = rank * 8 + file;
            uint64_t mask = (uint64_t) 1 << sq;
            char piece = 0;

            for (int i = 0; i < 12; i++) {
                if (*(pieces[i].pos_bb) & mask) {
                    piece = pieces[i].symbol;
                }
            }
            board[rank][file] = piece;
        }
    }
}

void render_promotion_squares(SDL_Renderer *renderer, Square output_square,
                              Piece pieces[], int direction,
                              char promotion_pieces[], GameState *game_state)
{
    char board[8][8];
    Square fake_square = {-1, -1};
    bitboards_to_board(pieces, board);
    uint64_t pos_mov = (uint64_t) 0;
    int render_bool = 0;

    render_board(renderer, board, pieces, fake_square, pos_mov, render_bool,
                 game_state);

    SDL_SetRenderDrawColor(renderer, 211, 211, 211, 255);
    for (int i = 0; i < 4; i++) {
        int row = (7 - output_square.row + (i * direction));
        int file = output_square.file;
        int center_x = file * SQUARE_SIZE + SQUARE_SIZE / 2;
        int center_y = row * SQUARE_SIZE + SQUARE_SIZE / 2;
        int radius = SQUARE_SIZE / 2;
        for (int w = 0; w < radius * 2; w++) {
            for (int h = 0; h < radius * 2; h++) {
                int dx = radius - w;
                int dy = radius - h;
                if ((dx * dx + dy * dy) <= (radius * radius)) {
                    SDL_RenderDrawPoint(renderer, center_x + dx, center_y + dy);
                }
            }
        }
        SDL_Texture *tex = NULL;
        char symbol = promotion_pieces[i];
        SDL_Surface *surface = IMG_Load(get_image_path(symbol));
        if (!surface) {
            printf("Failed to load image: %s\n", IMG_GetError());
            continue;
        }
        tex = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (tex) {
            SDL_Rect pieceRect = {file * SQUARE_SIZE, row * SQUARE_SIZE,
                                  SQUARE_SIZE, SQUARE_SIZE};
            SDL_RenderCopy(renderer, tex, NULL, &pieceRect);
        }
    }
    SDL_RenderPresent(renderer);
}

void get_promotion_pieces(char color, char promotion_pieces[4])
{
    if (color == 'w') {
        promotion_pieces[0] = 'Q';
        promotion_pieces[1] = 'R';
        promotion_pieces[2] = 'N';
        promotion_pieces[3] = 'B';
    } else {
        promotion_pieces[0] = 'q';
        promotion_pieces[1] = 'r';
        promotion_pieces[2] = 'n';
        promotion_pieces[3] = 'b';
    }
}

char get_promotion_piece(char color, int row)
{

    char promotion_pieces[4];
    get_promotion_pieces(color, promotion_pieces);
    if (color == 'b') {
        return promotion_pieces[row];
    } else {
        int index = abs(row - 7);
        return promotion_pieces[index];
    }
}

void validate_possible_moves_solve_check(uint64_t *pos_mov, Square input_square,
                                         GameState *game_state)
{
    // This could be done by checking which piece attacks the king -> calculate
    // which squares need to be blocked in order to fix check. Or take piece
    // that sets king in check In case of double check always move the king
    Piece *pieces = get_pieces();

    int update_state = 0;
    uint64_t copy_pos_mov = *pos_mov;
    char color_moving = color_to_move(game_state) == 'w' ? 'b' : 'w';

    while (copy_pos_mov) {
        int next_position = get_lowest_bit_index(copy_pos_mov);
        Square output_square = square_from_position(next_position);
        make_move(input_square, output_square, pieces, game_state,
                  update_state);

        if (is_check(pieces, game_state, color_moving)) {
            unset_bit(pos_mov, next_position);
        }
        make_move(output_square, input_square, pieces, game_state,
                  update_state);
        if (game_state->last_captured_piece != '\0') {
            int output_pos =
                get_position(output_square.file, output_square.row);
            for (int i = 0; i < 12; i++) {
                if (pieces[i].symbol == game_state->last_captured_piece) {
                    set_bit(pieces[i].pos_bb, output_pos);
                    break;
                }
            }
            game_state->last_captured_piece = '\0';
        }
        copy_pos_mov &= copy_pos_mov - 1;
    }
}

int main()
{
    GameState game_state = {0};
    char board[8][8];

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n",
               IMG_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window =
        SDL_CreateWindow("Chessboard", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 600, 600, SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Event event;
    Piece *pieces = get_pieces();
    bitboards_to_board(pieces, board);

    Square selected_square = {-1, -1};
    Square previous_square = {-1, -1};
    int awaiting_promotion = 0;

    int needs_redraw = 1;
    int running = 1;
    int piece_selected = 0;
    int promotion_rendered = 0;
    uint64_t pos_mov = (uint64_t) 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int sel_file = event.button.x / SQUARE_SIZE;
                int sel_row = 7 - (event.button.y / SQUARE_SIZE);

                if (promotion_rendered) {
                    char color_moving = color_to_move(&game_state);
                    game_state.promote_pawn = 1;
                    game_state.promote_to =
                        get_promotion_piece(color_moving, sel_row);
                    char last_move[5];
                    squares_to_notation(previous_square, selected_square,
                                        last_move);
                    snprintf(game_state.last_move, sizeof(game_state.last_move),
                             "%s", last_move);

                    int update_state = 1;
                    make_move(previous_square, selected_square, pieces,
                              &game_state, update_state);
                    selected_square = (Square) {-1, -1};
                    needs_redraw = 1;
                    promotion_rendered = 0;
                    break;
                }

                previous_square =
                    (Square) {selected_square.file, selected_square.row};

                if (sel_file == selected_square.file &&
                    sel_row == selected_square.row) {
                    piece_selected = 0;
                    selected_square = (Square) {-1, -1};
                    pos_mov = (uint64_t) 0;
                    needs_redraw = 1;
                } else {
                    int new_position = get_position(sel_file, sel_row);
                    Piece *selected_piece =
                        find_piece_by_position(new_position);
                    if (selected_piece != NULL &&
                        color_to_move(&game_state) != selected_piece->color &&
                        !(is_bit_set(pos_mov, new_position))) {
                        break;
                    }
                    selected_square = (Square) {sel_file, sel_row};
                }

                if (!(selected_square.file == -1 &&
                      selected_square.row == -1)) {
                    int position =
                        get_position(selected_square.file, selected_square.row);

                    Piece *piece = find_piece_by_position(position);

                    if (piece_selected && is_bit_set(pos_mov, position)) {
                        int previous_position = get_position(
                            previous_square.file, previous_square.row);
                        Piece *previous_piece =
                            find_piece_by_position(previous_position);
                        if (previous_piece != NULL &&
                            ((previous_piece->symbol == 'P' &&
                              selected_square.row == 7) ||
                             (previous_piece->symbol == 'p' &&
                              selected_square.row == 0))) {
                            awaiting_promotion = 1;
                        } else {
                            char last_move[5];
                            squares_to_notation(previous_square,
                                                selected_square, last_move);
                            snprintf(game_state.last_move,
                                     sizeof(game_state.last_move), "%s",
                                     last_move);
                            int update_state = 1;
                            make_move(previous_square, selected_square, pieces,
                                      &game_state, update_state);
                            selected_square = (Square) {-1, -1};
                        }
                        piece_selected = 0;
                        pos_mov = (uint64_t) 0;
                        needs_redraw = 1;
                    } else if (!(piece == NULL)) {
                        piece_selected = 1;
                        pos_mov = find_possible_moves(selected_square, position,
                                                      piece, &game_state);
                        if (game_state.is_check) {
                            validate_possible_moves_solve_check(
                                &pos_mov, selected_square, &game_state);
                        }
                        needs_redraw = 1;
                    } else {
                        piece_selected = 0;
                        pos_mov = (uint64_t) 0;
                        needs_redraw = 0;
                    }
                }
            }
        }

        if (awaiting_promotion) {
            char color_moving = color_to_move(&game_state);
            int direction = color_moving == 'w' ? 1 : -1;
            char promotion_pieces[4];
            get_promotion_pieces(color_moving, promotion_pieces);
            awaiting_promotion = 0;

            render_promotion_squares(renderer, selected_square, pieces,
                                     direction, promotion_pieces, &game_state);
            needs_redraw = 0;
            promotion_rendered = 1;
        }

        if (needs_redraw) {
            int render_bool = 1;
            pieces = get_pieces();
            bitboards_to_board(pieces, board);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            render_board(renderer, board, pieces, selected_square, pos_mov,
                         render_bool, &game_state);
            needs_redraw = 0;
        }
        SDL_Delay(10);
    }

    return 0;
}