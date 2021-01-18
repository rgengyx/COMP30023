#include "page.h"

typedef struct P{
    int arrival_time;
    int id;
    int required_size;
    int running_time;
    int remaining_time;
    int load_time;
    int current_time;
    int jobtime;
    int current_running_time;
    char state[10];
    int proc_remaining;
    page_t *page_queue;
    int page_fault;
    int occupied_pages;

} Process;