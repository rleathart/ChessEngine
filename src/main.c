#define _CRT_SECURE_NO_DEPRECATE
#define _USE_MATH_DEFINES
#include "matrix.h"
#include <assert.h>
#include <ipc/socket.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

void sleep_ms(int ms)
{
#ifdef _WIN32
  Sleep(ms);
#else
  struct timespec ts = {
      .tv_sec = ms / 1000,
      .tv_nsec = (ms % 1000) * 1e6,
  };
  nanosleep(&ts, NULL);
#endif
}

void* xmalloc(size_t size)
{
  void* p = malloc(size);
  if (p)
    return p;

  fprintf(stderr, "Error: Failed to allocate memory\n");
  exit(1);
}

void* xrealloc(void* old, size_t size)
{
  void* p = realloc(old, size);
  if (p)
    return p;

  fprintf(stderr, "Error: Failed to allocate memory\n");
  exit(1);
}

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef struct
{
  u8 data[2];
} Move;

enum
{
  Pos64 = 1 << 0,
  Pos88 = 1 << 1,
};

u8 pos64(u8 pos88)
{
  return (pos88 + (pos88 & 7)) >> 1;
}
u8 pos64fr(u8 file, u8 rank)
{
  return file + 8 * rank;
}
u8 pos88(u8 pos64)
{
  return pos64 + (pos64 & ~7);
}
u8 pos88fr(u8 file, u8 rank)
{
  return file + 16 * rank;
}
u8 file64(u8 pos64)
{
  return pos64 % 8;
}
u8 rank64(u8 pos64)
{
  return pos64 / 8;
}
u8 file88(u8 pos88)
{
  return pos88 % 16;
}
u8 rank88(u8 pos88)
{
  return pos88 / 16;
}

// New move from 0x88 positions
Move move_new(int from, int to)
{
  Move move = {
      .data = {from, to},
  };
  return move;
}

void move_print(Move move)
{
  printf("%02x-%02x\n", move.data[0], move.data[1]);
}

char* get_dotnet_pipe_name(char* name)
{
  char* pipename = calloc(1, 4096);
#ifdef _WIN32
  strcat(pipename, "\\\\.\\pipe\\");
  strcat(pipename, name);
#else
  char* tmpdir = getenv("TMPDIR");
  if (tmpdir)
    strcat(pipename, tmpdir);
  else
    strcat(pipename, "/tmp/");
  strcat(pipename, "CoreFxPipe_");
  strcat(pipename, name);
#endif
  pipename = realloc(pipename, strlen(pipename) + 1);
  return pipename;
}

char* sockname = "ChessIPC";

/* Return a random int between lower and upper inclusive. */
int randrange(int lower, int upper)
{
  return (rand() % (upper - lower + 1)) + lower;
}

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

typedef struct
{
  ChessPiece state[128]; // 0x88 representation of board
} Board;

ChessPiece* parse_fen(char* fen)
{
  ChessPiece* pieces = calloc(64, sizeof(ChessPiece));
  int idx = 0;
  for (char c = *fen; c; c = *(++fen))
  {
    if (c == '/')
      continue;
    if (c == ' ')
      break;

    char cstr[] = {c, '\0'};
    if (atoi(cstr))
      for (int i = 0; i < atoi(cstr); i++)
        pieces[idx++] = ChessPieceNone;
    else
      pieces[idx++] = piece_from_char(c);
  }
  return pieces;
}

void board_init(Board* board, char* fen)
{
  memset(board, 0, sizeof(*board));
  ChessPiece* pieces = parse_fen(fen);
  for (int i = 0; i < 64; i++)
    board->state[pos88(i)] = pieces[i];
}

void board_print(Board* board)
{
  for (int rank = 0; rank < 8; rank++)
  {
    for (int file = 0; file < 8; file++)
    {
      printf("%c ", piece_to_char(board->state[16 * rank + file]));
    }
    printf("\n");
  }
}

void board_update(Board* board, Move* move)
{
  board->state[move->data[1]] = board->state[move->data[0]];
  board->state[move->data[0]] = ChessPieceNone;
}

