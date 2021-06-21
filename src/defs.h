#pragma once

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef enum
{
  ChessPieceNone = 0,
  ChessPiecePawn = 1 << 0,
  ChessPieceKnight = 1 << 1,
  ChessPieceBishop = 1 << 2,
  ChessPieceCastle = 1 << 3,
  ChessPieceQueen = 1 << 4,
  ChessPieceKing = 1 << 5,
  ChessPieceIsWhite = 1 << 6
} ChessPiece;

typedef struct
{
  u8 from;
  u8 to;
} Move;

typedef struct
{
  ChessPiece state[64];
} Board;

enum
{
  GetMovesWhite = 1 << 0,
  GetMovesBlack = 1 << 1,
};
