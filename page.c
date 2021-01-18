#include <stdlib.h>

#include "page.h"

void insert_page(page_t * head, int number) {
    page_t *current = head;
    page_t * temp_node = NULL;

    while (current->next != NULL) {
        if(current->next->number > number){
            break;
        }

        current = current->next;
    }

    temp_node = (page_t *) malloc(sizeof(page_t));
    temp_node->number = number;
    temp_node->next = current->next;
    current->next = temp_node;
}