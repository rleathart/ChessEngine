#include <check.h>
#include <chess/board.h>
#include <chess/util.h>

START_TEST(test_pawn_moves)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "p 0 0 0 0 0 0 0"
                                "0 P 0 0 0 0 0 P"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");

  // Pawn should be able to move 1 or 2 squares forward and capture enemy pawn.
  // It should not wrap around the board to capture the pawn on the other side
  board_get_moves(board, topos64(0x10), &moves, &nmoves, 0);

  int expected_nmoves = 3;
  ck_assert_int_eq(nmoves, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (moves[i].from == topos64(0x10) && moves[i].to == topos64(0x20))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x10) && moves[i].to == topos64(0x30))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x10) && moves[i].to == topos64(0x21))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);

  // @@Implement explicitly check that pawns cant capture a piece directly in
  // front of them

  // @@Implement explicitly check that pawns can only move one square after they
  // leave their starting rank
}
END_TEST

START_TEST(test_knight_moves)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "n 0 0 0 0 0 0 0"
                                "0 0 P 0 0 0 P 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");

  board_get_moves(board, topos64(0x10), &moves, &nmoves, 0);

  int expected_nmoves = 3;
  ck_assert_int_eq(nmoves, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (moves[i].from == topos64(0x10) && moves[i].to == topos64(0x02))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x10) && moves[i].to == topos64(0x22))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x10) && moves[i].to == topos64(0x31))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);

  // @@Implement check knights moves in the middle of the board (with jumping
  // over pieces)
}
END_TEST

START_TEST(test_castle_moves)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "P r 0 0 0 0 0 0"
                                "P 0 0 0 0 0 0 0"
                                "0 p 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 P 0 0 0 0 0 0");

  board_get_moves(board, topos64(0x11), &moves, &nmoves, 0);

  int expected_nmoves = 9;
  ck_assert_int_eq(nmoves, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x01))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x10))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x12))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x13))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x14))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x15))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x16))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x17))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x21))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);

  // @@Implement Castling
}
END_TEST

START_TEST(test_bishop_moves)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "0 b 0 0 0 0 0 0"
                                "P 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 p 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0");

  board_get_moves(board, topos64(0x11), &moves, &nmoves, 0);

  int expected_nmoves = 5;
  ck_assert_int_eq(nmoves, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x00))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x02))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x20))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x22))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x33))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);
}
END_TEST

START_TEST(test_queen_moves)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "0 0 0 0 0 0 0 0"
                                "0 q 0 0 0 p 0 P"
                                "P 0 0 0 0 0 0 0"
                                "0 p 0 0 0 0 0 0"
                                "0 0 0 0 p 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 P 0 0 0 0 0 0");

  board_get_moves(board, topos64(0x11), &moves, &nmoves, 0);

  int expected_nmoves = 11;
  ck_assert_int_eq(nmoves, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x00))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x01))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x02))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x10))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x12))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x13))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x14))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x20))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x21))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x22))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x11) && moves[i].to == topos64(0x33))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);
}
END_TEST

START_TEST(test_king_moves)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  // First just do a simple test for the king moving 1 square
  board_new_from_string(&board, "0 0 k 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 P 0 0 0 0 0 0");

  board_get_moves(board, topos64(0x02), &moves, &nmoves, 0);

  int expected_nmoves = 5;
  ck_assert_int_eq(nmoves, expected_nmoves);

  bool found_moves[expected_nmoves];
  for (int i = 0; i < expected_nmoves; i++)
    found_moves[i] = false;

  int idx = 0;
  for (int i = 0; i < nmoves; i++)
  {
    if (moves[i].from == topos64(0x02) && moves[i].to == topos64(0x01))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x02) && moves[i].to == topos64(0x03))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x02) && moves[i].to == topos64(0x11))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x02) && moves[i].to == topos64(0x12))
      found_moves[idx++] = true;
    if (moves[i].from == topos64(0x02) && moves[i].to == topos64(0x13))
      found_moves[idx++] = true;
  }

  for (int i = 0; i < expected_nmoves; i++)
    fail_if(!found_moves[i]);

  // @@Implement test for check condition
}
END_TEST

// Pinned pieces should still be able to check the enemy king
START_TEST(test_pinned_check)
{
  Board board;
  Move* moves;
  size_t nmoves = 0;

  board_new_from_string(&board, "k 0 0 0 0 0 0 0"
                                "0 r 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 B 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 0 0 0 0 0 0 0"
                                "0 K 0 0 0 0 0 0");

  board_get_moves(board, topos64(0x44), &moves, &nmoves, ConsiderChecks);

  int expected_nmoves = 1;
  ck_assert_int_eq(nmoves, expected_nmoves);
  expected_nmoves = 4;
  board_get_moves(board, topos64(0x71), &moves, &nmoves, ConsiderChecks);
  ck_assert_int_eq(nmoves, expected_nmoves);
}
END_TEST

int main(int argc, char** argv)
{
  Suite* s1 = suite_create("Engine");
  TCase* tc1_1 = tcase_create("Engine");
  SRunner* sr = srunner_create(s1);
  int num_failed;

  tcase_add_test(tc1_1, test_pawn_moves);
  tcase_add_test(tc1_1, test_knight_moves);
  tcase_add_test(tc1_1, test_bishop_moves);
  tcase_add_test(tc1_1, test_castle_moves);
  tcase_add_test(tc1_1, test_queen_moves);
  tcase_add_test(tc1_1, test_king_moves);
  tcase_add_test(tc1_1, test_pinned_check);
  suite_add_tcase(s1, tc1_1);

  srunner_run_all(sr, CK_ENV);
  num_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return num_failed == 0 ? 0 : 1;
}
