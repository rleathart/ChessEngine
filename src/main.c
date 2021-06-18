#define _CRT_SECURE_NO_DEPRECATE
#include <ipc/socket.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

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
  u8 from[2];
  u8 to[2];
} Move;

void move_print(Move move)
{
  printf("%d-%d to %d-%d\n", move.from[0], move.from[1], move.to[0],
         move.to[1]);
}

void sleep_ms(int ms)
{
#ifdef _WIN32
  sleep_ms(ms);
#else
  struct timespec ts = {
    .tv_sec = ms / 1000,
    .tv_nsec = (ms % 1000) * 1e6,
  };
  nanosleep(&ts, NULL);
#endif
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
  return pipename;
}

char* sockname = "ChessIPC";

ipcError socket_connect_check(Socket* sock)
{
  if (!(sock->state.flags & SocketConnected))
    return 1;
#ifdef _WIN32
  ReadFile(sock->server, NULL, 0, NULL, NULL);
  DWORD lasterr = GetLastError();
  switch (lasterr)
  {
  case ERROR_PIPE_NOT_CONNECTED:
  case ERROR_BROKEN_PIPE:
  case ERROR_NO_DATA:
    printf("Error: %lu %s\n", lasterr, win32err(lasterr));
    return 1;
  }
#else
#endif

  return ipcErrorNone;
}

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

typedef struct
{
  ChessPiece state[64];
} Board;

void board_init(Board* board)
{
  memset(board, 0, sizeof(*board));
  for (int i = 0; i < 8; i++)
  {
    board->state[i + 8 * 1] = ChessPiecePawn;
    board->state[i + 8 * 6] = ChessPiecePawn | ChessPieceIsWhite;
  }
  board->state[0] = board->state[7] = ChessPieceCastle;
  board->state[56] = board->state[63] = ChessPieceCastle | ChessPieceIsWhite;
  board->state[1] = board->state[6] = ChessPieceKnight;
  board->state[57] = board->state[62] = ChessPieceKnight | ChessPieceIsWhite;
  board->state[2] = board->state[5] = ChessPieceBishop;
  board->state[58] = board->state[61] = ChessPieceBishop | ChessPieceIsWhite;
  board->state[3] = ChessPieceQueen;
  board->state[59] = ChessPieceQueen | ChessPieceIsWhite;
  board->state[4] = ChessPieceKing;
  board->state[60] = ChessPieceKing | ChessPieceIsWhite;
}

void board_print(Board* board)
{
  for (int rank = 0; rank < 8; rank++)
  {
    for (int file = 0; file < 8; file++)
    {
      printf("%2d ", board->state[8 * rank + file]);
    }
    printf("\n");
  }
}

void board_update(Board* board, Move* move)
{
  ChessPiece fromPiece = board->state[move->from[0] + 8 * move->from[1]];
  board->state[move->from[0] + 8 * move->from[1]] = ChessPieceNone;
  board->state[move->to[0] + 8 * move->to[1]] = fromPiece;
}

bool is_legal_move(Board* _board, Move* move)
{
  int originFile = move->from[0];
  int originRank = move->from[1];
  int targetFile = move->to[0];
  int targetRank = move->to[1];
  int originPos = originRank * 8 + originFile;
  int targetPos = targetRank * 8 + targetFile;
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
      checkPos = checkFile + 8 * checkRank;
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

void move_get_legal(Board* board, Move* move)
{
  do
  {
    move->from[0] = randrange(0, 7);
    move->from[1] = randrange(0, 7);
    move->to[0] = randrange(0, 7);
    move->to[1] = randrange(0, 7);
  } while (!is_legal_move(board, move));
}

int main(int argc, char* argv[])
{
  ipcError err = 0;
  Move move = {};
  Socket sock;
  Board board;
  board_init(&board);
  board_print(&board);
  socket_init(&sock, get_dotnet_pipe_name(sockname), SocketServer);
  printf("%d\n", socket_is_connected(&sock));
  for (;;)
  {
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
    }
  }

  return 0;
}
