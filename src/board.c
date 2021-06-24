#include <chess/board.h>
#include <chess/search.h>
#include <chess/util.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void board_new(Board* board, char* fen)
{
  memset(board, 0, sizeof(*board));
  ChessPiece* pieces = parse_fen(fen);
  for (int i = 0; i < 64; i++)
    board->state[i] = pieces[i];
}

char* board_tostring(Board board)
{
  char* str = calloc(1, 4096);
  char* tmp = calloc(1, 4096);
  for (int rank = 0; rank < 8; rank++)
  {
    for (int file = 0; file < 8; file++)
    {
      sprintf(tmp, "%c ", piece_to_char(board.state[file + 8 * rank]));
      strcat(str, tmp);
    }
    strcat(str, "\n");
  }
  free(tmp);
  str[strlen(str) - 1] = '\0'; // Remove trailng newline
  str = realloc(str, strlen(str) + 1);
  return str;
}

void board_update(Board* board, Move* move)
{
  board->state[move->to] = board->state[move->from];
  board->state[move->from] = ChessPieceNone;
}

Move* board_calculate_line(Board board, int depth, bool maximising_player)
{
  Move best_move;
  Move* line = calloc(depth, sizeof(Move));
  for (int i = 0; i < depth; i++)
  {
    minimax(board, depth - i, -INT_MAX, INT_MAX, maximising_player, &best_move,
            NULL);
    board_update(&board, &best_move);
    line[i] = best_move;
    maximising_player = !maximising_player;
  }
  return line;
}

// Checks that we're not doing a self capture.
// NOTE: Does not check for off-board wrapping
bool board_can_capture_or_move(Board board, int origin_pos, int target_pos)
{
  if (board.state[target_pos] == ChessPieceNone)
    return true;
  return (board.state[origin_pos] & ChessPieceIsWhite) ^
         (board.state[target_pos] & ChessPieceIsWhite);
}

void board_get_moves(Board _board, int pos, Move** moves, size_t* nmoves)
{
  size_t idx = 0;
  *moves = malloc(64 * sizeof(Move)); // Can be at most 64 moves

  u8 pos_88 = topos88(pos);

  if (pos_88 & 0x88)
    goto end;

  ChessPiece* board = _board.state;

  // Want to check:
  //  If we're off the board
  //  We aren't doing a self capture

  if (board[pos] & ChessPieceKnight)
  {
    // We use the 0x88 offsets here to check for off-board wrapping
    int move_offsets[] = {14, 18, -14, -18, 33, 31, -33, -31};
    for (int i = 0; i < 8; i++)
    {
      int tpos_88 = pos_88 + move_offsets[i];
      if (tpos_88 & 0x88)
        continue;
      if (board_can_capture_or_move(_board, pos, topos64(tpos_88)))
        (*moves)[idx++] = move_new(pos, topos64(tpos_88));
    }
  }

  if (board[pos] & ChessPiecePawn)
  {
    bool isWhite = board[pos] & ChessPieceIsWhite;
    int dirsgn = isWhite ? -1 : 1; // Pawns can only move forwards

    // Pawns can double move at the start
    int double_move_rank = isWhite ? 6 : 1;
    if (torank64(pos) == double_move_rank &&
        board[pos + dirsgn * 16] == ChessPieceNone)
      (*moves)[idx++] = move_new(pos, pos - 16);

    int tpos_88 = pos_88 + dirsgn * 16;

    // Pawns can only move forwards to an empty square
    if (!(tpos_88 & 0x88) && board[topos64(tpos_88)] == ChessPieceNone)
      (*moves)[idx++] = move_new(pos, topos64(tpos_88));

    int diagonals[] = {15, 17};
    for (int i = 0; i < 2; i++)
    {
      int tpos_88 = pos_88 + dirsgn * diagonals[i];
      int tpos = topos64(tpos);
      if (tpos_88 & 0x88)
        continue;

      // Pawns can only move diagonally if they're capturing a piece
      bool can_capture =
          board_can_capture_or_move(_board, pos, tpos) && board[tpos];

      if (can_capture)
        (*moves)[idx++] = move_new(pos, tpos);
    }
  }

  // Sliding moves (no castling)
  if (board[pos] &
      (ChessPieceBishop | ChessPieceCastle | ChessPieceQueen | ChessPieceKing))
  {
    int slide_offset[] = {-16, 16, -1, 1, 17, 15, -17, -15};

    int start = 0, end = 8;
    if (board[pos] & ChessPieceBishop)
      start = 4;
    else if (board[pos] & ChessPieceCastle)
      end = 4;

    for (int i = start; i < end; i++)
    {
      int tpos_88 = topos88(pos) + slide_offset[i];
      while (!(tpos_88 & 0x88) &&
             board_can_capture_or_move(_board, pos, topos64(tpos_88)))
      {
        (*moves)[idx++] = move_new(pos, topos64(tpos_88));

        if (board[topos64(tpos_88)])
          break;

        tpos_88 += slide_offset[i];
        if (board[pos] & ChessPieceKing)
          break;
      }
    }
  }

  // @@Implement Castling

end:
  *nmoves = idx;
}

void board_get_moves_all(Board board, Move** moves, size_t* nmoves, int flags)
{
  size_t idx = 0, current_alloc = 512;
  *moves = malloc(512 * sizeof(**moves));

  for (int i = 0; i < 64; i++)
  {
    if ((flags & GetMovesWhite) && !(flags & GetMovesBlack))
      if (!(board.state[i] & ChessPieceIsWhite))
        continue;
    if ((flags & GetMovesBlack) && !(flags & GetMovesWhite))
      if (board.state[i] & ChessPieceIsWhite)
        continue;

    Move* piece_moves;
    size_t nmoves = 0;
    board_get_moves(board, i, &piece_moves, &nmoves);
    for (size_t j = 0; j < nmoves; j++)
    {
      (*moves)[idx++] = piece_moves[j];
      if (idx == current_alloc)
      {
        current_alloc *= 2;
        *moves = realloc(*moves, current_alloc);
      }
    }
    free(piece_moves);
  }
  *nmoves = idx;
}
