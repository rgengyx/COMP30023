#include <stdlib.h>

#include "linkedlist.h"

node_t* push(node_t * head, Process *process) {
    node_t * current = head;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = (node_t *) malloc(sizeof(node_t));
    current->process = process;
    current->next->next = NULL;

    return head;

}


node_t* remove_by_process(node_t *head, Process *process) {
    node_t * current = head;
    node_t * temp_node = NULL;

    while (current->next != NULL) {
        if(current->process->id == process->id){
            break;
        }
        current = current->next;
    }

    if (current == head){
        temp_node = head;
        head = temp_node->next;

    }else{
        temp_node = current->next;
        current->next = temp_node->next;
    }

//    free(temp_node);

    return head;

}

node_t* push_and_order(node_t * head, Process *process) {
    node_t * current = head;
    node_t * temp_node = NULL;

    if(current->next == NULL){
        current->next = (node_t *) malloc(sizeof(node_t));
        current->process = process;
        current->next->next = NULL;

        return head;
    }


    while (current->next->next != NULL) {
        if(current->next->process->remaining_time > process->remaining_time){
            break;
        }

        current = current->next;
    }

    temp_node = (node_t *) malloc(sizeof(node_t));
    temp_node->process = process;
    temp_node->next = current->next;
    current->next = temp_node;

    return head;
}
