#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

extern FILE* logger_fd;
extern char* logger_filepath;

extern int depth;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef u8 byte;

typedef enum
{
  // Ordered by decreasing severity. If we specify DebugLevelWarning, then we
  // should print errors and warnings but not info.
  DebugLevelNone,

  DebugLevelError,
  DebugLevelWarning,
  DebugLevelInfo,
  DebugLevelDebug, // Used for throwaway prints

  DebugLevelAll, // Keep this at the end of the enum
} DebugLevel;

typedef enum
{
  MessageTypeLegalMoveRequest,
  MessageTypeLegalMoveReply,
  MessageTypeMakeMoveRequest,
  MessageTypeMakeMoveReply,
  MessageTypeBestMoveRequest,
  MessageTypeBestMoveReply,
  MessageTypeBoardStateRequest,
  MessageTypeBoardStateReply,
  MessageTypeGetMovesRequest,
  MessageTypeGetMovesReply,
  MessageTypeSetBoardRequest,
  MessageTypeSetBoardReply,
  // We need to use this to pad out the enum to make sure it's always
  // sizeof(int)
  __MessageTypeSizeMarker = 1 << (sizeof(int) - 1),
} MessageType;

typedef struct
{
  u32 len; // Size of data in bytes
  MessageType type;
  byte* data;
} Message;

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
  bool white_to_move;
  int en_passant_tile; // Set to -1 if en passant not possible

  // Stores whether castling is possible for each player, 1st element is black,
  // 2nd is white. This allows can_castle_ks[isWhite]
  bool can_castle_qs[2];
  bool can_castle_ks[2];

  int halfmove_clock;
  int fullmove_count;
} Board;

enum
{
  GetMovesWhite = 1 << 0,
  GetMovesBlack = 1 << 1,
  ConsiderChecks = 1 << 2,
};
typedef struct Node Node;
struct Node
{
  Node* parent;
  Node** children;
  size_t nchilds;
  int best_child; // Holds index to best child

  Move move;
  int value;    // The value of the best line that can be taken from this board
                // state
  bool isWhite; // Is it white's turn after this move
};

typedef struct Tree Tree;
struct Tree
{
  Node* root;
  Board board;
  size_t depth;
};