bool is_legal_move(Board* _board, Move* move)
{
  if (move->data[0] & 0x88 || move->data[1] & 0x88)
    return false;
  int originFile = move->data[0] % 16;
  int originRank = move->data[0] / 16;
  int targetFile = move->data[1] % 16;
  int targetRank = move->data[1] / 16;
  int originPos = originRank * 16 + originFile;
  int targetPos = targetRank * 16 + targetFile;
  ChessPiece* board = _board->state;
  ChessPiece originPieceColor = board[originPos] & ChessPieceIsWhite;
  ChessPiece targetPieceColor = board[targetPos] & ChessPieceIsWhite;

  if (board[originPos] == ChessPieceNone)
    return false;
  // Prevent self capture
  if (ChessPieceNone != board[targetPos] &&
      (originPieceColor ^ targetPieceColor) == 0)
    return false;

  // Only Knights can jump over pieces
  if ((board[originPos] & ChessPieceKnight) == 0)
  {
    int dirVec[] = {
        targetFile - originFile,
        targetRank - originRank,
    };

    for (int i = 0; i < 2; i++)
      if (dirVec[i] != 0)
        dirVec[i] /= abs(dirVec[i]);

    int checkPos = originPos;
    int checkFile = originFile;
    int checkRank = originRank;
    while (true)
    {
      checkFile += dirVec[0];
      checkRank += dirVec[1];
      checkPos = checkFile + 16 * checkRank;
      if (checkPos < 0 || checkPos >= 64)
        return false;
      if (checkPos == targetPos)
        break;
      if (board[checkPos] != ChessPieceNone)
        return false;
    }
  }

  if (0 != (board[originPos] & ChessPieceKnight))
  {
    if (abs(originRank - targetRank) == 2 && abs(originFile - targetFile) == 1)
      return true;
    else if (abs(originRank - targetRank) == 1 &&
             abs(originFile - targetFile) == 2)
      return true;
    else
      return false;
  }

  if ((board[originPos] & (ChessPieceBishop | ChessPieceQueen)) != 0)
  {
    if (abs(targetRank - originRank) == abs(targetFile - originFile))
      return true;
  }

  if ((board[originPos] & (ChessPieceCastle | ChessPieceQueen)) != 0)
  {
    if (targetRank == originRank)
      return true;
    if (targetFile == originFile)
      return true;
  }

  if ((board[originPos] & ChessPiecePawn) != 0)
  {
    // Pawns can only move forwards
    if ((board[originPos] & ChessPieceIsWhite) != 0)
    {
      if (targetRank > originRank)
        return false;
    }
    else
    {
      if (targetRank < originRank)
        return false;
    }

    // Allow moving 2 squares at the start
    if (originRank == 1 || originRank == 6)
    {
      if (abs(targetRank - originRank) > 2)
        return false;
    }
    else
    {
      if (abs(targetRank - originRank) > 1)
        return false;
    }

    if (targetFile == originFile && board[targetPos] == ChessPieceNone)
      return true;

    // Can't move horizontally
    if (targetRank == originRank)
      return false;

    // Can take pieces diagonally 1 square in front
    if (board[targetPos] == ChessPieceNone)
      return false;
    if ((abs(targetFile - originFile) == 1 &&
         abs(targetRank - originRank) == 1))
      return true;
  }

  if ((board[originPos] & ChessPieceKing) != 0)
  {
    if (abs(targetRank - originRank) <= 1 && abs(targetFile - originFile) <= 1)
      return true;
  }

  return false;
}

int board_get_pos_from_offset_matrix(int pos, Matrix mat)
{
  // We need a vector.
  assert(mat.m == 2);
  assert(mat.n == 1);
  Scalar i = matrix_get_elem(mat, 0, 0);
  Scalar j = matrix_get_elem(mat, 1, 0);
  return pos + i + 8 * j;
}

int pos_from_filerank(int file, int rank)
{
  return file + 8 * rank;
}

int* filerank_from_board_pos(int pos)
{
  int* rv = malloc(2 * sizeof(int));
  rv[0] = pos % 8; // file
  rv[1] = pos / 8; // rank
  return rv;
}

// Verifies if the target square is either empty or contains a piece of the
// opponent's colour
bool piece_can_move(ChessPiece piece, Board board, int pos88)
{
  if (pos88 & 0x88) // We're off the board
    return false;
  if (!board.state[pos88])
    return true;
  return (piece & ChessPieceIsWhite) ^ (board.state[pos88] & ChessPieceIsWhite);
}

