#include <rgl/logging.h>

#include <chess/board.h>
#include <chess/search.h>
#include <chess/tree.h>
#include <chess/util.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Does common initialisation for the Board struct
static void board_init(Board* board)
{
  memset(board, 0, sizeof(*board));
  board->en_passant_tile = -1;
  board->white_to_move = true;
  for (int i = 0; i < 2; i++)
    board->can_castle_ks[i] = board->can_castle_qs[i] = true;
}

static void parse_fen(Board* board, char* fen)
{
  board_init(board);
  int idx = 0;
  int stage = 0; // Which segment are we parsing? (board, turn, castling, ...)

  for (int i = 0; i < 2; i++)
    board->can_castle_ks[i] = board->can_castle_qs[i] = false;

  char ep_str[3] = {0};
  char hm_str[32] = {0};
  char fm_str[32] = {0};
  for (char c = *fen; c; c = *(++fen))
  {
    if (c == '/')
      continue;

    if (c == ' ')
    {
      stage++;
      continue;
    }

    char cstr[] = {c, '\0'};

    switch (stage)
    {
    case 0: // Board layout
      if (atoi(cstr))
        for (int i = 0; i < atoi(cstr); i++)
          board->state[idx++] = ChessPieceNone;
      else
        board->state[idx++] = piece_from_char(c);
      break;
    case 1: // Turn
      if (c == 'w')
        board->white_to_move = true;
      else if (c == 'b')
        board->white_to_move = false;
      break;
    case 2: // Castling
      switch (c)
      {
      case 'K':
        board->can_castle_ks[1] = true;
        break;
      case 'k':
        board->can_castle_ks[0] = true;
        break;
      case 'Q':
        board->can_castle_qs[1] = true;
        break;
      case 'q':
        board->can_castle_qs[0] = true;
        break;
      default:
        break;
      }
      break;

    case 3: // en passant
      if (c == '-')
      {
        board->en_passant_tile = -1;
        break;
      }

      idx = 0;
      while (isdigit(c))
      {
        ep_str[idx++] = c;
        c = *(++fen);
      }
      board->en_passant_tile = atoi(ep_str);
      c = *(--fen); // Since we increment at the end of the for loop, need to
                    // decrement here
      break;

    case 4: // Halfmove clock
      idx = 0;
      while (isdigit(c))
      {
        hm_str[idx++] = c;
        c = *(++fen);
      }
      board->halfmove_clock = atoi(hm_str);
      c = *(--fen);
      break;

    case 5: // Fullmove count
      idx = 0;
      while (isdigit(c))
      {
        fm_str[idx++] = c;
        c = *(++fen);
      }
      board->fullmove_count = atoi(fm_str);
      c = *(--fen);
      break;

    default:
      break;
    }
  }
}

void board_new(Board* board, char* fen)
{
  board_init(board);
  parse_fen(board, fen);
}

