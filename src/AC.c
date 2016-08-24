#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include "mem.h"
#include "common.h"
#include "patset.h"
#include "adt.h"

#include "AC.h"

 /*goto函数链表上的节点*/
typedef struct Edge {
    State_T state;
    char ch;
} *Edge_T;

extern int64_t outdegree_num[];

static int32_t compare_edge(void const *e1, void const *e2)
{
  return (int32_t) ((Edge_T) e1)->ch - ((Edge_T) e2)->ch;
}

/* 将每个模式串插入到DFA中 */
static void insert_pat(void **pat_p, void *DFA)
{
    char *ch_p = (char *) (*pat_p);
    
    struct Edge tmp_edge, *edge;
    
    /*字符对应的状态已经存在*/
    State_T cur_state;
    for (cur_state = ((DFA_T) DFA)->root;
	 (tmp_edge.ch = *ch_p)
	  && (edge = list_search_order(cur_state->go_to, &tmp_edge, compare_edge));
	 cur_state = edge->state, ch_p++)
      ;
	
    /*为后面字符新建状态*/
    while (*ch_p) { 
        State_T new_state;
	NEW0(new_state);
	new_state->go_to = list_new(NULL);
	new_state->output = list_new(NULL);
	/*将新状态插入状态机中*/
        Edge_T edge;
	NEW(edge);
	edge->ch = *ch_p++;
	edge->state = new_state;
	list_push_order(cur_state->go_to, edge, compare_edge);
        cur_state = new_state;
    
        ((DFA_T) DFA)->size++;
    }
    
    /*在状态cur_state的output中插入pattern*/
    list_push_front(cur_state->output, *pat_p);
}

static void copy_to_big_tab(void **edge_p, void *tab_p)
{
    unsigned char ch = ((Edge_T) (*edge_p))->ch;
    ((State_T *) tab_p)[ch] = ((Edge_T) (*edge_p))->state;
    FREE(*edge_p); /*销毁边 */
}

static void build_big_tab(void **goto_p)
{
    List_T list = *goto_p;
    /* 创建大表(256个指向状态的指针) */
    *goto_p = CALLOC(ALPHABET_SIZE, sizeof(State_T));
    /* 把链表中的每一条边,都拷入大表对应项,并销毁所有边 */
    list_traverse(list, copy_to_big_tab, *goto_p);
    list_free(&list);
}

static void copy_to_small_tab(void **edge_p, void *tab_p)
{
    (*((Edge_T*) tab_p))->state = ((Edge_T) (*edge_p))->state;
    (*((Edge_T*) tab_p))->ch = ((Edge_T) (*edge_p))->ch;
    (*((Edge_T*) tab_p))++;
    FREE(*edge_p); /* 销毁边 */
}

static void build_small_tab(void **goto_p)
{
    List_T list = *goto_p;
    *goto_p = MALLOC(list_size(list) * sizeof(struct Edge));
    Edge_T tab_p = *goto_p;
    /* 把链表的每一条边拷贝到小表,并销毁所有边 */
    list_traverse(list, copy_to_small_tab, &tab_p);
    list_free(&list);
}

static void push_queue(void **edge_p, void *state_q)
{
    queue_push(state_q, ((Edge_T)(*edge_p))->state);
}

static State_T small_tab_search(State_T cur_state, char ch)
{
    Edge_T tab = (Edge_T) cur_state->go_to;
    int16_t outdegree = cur_state->outdegree;
    int16_t i;
    extern uint64_t small_tab_search_num;
    small_tab_search_num++;
    
    for (i = 0; i < outdegree && tab[i].ch < ch; i++)
	;
    
    if (i == outdegree || tab[i].ch > ch)
	return NULL;
    
    return tab[i].state;
}

static State_T small_tab_bin_search(State_T cur_state, char ch)
{
    Edge_T tab = (Edge_T) cur_state->go_to;
    int16_t low = 0, high = cur_state->outdegree - 1;
    extern uint64_t small_tab_bin_search_num;
    small_tab_bin_search_num++;
    
    while (low <= high) {
	int16_t mid = (low + high) >> 1;
	
	if (ch < tab[mid].ch)
	    high = mid - 1;
	else if (ch > tab[mid].ch)
	    low = mid + 1;
	else
	    return tab[mid].state;
    }

    return NULL;
}

static State_T big_tab_search(State_T cur_state, char ch)
{
    extern uint64_t big_tab_search_num;
    big_tab_search_num++;
    return ((State_T *)(cur_state->go_to))[(unsigned char) ch];
}

static State_T return_null(State_T cur_state, char ch)
{
    return NULL;
}