void piece_get_moves(Board _board, int pos, Move** moves, size_t* nmoves)
{
  size_t idx = 0, current_alloc = 64;
  *moves = xmalloc(current_alloc * sizeof(Move));

  ChessPiece* board = _board.state;

#define CanMoveTo(__pos88) piece_can_move(board[pos], _board, __pos88)
#define ResizeIfNeeded()                                                       \
  do                                                                           \
  {                                                                            \
    if (idx == current_alloc)                                                  \
    {                                                                          \
      current_alloc *= 2;                                                      \
      *moves = xrealloc(*moves, current_alloc);                                \
    }                                                                          \
  } while (0)

  if (_board.state[pos] & ChessPieceKnight)
  {
    for (int file = file88(pos) - 2; file <= file88(pos) + 2; file++)
    {
      for (int rank = rank88(pos) - 2; rank <= rank88(pos) + 2; rank++)
      {
        if ((abs(file - file88(pos)) == 2 && abs(rank - rank88(pos)) == 1) ||
            (abs(file - file88(pos)) == 1 && abs(rank - rank88(pos)) == 2))
        {
          if (CanMoveTo(pos88fr(file, rank)))
          {
            (*moves)[idx++] = move_new(pos, pos88fr(file, rank));
            ResizeIfNeeded();
          }
        }
      }
    }
  }

  if (board[pos] & ChessPiecePawn)
  {
    bool isWhite = board[pos] & ChessPieceIsWhite;
    if (isWhite && rank88(pos) == 6 && CanMoveTo(pos - 32))
    {
      (*moves)[idx++] = move_new(pos, pos - 32);
      ResizeIfNeeded();
    }
    if (!isWhite && rank88(pos) == 1 && CanMoveTo(pos + 32))
    {
      ResizeIfNeeded();
      (*moves)[idx++] = move_new(pos, pos + 32);
    }
    int dirsgn = isWhite ? -1 : 1; // Pawns can only move forwards
    if (!(pos + dirsgn * 16 & 0x88) &&
        !board[pos + dirsgn * 16]) // Can only move forwards to empty square
    {
      ResizeIfNeeded();
      (*moves)[idx++] = move_new(pos, pos + dirsgn * 16);
    }
    int diagonals[] = {15, 17};
    for (int i = 0; i < 2; i++)
    {
      int tpos = pos + dirsgn * diagonals[i];
      if (tpos & 0x88)
        continue;
      bool canCapture = isWhite && !(board[tpos] & ChessPieceIsWhite);
      canCapture = canCapture || !isWhite && (board[tpos] & ChessPieceIsWhite);
      canCapture = canCapture && board[tpos];
      if (canCapture)
      {
        ResizeIfNeeded();
        (*moves)[idx++] = move_new(pos, pos + dirsgn * diagonals[i]);
      }
    }
  }

  if (board[pos] & (ChessPieceBishop | ChessPieceQueen | ChessPieceKing))
  {
    int slide_offset[] = {17, 15, -17, -15};
    for (int i = 0; i < 4; i++)
    {
      int tpos = pos + slide_offset[i];
      while (!(tpos & 0x88) && CanMoveTo(tpos))
      {
        (*moves)[idx++] = move_new(pos, tpos);
        ResizeIfNeeded();
        tpos += slide_offset[i];
        if (board[pos] & ChessPieceKing)
          break;
      }
    }
  }

  if (board[pos] & (ChessPieceCastle | ChessPieceQueen | ChessPieceKing))
  {
    int slide_offset[] = {-16, 16, -1, 1};
    for (int i = 0; i < 4; i++)
    {
      // Target position
      int tpos = pos + slide_offset[i];
      while (!(tpos & 0x88) && CanMoveTo(tpos))
      {
        (*moves)[idx++] = move_new(pos, tpos);
        ResizeIfNeeded();
        tpos += slide_offset[i];
        if (board[pos] & ChessPieceKing)
          break;
      }
    }
  }

  *nmoves = idx;
#undef CanMoveTo
#undef ResizeIfNeeded
}

void get_moves(Board board, Move** moves, size_t* nmoves)
{
  size_t idx = 0, current_alloc = 512;
  *moves = xmalloc(512 * sizeof(**moves));

  for (int i = 0; i < 64; i++)
  {
    Move* piece_moves;
    size_t nmoves = 0;
    piece_get_moves(board, pos88(i), &piece_moves, &nmoves);
    for (size_t j = 0; j < nmoves; j++)
    {
      (*moves)[idx++] = piece_moves[j];
      if (idx == current_alloc)
      {
        current_alloc *= 2;
        *moves = xrealloc(*moves, current_alloc);
      }
    }
  }
  *nmoves = idx;
}

void move_get_legal(Board* board, Move* move)
{
  do
  {
    move->data[0] = randrange(0, 0x88);
    move->data[1] = randrange(0, 0x88);
  } while (!is_legal_move(board, move));
}

