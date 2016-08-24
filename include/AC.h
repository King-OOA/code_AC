#pragma once

#include <stdio.h>
#include <stdint.h>
#include "adt.h"
#include "patset.h"

#define ALPHABET_SIZE 256
#define SMALL_TAB_SIZE 20


#define Get_Next_State(state, ch) (state)->get_next_state((state), (ch))


/*状态结构*/
typedef struct State {
    int16_t outdegree;
    void *go_to;   /*可能挂大小表*/
    struct State *fail; /*出错时需要跳转的状态*/
    List_T output; /*连接该状态的输出keys*/
    struct State *(*get_next_state)(struct State *, char); /*匹配函数*/
} *State_T;

/*AC自动机*/
typedef struct DFA {
    State_T root; /*指向0状态*/
    int64_t size; /*AC自动机包含的状态总数*/
    int64_t small_tab_num;
    int64_t big_tab_num;
} *DFA_T; 

void matching(DFA_T DFA, FILE *text_fp, bool print_result);
DFA_T build_DFA(Patset_T patset);
