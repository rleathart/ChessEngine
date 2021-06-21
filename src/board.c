#include "board.h"
#include "util.h"

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

// Verifies if the target square is either empty or contains a piece of the
// opponent's colour
bool board_can_move(ChessPiece piece, Board board, int pos)
{
  int pos88 = topos88(pos);
  if (pos88 & 0x88) // We're off the board
    return false;
  if (!board.state[pos])
    return true;
  return (piece & ChessPieceIsWhite) ^ (board.state[pos] & ChessPieceIsWhite);
}

static void add_to_moves(Move** moves, Move move, size_t* current_alloc,
                         size_t* idx)
{
  (*moves)[(*idx)++] = move;
  if (*idx == *current_alloc)
  {
    *current_alloc *= 2;
    *moves = realloc(*moves, *current_alloc);
  }
}

void board_get_moves(Board _board, int pos, Move** moves, size_t* nmoves)
{
  size_t idx = 0, current_alloc = 64;
  *moves = malloc(current_alloc * sizeof(Move));

  u8 pos_88 = topos88(pos);

  if (pos_88 & 0x88)
    goto end;

  ChessPiece* board = _board.state;

#define AddMove(move) add_to_moves(moves, move, &current_alloc, &idx)
#define CanMoveTo(__pos) board_can_move(board[pos], _board, __pos)

  if (board[pos] & ChessPieceKnight)
  {
    // We use the 0x88 offsets here to check for off-board wrapping
    int move_offsets[] = {14, 18, -14, -18, 33, 31, -33, -31};
    for (int i = 0; i < 8; i++)
    {
      int tpos_88 = topos88(pos) + move_offsets[i];
      if (tpos_88 & 0x88)
        continue;
      if (CanMoveTo(topos64(tpos_88)))
        AddMove(move_new(pos, topos64(tpos_88)));
    }
  }

  if (board[pos] & ChessPiecePawn)
  {
    bool isWhite = board[pos] & ChessPieceIsWhite;
    if (isWhite && torank64(pos) == 6 && board[pos - 16] == ChessPieceNone)
    {
      AddMove(move_new(pos, pos - 16));
    }
    if (!isWhite && torank64(pos) == 1 && board[pos + 16] == ChessPieceNone)
    {
      AddMove(move_new(pos, pos + 16));
    }
    int dirsgn = isWhite ? -1 : 1; // Pawns can only move forwards
    int tpos = pos + dirsgn * 8;
    if (!(topos88(tpos) & 0x88) &&
        !board[tpos]) // Can only move forwards to empty square
    {
      AddMove(move_new(pos, pos + dirsgn * 8));
    }
    int diagonals[] = {7, 9};
    for (int i = 0; i < 2; i++)
    {
      int tpos = pos + dirsgn * diagonals[i];
      if (topos88(tpos) & 0x88)
        continue;
      bool canCapture = isWhite && !(board[tpos] & ChessPieceIsWhite);
      canCapture = canCapture || !isWhite && (board[tpos] & ChessPieceIsWhite);
      canCapture = canCapture && board[tpos];
      if (canCapture)
      {
        AddMove(move_new(pos, pos + dirsgn * diagonals[i]));
      }
    }
  }

  if (board[pos] & (ChessPieceBishop | ChessPieceQueen | ChessPieceKing))
  {
    int slide_offset[] = {9, 7, -9, -7};
    for (int i = 0; i < 4; i++)
    {
      int tpos = pos + slide_offset[i];
      while (!(tpos & 0x88) && CanMoveTo(tpos))
      {
        AddMove(move_new(pos, tpos));
        tpos += slide_offset[i];
        if (board[pos] & ChessPieceKing)
          break;
      }
    }
  }

  if (board[pos] & (ChessPieceCastle | ChessPieceQueen | ChessPieceKing))
  {
    int slide_offset[] = {-8, 8, -1, 1};
    for (int i = 0; i < 4; i++)
    {
      // Target position
      int tpos = pos + slide_offset[i];
      while (!(tpos & 0x88) && CanMoveTo(tpos))
      {
        AddMove(move_new(pos, tpos));
        tpos += slide_offset[i];
        if (board[pos] & ChessPieceKing)
          break;
      }
    }
  }

end:
  *nmoves = idx;
#undef CanMoveTo
#undef AddMove
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
