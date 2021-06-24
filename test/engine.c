#include <check.h>
#include <chess/board.h>

START_TEST(test_board_update)
{
}
END_TEST

int main(int argc, char** argv)
{
  Suite* s1 = suite_create("Engine");
  TCase* tc1_1 = tcase_create("Engine");
  SRunner* sr = srunner_create(s1);
  int num_failed;

  tcase_add_test(tc1_1, test_board_update);
  suite_add_tcase(s1, tc1_1);

  srunner_run_all(sr, CK_ENV);
  num_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return num_failed == 0 ? 0 : 1;
}