/*构建goto函数*/
static void build_goto(DFA_T DFA, Patset_T patset) 
{
    /* 创建根节点 */
    NEW0(DFA->root);
    State_T root = DFA->root;
    root->go_to = list_new(NULL);
    root->output = list_new(NULL); 
    DFA->size++;
    
    /*将模式串依次插入到DFA中,此时每个节点的出度结构,都是链表*/
    list_traverse(patset->pat_list, insert_pat, DFA);

    Queue_T state_q = queue_new();

    /* 单独处理根节点,为其建立大表 */
    build_big_tab(&root->go_to);
    root->get_next_state = big_tab_search;
    DFA->big_tab_num++;
    State_T *root_tab = (State_T *) root->go_to;
    for (int16_t i = 0; i < ALPHABET_SIZE; i++)
	if (root_tab[i] == NULL)
	    root_tab[i] = root;

    /* 将深度为1的节点放入队列 */
    for (int16_t i = 0; i < ALPHABET_SIZE; i++)
	if (root_tab[i] != root)
	    queue_push(state_q, root_tab[i]);
    
    /* 将每个节点的链表转化为大小访问表 */
    while (!queue_empty(state_q)) {
	State_T state = queue_pop(state_q);
	state->outdegree = list_size(state->go_to);
	outdegree_num[state->outdegree]++;
	if (state->outdegree) {	/* 内部节点 */
	    /* 将state的后续节点压入队列 */
	    list_traverse(state->go_to, push_queue, state_q);
	    if (state->outdegree > SMALL_TAB_SIZE) { /* 创建大表,同时销毁链表 */
		build_big_tab(&state->go_to);
		state->get_next_state = big_tab_search;
		DFA->big_tab_num++;
	    } else { /* 创建小表,同时销毁链表 */
		build_small_tab(&state->go_to);
		state->get_next_state =
		    (state->outdegree > 5) ? small_tab_bin_search : small_tab_search;
		DFA->small_tab_num++;
	    }
	} else { /* 叶节点 */
	    list_free((List_T *) &state->go_to);
	    state->get_next_state = return_null;
	}
    }
    
    queue_free(&state_q);
}

static void build_fail(DFA_T DFA)
{
    Queue_T state_q = queue_new(); /*状态队列*/
    State_T state; /*深度为d的状态*/
    State_T next_state; /*state的孩子，深度为d+1的状态*/
    State_T fail; /*state的fail值*/
    State_T fail_next; /*fial的孩子*/

    /*处理深度为1的状态*/
    for (int16_t ch = 0; ch < ALPHABET_SIZE; ch++) /*扫描根节点的大表*/
	if ((state = ((State_T *) DFA->root->go_to)[ch]) != DFA->root) {
	    state->fail = DFA->root; /*其错误状态均为根节点*/
	    queue_push(state_q, state);
	}

    /*广度优先遍历状态机*/
    while (!queue_empty(state_q)) {
	state = queue_pop(state_q); /*返回深度为d的状态*/
	fail = state->fail;
	for (int16_t ch = 0; ch < ALPHABET_SIZE; ch++) /*寻找state的所有下一个有效状态*/
	    if ((next_state = Get_Next_State(state, ch))) {
		queue_push(state_q, next_state);
		/*一直回溯直到找到合法的fail状态*/
		while ((fail_next = Get_Next_State(fail, ch)) == NULL) 
		    fail = fail->fail;
		next_state->fail = fail_next;
		/*并入fail处的output值*/
		list_merge(next_state->output, next_state->fail->output);
	    }
    }

    queue_free(&state_q); /*销毁队列*/
}

DFA_T build_DFA(Patset_T patset)
{
    clock_t start, end;
    
    /* 初始化DFA */
    DFA_T DFA;
    NEW0(DFA);

   /*构建goto表*/
    printf("building goto.....  ");
    fflush(stdout);
    start = clock();
    build_goto(DFA, patset);
    end = clock();
    printf("Done!\n%f\n", (double) (end - start) / CLOCKS_PER_SEC);

    /*构建fail表*/
    printf("buliding fail.....  ");
    fflush(stdout);
    start = clock();
    build_fail(DFA);
    end = clock();
    printf("Done!\n%f\n", (double) (end - start) / CLOCKS_PER_SEC);

    return DFA;
}

static void print_pat(void **xp, void *pos_p)
{
    printf("%ld: %s", *((int64_t *)pos_p) - strlen(*xp), (char *)(*xp));
}

void matching(DFA_T DFA, FILE *text_fp, bool print_result)
{
    State_T state = DFA->root;
    State_T next_state;
    int64_t pos = 0;
    int16_t ch;
    extern uint64_t total_matched_num;
    extern uint64_t trace_back_num;
    
    while ((ch = getc(text_fp)) != EOF) {
	pos++;
	while ((next_state = Get_Next_State(state,ch)) == NULL) {
	    state = state->fail;
	    trace_back_num++;
	}
	state = next_state;
	
	if(!list_empty(state->output)) {
	    total_matched_num += list_size(state->output);
	    if (print_result) {
		list_traverse(state->output, print_pat, &pos);
		putchar('\n');
	  }
	}
    }
}

#if (NOFAIL == 1)
static void remove_fail(void)
{
    queue_t *state_q = cre_queue();
    State_T next_state, *state;
    int ch;

    for (ch = 0; ch < ALPHABET_SIZE; ch++) 
	if ((state = get_next_state(state_0, ch)) != state_0)
	    in_queue(state_q, state);
	
    while (!queue_is_empty(state_q)) {
	state = out_queue(state_q);
	for (ch = 0; ch < ALPHABET_SIZE; ch++)
	    if (next_state = get_next_state(state, ch))
		in_queue(state_q, next_state);
	    else
		link_states(state, ch, get_next_state(state->fail, ch));
    }

    des_queue(state_q);
}
#endif 
