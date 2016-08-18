#pragma once

#include <stdio.h>
#include <stdint.h>
#include "make_patset.h"

#define ALPHABET_SIZE 256
#define NOFAIL 0
#define MAX_LIST_LEN 20         /* goto 链表所允许的最大长度 */
#define MATCHING 0
#define SHOW_IN_FILE 0          /* 默认在屏幕上显示结果 */
#define RESULTS_FILE "/home/pz/projects/AC/results"

#define DATA_PATH  "/home/pz/data/"
#define TEXT_FILE DATA_PATH"English/500MB"
#define PATS_FILE DATA_PATH"patterns/size/100w"

typedef enum {TABLE,LIST} Goto_T; /* goto 函数实现方法 */

/*每个状态上output链表的节点*/
typedef struct Output_Node {
    patnode_t *pat; /*指向模式集中的模式*/
    struct output_node *next;
} O_Node_T;

/*状态结构*/
typedef struct State {
    void *go_to;   /*可能挂数组或有序链表*/
    struct State *fail; /*出错时需要跳转的状态*/
    O_Node_T *output; /*连接该状态的输出keys*/
    Goto_T g_type; /*标明goto函数是数组还是链表*/
} State_T;

typedef struct Goto_Node { /*goto函数链表上的节点*/
    char input_char;
    State_T *next_state;
    struct Goto_Node *next;
} G_Node_T;

/*AC自动机*/
struct Automata {
    State_T *root; /*指向0状态*/
    int64_t state_num; /*AC自动机包含的状态总数*/
    int64_t tab_num;
    int64_t list_num;
}; 

void AC(FILE *text, patset_t *pat);