// Initialises a board from the string representation of a board.
//
// board_str is like that which board_tostring returns except without the
// newlines.
void board_new_from_string(Board* board, char* board_str)
{
  board_init(board);

  int idx = 0;
  for (char c = *board_str; *board_str; c = *(++board_str))
  {
    if (c == ' ')
      continue;
    board->state[idx++] = piece_from_char(c);
  }
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
  bool isWhite = board->state[move->to] & ChessPieceIsWhite;

  // Do castling move
  if (board->state[move->to] & ChessPieceKing)
  {
    if (board->can_castle_qs[isWhite] && move->to - move->from == -2)
    {
      board->state[move->to + 1] =
          ChessPieceCastle | (isWhite ? ChessPieceIsWhite : 0);
      board->state[move->to - 2] = ChessPieceNone;
    }
    else if (board->can_castle_ks[isWhite] && move->to - move->from == 2)
    {
      board->state[move->to - 1] =
          ChessPieceCastle | (isWhite ? ChessPieceIsWhite : 0);
      board->state[move->to + 1] = ChessPieceNone;
    }
  }

  // If we've moved our king we can't castle anymore
  if (board->state[move->to] & ChessPieceKing)
    board->can_castle_ks[isWhite] = board->can_castle_qs[isWhite] = false;
  // If we move our king/queen side castle then we can no longer castle on
  // that side
  if (board->state[move->to] & ChessPieceCastle)
  {
    if (tofile64(move->from) == 0)
      board->can_castle_qs[isWhite] = false;
    if (tofile64(move->from) == 7)
      board->can_castle_ks[isWhite] = false;
  }

  // Pawn promotion
  if (board->state[move->to] & ChessPiecePawn)
  {
    if (torank64(move->to) == (isWhite ? 0 : 7) && move->promotion)
    {
      ILOG("Promoting %d at %d to %d\n", board->state[move->to], move->to, move->promotion);
      board->state[move->to] =
          move->promotion | (isWhite ? ChessPieceIsWhite : 0);
    }
  }

  if (board->en_passant_tile >= 0 && move->to == board->en_passant_tile)
    board->state[move->to + (isWhite ? 8 : -8)] = ChessPieceNone;

  board->en_passant_tile = -1;

  if (board->state[move->to] & ChessPiecePawn)
    if (abs(move->to - move->from) == 16) // Pawn has double moved
      board->en_passant_tile = move->to + (isWhite ? 8 : -8);
}

// Checks that we're not doing a self capture.
static bool is_self_capture(Board board, int origin_pos, int target_pos)
{
  if (board.state[target_pos] == ChessPieceNone)
    return false;
  return !((board.state[origin_pos] & ChessPieceIsWhite) ^
           (board.state[target_pos] & ChessPieceIsWhite));
}

static int find_king(Board board, bool isWhite)
{
  for (int i = 0; i < 64; i++)
  {
    if ((board.state[i] & ChessPieceKing) == 0)
      continue;
    if (isWhite && (board.state[i] & ChessPieceIsWhite) != 0)
      return i;
    if (!isWhite && (board.state[i] & ChessPieceIsWhite) == 0)
      return i;
  }
  return -1;
}

static int typed_pos_of_checker(Board board, int king_pos,
                                ChessPiece attacker_type)
{
  int rv = -1;
  bool is_white = board.state[king_pos] & ChessPieceIsWhite;
  attacker_type &= ~ChessPieceIsWhite;

  // We want the colour of the king but the type of the attacker
  board.state[king_pos] =
      attacker_type | board.state[king_pos] & ChessPieceIsWhite;

  Array moves = board_get_moves(board, king_pos, 0);
  for (int i = 0; i < moves.capacity; i++)
  {
    if (!array_index_is_allocated(&moves, i))
      continue;
    int frompos = (*(Move*)array_get(&moves, i)).to;
    if (board.state[frompos] & (attacker_type))
    {
      if (is_white && (board.state[frompos] & ChessPieceIsWhite) == 0)
      {
        rv = frompos;
        goto end;
      }
      if (!is_white && (board.state[frompos] & ChessPieceIsWhite) != 0)
      {
        rv = frompos;
        goto end;
      }
    }
  }
end:
  array_free(&moves);
  return rv;
}

/// @return position of piece that is checking the king or -1 if the king is not
///         in check
static int position_of_checker(Board board, bool isWhite)
{
  int frompos;
  int king_pos = find_king(board, isWhite);

  frompos = typed_pos_of_checker(board, king_pos, ChessPieceQueen);
  if (frompos >= 0)
    return frompos;
  frompos = typed_pos_of_checker(board, king_pos, ChessPieceCastle);
  if (frompos >= 0)
    return frompos;
  frompos = typed_pos_of_checker(board, king_pos, ChessPieceBishop);
  if (frompos >= 0)
    return frompos;
  frompos = typed_pos_of_checker(board, king_pos, ChessPieceKnight);
  if (frompos >= 0)
    return frompos;
  frompos = typed_pos_of_checker(board, king_pos, ChessPiecePawn);
  if (frompos >= 0)
    return frompos;
  frompos = typed_pos_of_checker(board, king_pos, ChessPieceKing);
  if (frompos >= 0)
    return frompos;

  return frompos;
}

