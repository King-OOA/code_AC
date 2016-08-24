#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"
#include "patset.h"
#include "AC.h"

/* 总共匹配次数 */
uint64_t total_matched_num;
uint64_t trace_back_num;
uint64_t big_tab_search_num;
uint64_t small_tab_search_num;
uint64_t small_tab_bin_search_num;

/*每个出度所包含节点的数量*/
int64_t outdegree_num[ALPHABET_SIZE+1] = {0};

int main(int argc, char *argv[])
{    
  /* 创建模式集 */
  Patset_T patset = patset_new(argv[1]);
  /* 构建AC自动机 */
  DFA_T DFA = build_DFA(patset);
    
  /* 匹配 */
  FILE *text_fp = efopen(argv[2], "r");
  printf("matching....  ");
  fflush(stdout);
  clock_t start = clock();
  matching(DFA, text_fp, 0);
  clock_t end = clock();
  printf("Done!\n%f\n", (double) (end - start) / CLOCKS_PER_SEC);
  printf("\nTotal matched num: %ld\n", total_matched_num);

  printf("\nSmall table: %ld, Big Table: %ld\n",
	 DFA->small_tab_num, DFA->big_tab_num);
  /* 函数调用次数 */
  printf("Big tab search num: %ld\n", big_tab_search_num);
  printf("Small tab search num: %ld\n", small_tab_search_num);
  printf("Small tab bin search num: %ld\n", small_tab_bin_search_num);
  printf("Trace back num: %ld\n", trace_back_num);

 /* /\* 输出统计信息 *\/ */
 /*  for (int16_t i = 0; i <= ALPHABET_SIZE; i++) */
 /*    if (outdegree_num[i]) */
 /*      printf("\noutdegree: %2d num: %4ld", i, outdegree_num[i]); */
 

  return 0;
}
