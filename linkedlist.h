#include "process.h"

typedef struct node {
    Process *process;
    struct node* next;
} node_t;

/* function declaration */
node_t* push(node_t * head, Process *process);

node_t* remove_by_process(node_t *head, Process *process);

node_t* push_and_order(node_t * head, Process *process);