CheckInfo get_check_info(Board board, bool isWhite)
{
  CheckInfo result = CheckInfoNone;
  Array moves = board_get_moves_all(board, isWhite ? GetMovesWhite : GetMovesBlack);
  bool can_move = moves.used != 0;
  bool in_check = is_in_check(board, isWhite);

  if (in_check)
    result = CheckInfoCheck;

  if (!can_move && in_check)
    result = CheckInfoCheckmate;

  if (!can_move && !in_check)
    result = CheckInfoStalemate;

  array_free(&moves);
  return result;
}

bool is_in_check(Board board, bool isWhite)
{
  return position_of_checker(board, isWhite) >= 0;
}

bool can_move(Board board, bool is_white)
{
  Array moves = board_get_moves_all(board, is_white ? GetMovesWhite : GetMovesBlack);
  return array_free(&moves), moves.used != 0;
}
bool is_in_checkmate(Board board, bool is_white)
{
  if (!is_in_check(board, is_white))
    return false;
  return can_move(board, is_white);
}
bool is_in_stalemate(Board board, bool is_white)
{
  if (is_in_check(board, is_white))
    return false;
  return can_move(board, is_white);
}

/// @return Array array of moves
Array board_get_moves(Board _board, int pos, GetMovesFlags flags)
{
  Array moves;
  array_new(&moves, 32, sizeof(Move));

  u8 pos_88 = topos88(pos);

  if (pos_88 & 0x88)
    goto end;

  ChessPiece* board = _board.state;
  bool isWhite = board[pos] & ChessPieceIsWhite;

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
      if (!is_self_capture(_board, pos, topos64(tpos_88)))
      {
        Move move = move_new(pos, topos64(tpos_88));
        array_push(&moves, &move);
      }
    }
  }

  if (board[pos] & ChessPiecePawn)
  {
    bool isWhite = board[pos] & ChessPieceIsWhite;
    int dirsgn = isWhite ? -1 : 1; // Pawns can only move forwards

    // Pawns can double move at the start
    int double_move_rank = isWhite ? 6 : 1;
    if (torank64(pos) == double_move_rank &&
        board[pos + dirsgn * 16] == ChessPieceNone &&
        board[pos + dirsgn * 8] == ChessPieceNone)
    {
      Move move = move_new(pos, pos + dirsgn * 16);
      array_push(&moves, &move);
    }

    int tpos_88 = pos_88 + dirsgn * 16;

    ChessPiece pieces[] = {ChessPieceCastle, ChessPieceKnight, ChessPieceBishop,
                           ChessPieceQueen};

    // Pawns can only move forwards to an empty square
    if (!(tpos_88 & 0x88) && board[topos64(tpos_88)] == ChessPieceNone)
    {
      // @@Rework make this only need one promotion if block
      if (torank88(tpos_88) == (isWhite ? 0 : 7))
      {
        for (int i = 0; i < 4; i++)
        {
          Move promotion = move_new(pos, topos64(tpos_88));
          promotion.promotion = pieces[i]; // Handle this in board update
          array_push(&moves, &promotion);
        }
      }
      else
      {
        Move move = move_new(pos, topos64(tpos_88));
        array_push(&moves, &move);
      }
    }

    int diagonals[] = {15, 17};
    for (int i = 0; i < 2; i++)
    {
      int tpos_88 = pos_88 + dirsgn * diagonals[i];
      int tpos = topos64(tpos_88);
      if (tpos_88 & 0x88)
        continue;

      // Pawns can only move diagonally if they're capturing a piece or taking
      // en_passant_tile
      bool can_capture = !is_self_capture(_board, pos, tpos) && board[tpos];
      can_capture = can_capture || tpos == _board.en_passant_tile;

      if (can_capture)
      {
        if (torank88(tpos_88) == (isWhite ? 0 : 7))
        {
          for (int i = 0; i < 4; i++)
          {
            Move promotion = move_new(pos, topos64(tpos_88));
            promotion.promotion = pieces[i]; // Handle this in board update
            array_push(&moves, &promotion);
          }
        }
        else
        {
          Move move = move_new(pos, tpos);
          array_push(&moves, &move);
        }
      }
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
             !is_self_capture(_board, pos, topos64(tpos_88)))
      {
        Move move = move_new(pos, topos64(tpos_88));
        array_push(&moves, &move);

        if (board[topos64(tpos_88)])
          break;

        tpos_88 += slide_offset[i];
        if (board[pos] & ChessPieceKing)
          break;
      }
    }
  }

  // Castling
  if (board[pos] & ChessPieceKing)
  {
    for (int castling_ks = 0; castling_ks < 2; castling_ks++)
    {
      if (!(castling_ks ? _board.can_castle_ks[isWhite]
                        : _board.can_castle_qs[isWhite]))
        continue;

      // There needs to actually be a castle in the corner square
      if (!(board[topos64fr(castling_ks ? 7 : 0, isWhite ? 7 : 0)] &
            ChessPieceCastle))
        break;

      // Can't castle if we're in check
      if (flags & ConsiderChecks)
        if (position_of_checker(_board, isWhite) >= 0)
          break;

      bool can_castle = true;
      Board board_cpy = _board;
      int sign = castling_ks ? 1 : -1;
      for (int i = 1; i < (castling_ks ? 3 : 4); i++)
      {
        int tpos = pos + sign * i;
        if (board[tpos] != ChessPieceNone)
          can_castle = false;

        // If moving through this square would put the king in check, we can't
        // castle
        Move tmp = move_new(pos, tpos);
        board_update(&board_cpy, &tmp);
        if (flags & ConsiderChecks)
          if (position_of_checker(board_cpy, isWhite) >= 0)
            can_castle = false;
        board_cpy = _board;
      }

      if (can_castle)
      {
        Move move = move_new(pos, pos + sign * 2);
        array_push(&moves, &move);
      }
    }
  }

  // If making any of these moves would put our king in check then we cant make
  // them
  // we take considerChecks as an argument to this function because a pinned
  // piece can still give check: i.e. a piece that is pinned against the king
  // can still move to kill the enemy king even if doing so leaves its own
  // king in check.
  if (flags & ConsiderChecks)
  {
    Board board_after = _board;
    for (int i = 0; i < moves.capacity; i++)
    {
      if (!array_index_is_allocated(&moves, i))
        continue;
      board_update(&board_after, array_get(&moves, i));
      if ((position_of_checker(board_after, board[pos] & ChessPieceIsWhite)) >=
          0)
        array_remove(&moves, i);
      board_after = _board;
    }
  }

end:
  array_squash(&moves);
  return moves;
}

Array board_get_moves_all(Board board, GetMovesAllFlags flags)
{
  Array moves;
  array_new(&moves, 64, sizeof(Move));

  for (int i = 0; i < 64; i++)
  {
    if ((flags & GetMovesWhite) && !(flags & GetMovesBlack))
      if (!(board.state[i] & ChessPieceIsWhite))
        continue;
    if ((flags & GetMovesBlack) && !(flags & GetMovesWhite))
      if (board.state[i] & ChessPieceIsWhite)
        continue;

    size_t nmoves = 0;
    Array piece_moves = board_get_moves(board, i, ConsiderChecks);
    for (size_t j = 0; j < piece_moves.capacity; j++)
    {
      if (!array_index_is_allocated(&piece_moves, j))
        continue;
      array_push(&moves, array_get(&piece_moves, j));
    }
    array_free(&piece_moves);
  }
  array_squash(&moves);
  return moves;
}
