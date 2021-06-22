#include "evaluate.h"

#include <stdio.h>

int table_white_pawn[64] = {
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 15, 15, 000, 000, 000,
  000, 000, 15, 30, 30, 15, 000, 000,
  10, 15, 20, 25, 25, 20, 15, 10,
  20, 20, 20, 000, 000, 20, 20, 20,
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
};

int table_white_knight[64] = {
  -90, -90, -90, -90, -90, -90, -90, -90,
  -90, 000, 000, 000, 000, 000, 000, -90,
  -50, 60, 90, 75, 75, 90, 60, -50,
  -70, 000, 60, 30, 30, 60, 000, -70,
  -70, 000, 60, 30, 30, 60, 000, -70,
  -70, 000, 60, 60, 60, 60, 000, -70,
  -70, -30, -30, -30, -30, -30, -30, -70,
  -90, -70, -70, -70, -70, -70, -70, -90,
};

int table_white_bishop[64] = {
  -90, -90, -90, -90, 000, -90, -90, -90,
  -90, -60, -60, -60, -60, -60, -60, -90,
  -60, -60, 000, 000, 000, 000, -60, -60,
  000, 000, 40, 40, 40, 40, 000, 000,
  000, 40, 40, 40, 40, 40, 40, 000,
  000, 40, 40, 40, 40, 40, 40, 000,
  -30, 000, 000, 000, 000, 000, 000, -30,
  -30, -30, -30, -30, -30, -30, -30, -30,
};

int table_white_castle[64] = {
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
};

int table_white_queen[64] = {
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
  000, 000, 000, 000, 000, 000, 000, 000,
};

int table_white_king[64] = {
  -90, -90, -90, -90, -90, -90, -90, -90,
  -90, -90, -90, -90, -90, -90, -90, -90,
  -90, -90, -90, -90, -90, -90, -90, -90,
  -90, -90, -90, -90, -90, -90, -90, -90,
  -40, -40, -90, -90, -90, -90, -40, -40,
  000, 000, -40, -40, -40, -40, 000, 000,
  50, 50, 000, -40, -40, 000, 50, 50,
  100, 100, 50, -40, -40, 50, 100, 100,
};


int table_black_pawn[64];
int table_black_knight[64];
int table_black_bishop[64];
int table_black_castle[64];
int table_black_queen[64];
int table_black_king[64];

void table_black_init()
{
  for (int i = 0; i < 64; i++)
  {
    table_black_pawn[i] = -1 * table_white_pawn[63 - i];
    table_black_knight[i] = -1 * table_white_knight[63 - i];
    table_black_bishop[i] = -1 * table_white_bishop[63 - i];
    table_black_castle[i] = -1 * table_white_castle[63 - i];
    table_black_king[i] = -1 * table_white_king[63 - i];
  }
}

int get_positional_value(Board board)
{
  int value = 0;
  for (int i = 0; i < 64; i++)
  {
    if (board.state[i] == ChessPieceNone)
      continue;

    switch ((int)board.state[i])
    {
      case ChessPiecePawn:
        value += table_black_pawn[i]; break;
      case ChessPieceKnight:
        value += table_black_knight[i]; break;
      case ChessPieceBishop:
        value += table_black_bishop[i]; break;
      case ChessPieceCastle:
        value += table_black_castle[i]; break;
      case ChessPieceQueen:
        value += table_black_queen[i]; break;
      case ChessPieceKing:
        value += table_black_king[i]; break;

      case ChessPiecePawn | ChessPieceIsWhite:
        value += table_white_pawn[i]; break;
      case ChessPieceKnight | ChessPieceIsWhite:
        value += table_white_knight[i]; break;
      case ChessPieceBishop | ChessPieceIsWhite:
        value += table_white_bishop[i]; break;
      case ChessPieceCastle | ChessPieceIsWhite:
        value += table_white_castle[i]; break;
      case ChessPieceQueen | ChessPieceIsWhite:
        value += table_white_queen[i]; break;
      case ChessPieceKing | ChessPieceIsWhite:
        value += table_white_king[i]; break;
      default:
        break;
    }
  }
  return value;
}

int get_piece_value(Board board)
{
  int value = 0;
  // @@Implement dictionary and use dictionary instead of switch
   for (int i = 0; i < 64; i++)
  {
    ChessPiece piece = board.state[i];
    int sign;
    if (piece & ChessPieceIsWhite)
      sign = 1;
    else
      sign = -1;

    piece &= ~ChessPieceIsWhite;
    switch(piece)
    {
      case ChessPiecePawn:
        value += sign * 100; break;
      case ChessPieceKnight:
      case ChessPieceBishop:
        value += sign * 350; break;
      case ChessPieceCastle:
        value += sign * 525; break;
      case ChessPieceQueen:
        value += sign * 1000; break;
      default:
        break;
    }
  }
  return value;
}

int evaluate_board(Board board)
{
  int value = 0;

  value += get_piece_value(board);
  value += get_positional_value(board);

  return value;
}


