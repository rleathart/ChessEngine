#include "piece.h"

#include <ctype.h>

ChessPiece piece_from_char(char c)
{
  ChessPiece piece;
  switch (tolower(c))
  {
  case '0':
    piece = ChessPieceNone;
    break;
  case 'p':
    piece = ChessPiecePawn;
    break;
  case 'n':
    piece = ChessPieceKnight;
    break;
  case 'b':
    piece = ChessPieceBishop;
    break;
  case 'r':
    piece = ChessPieceCastle;
    break;
  case 'q':
    piece = ChessPieceQueen;
    break;
  case 'k':
    piece = ChessPieceKing;
    break;
  }
  if (isupper(c))
    piece |= ChessPieceIsWhite;
  return piece;
}

char piece_to_char(ChessPiece piece)
{
  char c;
  switch (piece & ~ChessPieceIsWhite)
  {
  case ChessPieceNone:
    c = '0';
    break;
  case ChessPiecePawn:
    c = 'p';
    break;
  case ChessPieceKnight:
    c = 'n';
    break;
  case ChessPieceBishop:
    c = 'b';
    break;
  case ChessPieceCastle:
    c = 'r';
    break;
  case ChessPieceQueen:
    c = 'q';
    break;
  case ChessPieceKing:
    c = 'k';
    break;
  }
  if (piece & ChessPieceIsWhite)
    c = toupper(c);
  return c;
}