typedef struct
{
  int wPieces;
  int bPieces;
} StaticEval;

int minimax(Board board, size_t depth, bool maximising_player)
{
  int rv = 0;
  if (depth == 0)
  {
    // How many pieces does each player have?
    for (int i = 0; i < 64; i++)
    {
      if (board.state[i] && (board.state[i] & ChessPieceIsWhite))
        rv++;
      if (board.state[i] && !(board.state[i] & ChessPieceIsWhite))
        rv--;
    }
    return rv;
  }

  if (maximising_player)
  {
    int max_eval = -INT_MAX;
    Board new_board = board;
    Move* moves;
    size_t nmoves = 0;
    get_moves(board, &moves, &nmoves);
    for (int i = 0; i < 64; i++)
    {
      board_update(&new_board, &(moves[i]));
      int eval = minimax(new_board, depth - 1, false);
      max_eval = max(max_eval, eval);
    }
    return max_eval;
  }
  else
  {
    int min_eval = INT_MAX;
    Board new_board = board;
    Move* moves;
    size_t nmoves = 0;
    get_moves(board, &moves, &nmoves);
    for (int i = 0; i < 64; i++)
    {
      board_update(&new_board, &(moves[i]));
      int eval = minimax(new_board, depth - 1, true);
      min_eval = max(min_eval, eval);
    }
    return min_eval;
  }
  return rv;
}

int main(int argc, char* argv[])
{
  ipcError err = 0;
  Move move = {};
  Socket sock;
  Board board;
  board_init(&board,
             "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  board_print(&board);
  Scalar knight_arr[] = {2, 1};
  Matrix mat = matrix_new(3, 3);
  Matrix knight = matrix_new_from_array(2, 1, knight_arr);
  matrix_print(knight);
  matrix_print(matrix_mul(matrix_new_rotation(M_PI), knight));
  Move* moves;
  size_t nmoves;
  piece_get_moves(board, pos88(62), &moves, &nmoves);
  for (int i = 0; i < nmoves; i++)
    move_print(moves[i]);
  move = move_new(pos88fr(6, 6), pos88fr(6, 5));
  /* board_update(&board, &move); */
  move = move_new(pos88fr(7, 6), pos88fr(7, 5));
  /* board_update(&board, &move); */
  board_print(&board);
  piece_get_moves(board, pos88fr(5, 7), &moves, &nmoves);
  for (int i = 0; i < nmoves; i++)
    move_print(moves[i]);
  printf("Pawn moves\n");
  piece_get_moves(board, pos88fr(0, 6), &moves, &nmoves);
  for (int i = 0; i < nmoves; i++)
    move_print(moves[i]);
  printf("Pawn moves 1\n");
  piece_get_moves(board, pos88fr(7, 5), &moves, &nmoves);
  for (int i = 0; i < nmoves; i++)
    move_print(moves[i]);
  printf("All moves\n");
  get_moves(board, &moves, &nmoves);
  for (int i = 0; i < nmoves; i++)
    move_print(moves[i]);
  printf("minimax: %d\n", minimax(board, 2, true));
  socket_init(&sock, get_dotnet_pipe_name(sockname), SocketServer);
  for (;;)
  {
    /* board_init(&board); */
    /* board_print(&board); */
    while ((err = socket_connect(&sock)))
    {
      /* fprintf(stderr, "Error[%d]: %s\n", err, ipcError_str(err)); */
      sleep_ms(200);
    }
    /* printf("%d\n", socket_is_connected(&sock)); */
    printf("Got connection\n");

    for (;;)
    {
      if (!socket_is_connected(&sock))
      {
        fprintf(stderr, "Error: Socket connection lost\n");
        break;
      }
      while (socket_read_bytes(&sock, &move, sizeof(move)) ==
             ipcErrorSocketHasMoreData)
        sleep_ms(10);

      move_print(move);
      board_update(&board, &move);

      Move server_move = {};
      move_get_legal(&board, &server_move);
      move_print(server_move);
      while (socket_write_bytes(&sock, &server_move, sizeof(move)) ==
             ipcErrorSocketHasMoreData)
        sleep_ms(10);
      board_update(&board, &server_move);
      board_print(&board);
      Move* moves;
      size_t nmoves = 0;
      get_moves(board, &moves, &nmoves);
      for (size_t i = 0; i < nmoves; i++)
        move_print(moves[i]);
    }
  }

  return 0;
}
