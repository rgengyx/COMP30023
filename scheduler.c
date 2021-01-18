#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "linkedlist.h"

/* function declaration */
node_t *form_linkedlist(char *filename);

int get_num_processes(node_t *head);

void print_events(node_t *head, int memory, char *memory_management);

void print_statistics(node_t *head, node_t *h);

void scheduling(char *filename, int memory, char *scheduling_algorithm, char *memory_management, int quantum);


int main(int argc, char *argv[]) {

    int opt;
    char filename[100];
    char scheduling_algorithm[4];
    char memory_management[4];
    int quantum = 0;
    int memory = 0;
    while ((opt = getopt(argc, argv, "f:a:m:s:q:")) != -1) {
        switch (opt) {
            case 'f':
                strcpy(filename, optarg);
                break;
            case 'a':
                if (strcmp(optarg, "ff") == 0) {
                    strcpy(scheduling_algorithm, "ff");
                } else if (strcmp(optarg, "rr") == 0) {
                    strcpy(scheduling_algorithm, "rr");
                } else if (strcmp(optarg, "cs") == 0) {
                    strcpy(scheduling_algorithm, "cs");
                }
                break;
            case 'm':
                if (strcmp(optarg, "u") == 0) {
                    strcpy(memory_management, "u");
                } else if (strcmp(optarg, "p") == 0) {
                    strcpy(memory_management, "p");
                } else if (strcmp(optarg, "v") == 0) {
                    strcpy(memory_management, "v");
                }
                break;
            case 's':
                memory = atoi(optarg);
                break;
            case 'q':
                quantum = atoi(optarg);
                break;
        }
    }

    scheduling(filename, memory, scheduling_algorithm, memory_management, quantum);


    return 0;
}


node_t *form_linkedlist(char *filename) {

    node_t *head = (node_t *) malloc(sizeof(node_t));
    head->next = NULL;

    FILE *file = fopen(filename, "r");
    while (!feof(file)) {
        Process *p = (Process *) malloc(sizeof(Process));
        int v = fscanf(file, "%d %d %d %d", &p->arrival_time, &p->id, &p->required_size, &p->remaining_time);
        if (v != 4) {
            break;
        }
        p->jobtime = p->remaining_time;
        p->load_time = 0;
        head = push(head, p);
    }

    fclose(file);
    return head;
}

int get_num_processes(node_t *head) {

    node_t *current = head;
    int num_processes = 0;
    while (current->next != NULL) {
        current = current->next;
        num_processes++;
    }

    return num_processes;
}

int get_num_pages(page_t *head) {

    page_t *current = head;
    int num_pages = 0;
    while (current->next != NULL) {
        current = current->next;
        num_pages++;
    }
    return num_pages;
}


void scheduling(char *filename, int memory, char *scheduling_algorithm, char *memory_management, int quantum) {

    // Init Process Queue

    node_t *process_queue = form_linkedlist(filename);

    // Init Running Queue
    node_t *running_queue = (node_t *) malloc(sizeof(node_t));
    running_queue->next = NULL;

    // Init Memory Queue
    node_t *memory_queue = (node_t *) malloc(sizeof(node_t));
    memory_queue->next = NULL;

    // Init Memory Table
    int total_pages = memory / 4;
    Process *p;
    Process **memory_table = malloc(total_pages * sizeof(*p));
    for (int i = 0; i < total_pages; i++) {
        memory_table[i] = NULL;
    }

    // Init Event Queue
    node_t *event_queue = (node_t *) malloc(sizeof(node_t));
    event_queue->next = NULL;

    int time = 0;
    int num_finished = 0;
    int num_processes = get_num_processes(process_queue);
    while (1) {

        if (num_finished >= num_processes) {
            break;
        }

        // Loading Process Queue to Memory Queue
        node_t *process_queue_current = process_queue;
        while (process_queue_current->next != NULL) {
            Process *p = process_queue_current->process;
            if (p->arrival_time == time) {

                // Memory Managment first
                if (strcmp(memory_management, "u") == 0) {

                } else if (strcmp(memory_management, "p") == 0) {
                    int num_pages = p->required_size / 4;
                    int load_time = 2 * num_pages;
                    p->load_time = load_time;
                } else if (strcmp(memory_management, "v") == 0) {
                    int num_pages = p->required_size / 4;
                    int load_time = 2 * num_pages;
                    p->load_time = load_time;
                }

                if (strcmp(scheduling_algorithm, "ff") == 0) {
                    p->current_running_time = p->remaining_time + p->load_time;
                    running_queue = push(running_queue, p);

                } else if (strcmp(scheduling_algorithm, "rr") == 0) {
                    p->current_running_time = quantum + p->load_time;
                    running_queue = push(running_queue, p);

                } else if (strcmp(scheduling_algorithm, "cs") == 0) {
                    p->current_running_time = p->remaining_time + p->load_time;
                    running_queue = push_and_order(running_queue, p);

                }


                // Start first process

                if (memory_queue->next == NULL) {
                    page_t *page_queue = (page_t *) malloc(sizeof(page_t));
                    page_queue->next = NULL;

                    int num_pages_required = p->required_size / 4;
                    int occupied_pages = 0;

                    // Load 1st process to memory table and queue
                    for (int i = 0; i < total_pages; i++) {
                        if (memory_table[i] == NULL) {
                            memory_table[i] = p;
                            occupied_pages++;

                            insert_page(page_queue, i);

                            if (occupied_pages == num_pages_required) {
                                break;
                            }
                        }
                    }

                    memory_queue = push(memory_queue, p);
                    p->page_queue = page_queue;
                    // Event Queue [Running: After looping]
                    Process *temp = (Process *) malloc(sizeof(Process));
                    memcpy(temp, p, sizeof *p);
                    temp->current_time = time;
                    temp->page_queue = page_queue;
                    temp->occupied_pages = occupied_pages;
                    strcpy(temp->state, "RUNNING");
                    event_queue = push(event_queue, temp);

                }


            }
            process_queue_current = process_queue_current->next;
        }

        // Memory Management
        if (strcmp(memory_management, "u") == 0) {

        } else if (strcmp(memory_management, "p") == 0) {

            Process *first_p = running_queue->process;
            if (first_p == NULL) {
                time++;
                continue;
            }

            if (first_p->current_running_time == 0) {

                if (running_queue->next->next != NULL) {

                    // >= 2
                    // Evict first process first
                    // Last process in memory table, just evict it.
                    // Evict
                    if (first_p->remaining_time == 0) {

                        node_t *current = memory_queue;
                        while (current->next->next != NULL) {
                            if (current->process->id == first_p->id) {
                                break;
                            }
                            current = current->next;
                        }
                        Process *next_p = current->process;

                        page_t *page_queue = (page_t *) malloc(sizeof(page_t));
                        page_queue->next = NULL;


                        // Free memory table and queue
                        for (int i = 0; i < total_pages; i++) {
                            if (memory_table[i] == next_p) {
                                memory_table[i] = NULL;

                                insert_page(page_queue, i);

                            }
                        }
                        memory_queue = remove_by_process(memory_queue, next_p);

                        // Event Queue [Evict: before looping]
                        Process *temp = (Process *) malloc(sizeof(Process));
                        memcpy(temp, next_p, sizeof *next_p);
                        temp->current_time = time;
                        temp->page_queue = page_queue;
                        strcpy(temp->state, "EVICTED");
                        event_queue = push(event_queue, temp);

                    }

                    // Check if new process can load in
                    Process *next_first_p = running_queue->next->process;


                    //TODO: Check if next_first_p already exists in memory
                    // If yes, continue
                    // Else: try loading


                    int required = next_first_p->required_size / 4;

                    // Check if next_first_p already exists in memory
                    int num_pages_matched = 0;
                    // Check num of available pages
                    for (int i = 0; i < total_pages; i++) {

                        if (memory_table[i] == NULL) {
                            continue;
                        } else if (memory_table[i]->id == next_first_p->id) {
                            num_pages_matched++;
                        }
                    }
                    if (num_pages_matched != required) {

                        // Try loading
                        int num_pages_available = 0;
                        // Check num of available pages
                        for (int i = 0; i < total_pages; i++) {
                            if (memory_table[i] == NULL) {
                                num_pages_available++;
                            }
                        }

                        int num_pages_required = next_first_p->required_size / 4;
                        if (num_pages_required <= num_pages_available) {
                            // Enough pages, let's add the process to table
                            int occupied_pages = 0;
                            for (int i = 0; i < total_pages; i++) {
                                if (memory_table[i] == NULL) {
                                    memory_table[i] = next_first_p;
                                    occupied_pages++;
                                    if (occupied_pages == num_pages_required) {
                                        break;
                                    }
                                }
                            }
                            memory_queue = push(memory_queue, next_first_p);

                        } else {

                            // Not enough pages available, evicting
                            page_t *page_queue = (page_t *) malloc(sizeof(page_t));
                            page_queue->next = NULL;
                            node_t *memory_queue_current = memory_queue;
                            while (memory_queue_current->next != NULL) {

                                // Evict
                                Process *next_p = memory_queue_current->process;

                                // Evicting
                                for (int i = 0; i < total_pages; i++) {
                                    if (memory_table[i] == next_p) {
                                        memory_table[i] = NULL;

                                        insert_page(page_queue, i);

                                    }
                                }


                                int num_pages = next_p->required_size / 4;
                                int load_time = 2 * num_pages;
                                next_p->load_time += load_time;
                                if (next_p->id != first_p->id) {
                                    next_p->current_running_time += load_time;
                                }

                                memory_queue = remove_by_process(memory_queue, next_p);

                                // Re-check if there is enough pages
                                int num_pages_available = 0;
                                // Check num of available pages
                                for (int i = 0; i < total_pages; i++) {
                                    if (memory_table[i] == NULL) {
                                        num_pages_available++;
                                    }
                                }

                                int num_pages_required = first_p->required_size / 4;

                                if (num_pages_required <= num_pages_available) {
                                    // Enough pages, let's add the process to table
                                    int occupied_pages = 0;
                                    for (int i = 0; i < total_pages; i++) {
                                        if (memory_table[i] == NULL) {
                                            memory_table[i] = next_first_p;
                                            occupied_pages++;
                                            if (occupied_pages == num_pages_required) {
                                                break;
                                            }
                                        }
                                    }
                                    memory_queue = push(memory_queue, next_first_p);
                                    break;
                                }

                                memory_queue_current = memory_queue_current->next;
                            }

                            // Event Queue [Evict: before looping]
                            Process *temp = (Process *) malloc(sizeof(Process));
                            memcpy(temp, next_first_p, sizeof *next_first_p);
                            temp->current_time = time;
                            temp->page_queue = page_queue;
                            strcpy(temp->state, "EVICTED");
                            event_queue = push(event_queue, temp);
                        }
                    }

                } else if (first_p->remaining_time == 0) {

                    // Last process in memory table, just evict it.
                    // Evict
                    Process *next_p = memory_queue->process;

                    page_t *page_queue = (page_t *) malloc(sizeof(page_t));
                    page_queue->next = NULL;


                    for (int i = 0; i < total_pages; i++) {
                        if (memory_table[i] == next_p) {
                            memory_table[i] = NULL;

                            insert_page(page_queue, i);

                        }
                    }
                    memory_queue = remove_by_process(memory_queue, next_p);

                    // Event Queue [Evict: before looping]
                    Process *temp = (Process *) malloc(sizeof(Process));
                    memcpy(temp, next_p, sizeof *next_p);
                    temp->current_time = time;
                    temp->page_queue = page_queue;
                    strcpy(temp->state, "EVICTED");
                    event_queue = push(event_queue, temp);
                }

            }

        } else if (strcmp(memory_management, "v") == 0) {

            Process *first_p = running_queue->process;
            if (first_p == NULL) {
                time++;
                continue;
            }

            if (first_p->current_running_time == 0) {

                if (running_queue->next->next != NULL) {

                    // >= 2
                    // Evict first process first
                    // Last process in memory table, just evict it.
                    // Evict
                    if (first_p->remaining_time == 0) {


                        node_t *current = memory_queue;
                        while (current->next->next != NULL) {
                            if (current->process->id == first_p->id) {
                                break;
                            }
                            current = current->next;
                        }
                        Process *next_p = current->process;

                        page_t *page_queue = (page_t *) malloc(sizeof(page_t));
                        page_queue->next = NULL;

                        // Free memory table and queue
                        for (int i = 0; i < total_pages; i++) {
                            if (memory_table[i] == next_p) {
                                memory_table[i] = NULL;

                                insert_page(page_queue, i);

                            }
                        }
                        memory_queue = remove_by_process(memory_queue, next_p);

                        // Event Queue [Evict: before looping]
                        Process *temp = (Process *) malloc(sizeof(Process));
                        memcpy(temp, next_p, sizeof *next_p);
                        temp->current_time = time;
                        temp->page_queue = page_queue;
                        strcpy(temp->state, "EVICTED");
                        event_queue = push(event_queue, temp);

                    }

                    // Check if new process can load in
                    Process *next_first_p = running_queue->next->process;


                    //TODO: Check if next_first_p already exists in memory
                    // If yes, continue
                    // Else: try loading


                    int required = next_first_p->required_size / 4;
                    // Check if next_first_p already exists in memory
                    int exist = 0;
                    int num_existed = 0;
                    // Check num of available pages
                    for (int i = 0; i < total_pages; i++) {

                        if (memory_table[i] == NULL) {
                            continue;
                        } else if (memory_table[i]->id == next_first_p->id) {
                            exist = 1;
                            num_existed++;
                        }
                    }


                    if (num_existed != required) {
                        // Try loading
                        int num_pages_available = 0;
                        // Check num of available pages
                        for (int i = 0; i < total_pages; i++) {
                            if (memory_table[i] == NULL) {
                                num_pages_available++;
                            }
                        }

                        int num_pages_required = next_first_p->required_size / 4;

                        if (num_pages_available + num_existed >= 4) {

                            // Enough pages, let's add the process to table
                            page_t *page_queue = (page_t *) malloc(sizeof(page_t));
                            page_queue->next = NULL;

                            for (int i = 0; i < total_pages; i++) {
                                if (memory_table[i] == NULL) {
                                    continue;
                                } else if (memory_table[i]->id == next_first_p->id) {
                                    insert_page(page_queue, i);
                                }
                            }

                            int occupied_pages = 0;
                            for (int i = 0; i < total_pages; i++) {
                                if (memory_table[i] == NULL) {
                                    memory_table[i] = next_first_p;
                                    occupied_pages++;
                                    insert_page(page_queue, i);
                                    if (occupied_pages+num_existed == num_pages_required) {
                                        break;
                                    }
                                }
                            }
                            next_first_p->page_queue = page_queue;
                            memory_queue = push(memory_queue, next_first_p);

                            int num_pages = num_pages_required;
                            if (occupied_pages < num_pages_required) {
                                num_pages = occupied_pages;
                            }

                            int load_time = 2 * num_pages;
                            next_first_p->load_time = load_time;
                            next_first_p->page_fault = num_pages_required - occupied_pages - num_existed;
                            next_first_p->remaining_time += next_first_p->page_fault;

                        } else {
                            // Not enough pages available,  evicting
                            page_t *page_queue = (page_t *) malloc(sizeof(page_t));
                            page_queue->next = NULL;
                            node_t *memory_queue_current = memory_queue;
                            int total_evicted = 0;
                            while (memory_queue_current->next != NULL) {

                                // Evict
                                Process *next_p = memory_queue_current->process;

                                if (next_p->id == next_first_p->id) {
                                    memory_queue_current = memory_queue_current->next;
                                    continue;
                                }

                                // Evicting
                                int evicted_pages = 0;
                                for (int i = 0; i < total_pages; i++) {
                                    if (memory_table[i] == next_p) {
                                        memory_table[i] = NULL;
                                        insert_page(page_queue, i);

                                        total_evicted++;
                                        evicted_pages++;
                                        if (total_evicted + num_existed >= 4) {
                                            break;
                                        }
                                    }
                                }


                                if (next_p->id != first_p->id) {
//                                    next_p->current_running_time += load_time;
                                }

                                // Check if all pages of a process is evicted

                                if (evicted_pages == get_num_pages(next_p->page_queue)) {
                                    // All pages evicted, remove process
                                    memory_queue = remove_by_process(memory_queue, next_p);
                                } else {

                                    page_t *page_queue = (page_t *) malloc(sizeof(page_t));
                                    page_queue->next = NULL;
                                    for (int i = 0; i < total_pages; i++) {
                                        if (memory_table[i] == next_p) {
                                            insert_page(page_queue, i);
                                        }
                                    }

                                    next_p->page_queue = page_queue;
                                }

                                if (total_evicted + num_existed < 4) {
                                    memory_queue_current = memory_queue_current->next;
                                    continue;
                                }

                                // Re-check if there is enough pages
                                int num_pages_available = 0;
                                // Check num of available pages
                                for (int i = 0; i < total_pages; i++) {
                                    if (memory_table[i] == NULL) {
                                        num_pages_available++;
                                    }
                                }

                                int num_pages_required = first_p->required_size / 4;

                                if (num_pages_available > 0) {
                                    // Enough pages, let's add the process to table
                                    page_t *page_queue = (page_t *) malloc(sizeof(page_t));
                                    page_queue->next = NULL;
                                    int occupied_pages = 0;
                                    for (int i = 0; i < total_pages; i++) {
                                        if (memory_table[i] == NULL) {
                                            memory_table[i] = next_first_p;
                                            occupied_pages++;
                                            insert_page(page_queue, i);
                                            if (occupied_pages == total_evicted) {
                                                break;
                                            }
                                        }
                                    }
                                    next_first_p->page_queue = page_queue;
                                    memory_queue = push(memory_queue, next_first_p);

                                    break;
                                }

                                memory_queue_current = memory_queue_current->next;
                            }

                            int num_pages = num_pages_required;
                            if (total_evicted + num_existed < num_pages_required) {
                                num_pages = total_evicted;
                            }

                            int load_time = 2 * num_pages;
                            next_first_p->load_time = load_time;
                            next_first_p->page_fault = num_pages_required - total_evicted - num_existed;
                            next_first_p->remaining_time += next_first_p->page_fault;

                            // Event Queue [Evict: before looping]
                            Process *temp = (Process *) malloc(sizeof(Process));
                            memcpy(temp, next_first_p, sizeof *next_first_p);
                            temp->current_time = time;
                            temp->page_queue = page_queue;
                            strcpy(temp->state, "EVICTED");
                            event_queue = push(event_queue, temp);
                        }
                    }

                } else if (first_p->remaining_time == 0) {


                    // Last process in memory table, just evict it.
                    // Evict
                    Process *next_p = memory_queue->process;

                    page_t *page_queue = (page_t *) malloc(sizeof(page_t));
                    page_queue->next = NULL;

                    for (int i = 0; i < total_pages; i++) {
                        if (memory_table[i] == next_p) {
                            memory_table[i] = NULL;
                            insert_page(page_queue, i);

                        }
                    }
                    memory_queue = remove_by_process(memory_queue, next_p);

                    // Event Queue [Evict: before looping]
                    Process *temp = (Process *) malloc(sizeof(Process));
                    memcpy(temp, next_p, sizeof *next_p);
                    temp->current_time = time;
                    temp->page_queue = page_queue;

                    strcpy(temp->state, "EVICTED");
                    event_queue = push(event_queue, temp);

                }

            }
        }

        // Scheduling
        if (strcmp(scheduling_algorithm, "ff") == 0 || strcmp(scheduling_algorithm, "cs") == 0) {

            Process *first_p = running_queue->process;
            if (first_p->current_running_time > 0) {

                if (first_p->load_time > 0) {
                    first_p->load_time--;
                } else if (first_p->load_time == 0) {
                    first_p->remaining_time--;
                }
                first_p->current_running_time--;
            } else if (first_p->remaining_time == 0) {

                //TODO: Finishes Process

                running_queue = remove_by_process(running_queue, first_p);
                num_finished++;

                int proc_remaining = get_num_processes(running_queue);

                // Event Queue

                Process *temp = (Process *) malloc(sizeof(Process));
                memcpy(temp, first_p, sizeof *first_p);
                temp->current_time = time;
                strcpy(temp->state, "FINISHED");
                temp->proc_remaining = proc_remaining;

                event_queue = push(event_queue, temp);

                if (proc_remaining > 0) {


                    Process *next_p = running_queue->process;

                    page_t *page_queue = (page_t *) malloc(sizeof(page_t));
                    page_queue->next = NULL;

                    // Load 1st process to memory table and queue
                    int unoccupied_pages = 0;
                    for (int i = 0; i < total_pages; i++) {
                        if (memory_table[i] == NULL) {
                            unoccupied_pages++;
                            continue;
                        } else if (memory_table[i]->id == next_p->id) {
                            insert_page(page_queue, i);
                        }
                    }

                    // Event Queue [Running: After looping]

                    Process *temp = (Process *) malloc(sizeof(Process));
                    memcpy(temp, next_p, sizeof *next_p);
                    temp->current_time = time;
                    temp->page_queue = page_queue;
                    temp->occupied_pages = total_pages - unoccupied_pages;
                    strcpy(temp->state, "RUNNING");
                    event_queue = push(event_queue, temp);


                    if (next_p->current_running_time > 0) {
                        if (next_p->load_time > 0) {
                            next_p->load_time--;
                        } else if (next_p->load_time == 0) {
                            next_p->remaining_time--;
                        }
                        next_p->current_running_time--;
                    }

                }

            }
        } else if (strcmp(scheduling_algorithm, "rr") == 0) {
            Process *first_p = running_queue->process;

            if (first_p->current_running_time > 0) {
                if (first_p->load_time > 0) {
                    first_p->load_time--;
                } else if (first_p->load_time == 0) {
                    first_p->remaining_time--;
                }
                first_p->current_running_time--;

            } else if (first_p->remaining_time == 0) {
                //TODO: Finishes Process
                running_queue = remove_by_process(running_queue, first_p);
                num_finished++;

                int proc_remaining = get_num_processes(running_queue);

                // Event Queue
                Process *temp = (Process *) malloc(sizeof(Process));
                memcpy(temp, first_p, sizeof *first_p);
                temp->current_time = time;
                strcpy(temp->state, "FINISHED");
                temp->proc_remaining = proc_remaining;
                event_queue = push(event_queue, temp);

                if (proc_remaining > 0) {
                    Process *next_p = running_queue->process;

                    if (quantum > next_p->remaining_time) {
                        next_p->current_running_time = next_p->remaining_time + next_p->load_time;
                    } else {
                        next_p->current_running_time = quantum + next_p->load_time;
                    }

                    page_t *page_queue = (page_t *) malloc(sizeof(page_t));
                    page_queue->next = NULL;

                    // Load 1st process to memory table and queue
                    int unoccupied_pages = 0;
                    for (int i = 0; i < total_pages; i++) {
                        if (memory_table[i] == NULL) {
                            unoccupied_pages++;
                            continue;
                        } else if (memory_table[i]->id == next_p->id) {
                            insert_page(page_queue, i);
                        }
                    }

                    // Event Queue [Running: After looping]
                    Process *temp = (Process *) malloc(sizeof(Process));
                    memcpy(temp, next_p, sizeof *next_p);
                    temp->current_time = time;
                    temp->page_queue = page_queue;
                    temp->occupied_pages = total_pages - unoccupied_pages;
                    strcpy(temp->state, "RUNNING");
                    event_queue = push(event_queue, temp);


                    if (next_p->current_running_time > 0) {

                        if (next_p->load_time > 0) {
                            next_p->load_time--;
                        } else if (next_p->load_time == 0) {
                            next_p->remaining_time--;
                        }
                        next_p->current_running_time--;

                    }

                }

            } else if (first_p->current_running_time == 0) {

                // Reach Quantum
                running_queue = remove_by_process(running_queue, first_p);

                if (quantum > first_p->remaining_time) {
                    first_p->current_running_time = first_p->remaining_time + first_p->load_time;
                } else {
                    first_p->current_running_time = quantum + first_p->load_time;
                }

                running_queue = push(running_queue, first_p);

                int proc_remaining = get_num_processes(running_queue);

                if (proc_remaining > 0) {

                    Process *next_p = running_queue->process;

                    if (quantum > next_p->remaining_time) {
                        next_p->current_running_time = next_p->remaining_time + next_p->load_time;
                    } else {
                        next_p->current_running_time = quantum + next_p->load_time;
                    }

                    page_t *page_queue = (page_t *) malloc(sizeof(page_t));
                    page_queue->next = NULL;

                    // Load 1st process to memory table and queue
                    int unoccupied_pages = 0;
                    for (int i = 0; i < total_pages; i++) {
                        if (memory_table[i] == NULL) {
                            unoccupied_pages++;
                            continue;
                        } else if (memory_table[i]->id == next_p->id) {
                            insert_page(page_queue, i);
                        }
                    }

                    // Event Queue [Running: After looping]
                    Process *temp = (Process *) malloc(sizeof(Process));
                    memcpy(temp, next_p, sizeof *next_p);
                    temp->current_time = time;
                    temp->page_queue = page_queue;
                    temp->occupied_pages = total_pages - unoccupied_pages;
                    strcpy(temp->state, "RUNNING");
                    event_queue = push(event_queue, temp);

                    if (next_p->current_running_time > 0) {

                        if (next_p->load_time > 0) {
                            next_p->load_time--;
                        } else if (next_p->load_time == 0) {
                            next_p->remaining_time--;
                        }
                        next_p->current_running_time--;

                    }

                }
            }

        }

        time++;
    }

    print_events(event_queue, memory, memory_management);
    print_statistics(event_queue, process_queue);
}


void print_events(node_t *head, int memory, char *memory_management) {

    // Printing
    node_t *current = head;
    while (current->next != NULL) {
        Process *p = current->process;

        int current_time = p->current_time;
        int process_id = p->id;
        int remaining_time = p->remaining_time;
        int proc_remaining = p->proc_remaining;
        int load_time = p->load_time;
        char state[10];
        strcpy(state, p->state);

        printf("%d", current_time);
        printf(", %s", state);

        if (strcmp(state, "RUNNING") == 0) {
            int occupied_pages = p->occupied_pages;
            printf(", id=%d", process_id);
            printf(", remaining-time=%d", remaining_time);

            if (strcmp(memory_management, "u") == 0) {

            } else if (strcmp(memory_management, "p") == 0) {
                page_t *page_queue = p->page_queue;

                int total_pages = memory / 4;


                printf(", load-time=%d", load_time);
                printf(", mem-usage=");
                int mem_usage = (occupied_pages * 100 + total_pages - 1) / total_pages;
                printf("%d%%", mem_usage);

                printf(", mem-addresses=[");

                page_t *c = page_queue;
                int commas = 0;
                while (c->next != NULL) {
                    printf("%d", c->next->number);
                    c = c->next;
                    commas++;
                    if (commas < get_num_pages(page_queue)) {
                        printf(",");
                    }
                }
                printf("]");
            } else if (strcmp(memory_management, "v") == 0) {
                page_t *page_queue = p->page_queue;

                int total_pages = memory / 4;


                printf(", load-time=%d", load_time);
                printf(", mem-usage=");
                int mem_usage = (occupied_pages * 100 + total_pages - 1) / total_pages;
                printf("%d%%", mem_usage);

                printf(", mem-addresses=[");
                page_t *c = page_queue;
                int commas = 0;
                while (c->next != NULL) {
                    printf("%d", c->next->number);
                    c = c->next;
                    commas++;
                    if (commas < get_num_pages(page_queue)) {
                        printf(",");
                    }
                }
                printf("]");
            }

            printf("\n");
        } else if (strcmp(state, "EVICTED") == 0) {

            if (strcmp(memory_management, "u") == 0) {

            } else if (strcmp(memory_management, "p") == 0) {
                page_t *page_queue = p->page_queue;

                printf(", mem-addresses=[");
                page_t *c = page_queue;
                int commas = 0;
                while (c->next != NULL) {
                    printf("%d", c->next->number);
                    c = c->next;
                    commas++;
                    if (commas < get_num_pages(page_queue)) {
                        printf(",");
                    }
                }
                printf("]");
                printf("\n");
            } else if (strcmp(memory_management, "v") == 0) {
                page_t *page_queue = p->page_queue;

                printf(", mem-addresses=[");
                page_t *c = page_queue;
                int commas = 0;
                while (c->next != NULL) {
                    printf("%d", c->next->number);
                    c = c->next;
                    commas++;
                    if (commas < get_num_pages(page_queue)) {
                        printf(",");
                    }
                }
                printf("]");
                printf("\n");
            }


        } else if (strcmp(state, "FINISHED") == 0) {
            printf(", id=%d", process_id);
            printf(", proc-remaining=%d", proc_remaining);
            printf("\n");
        }


        current = current->next;

    }
}


void print_statistics(node_t *head, node_t *h) {

    // Throughput
    node_t *current = head;
    int max_throughput = 0;
    int current_time = 0;
    int num_processes = get_num_processes(h);
    int min_throughput = num_processes;
    int finished_time = 0;

    while (current->next != NULL) {
        Process *p = current->process;
        current_time = p->current_time;
        finished_time = current_time;
        current = current->next;
    }

    int num_interval = (finished_time + 60 - 1) / 60;

    for (int i = 0; i < num_interval; i++) {
        int throughput = 0;

        current = head;
        while (current->next != NULL) {
            Process *p = current->process;
            int current_time = p->current_time;

            if (strcmp(p->state, "FINISHED") == 0 && (current_time - 1) / 60 == i) {
                throughput++;
            }

            current = current->next;
        }


        if (min_throughput > throughput) {
            min_throughput = throughput;
        }

        if (max_throughput < throughput) {
            max_throughput = throughput;
        }
    }

    int avg_throughput = (num_processes + (num_interval - 1)) / num_interval;
    printf("Throughput %d, %d, %d\n", avg_throughput, min_throughput, max_throughput);

    // Turnaround time
    current = head;
    int sum_turnaround = 0;
    while (current->next != NULL) {

        Process *p = current->process;

        if (strcmp(p->state, "FINISHED") == 0) {
            int current_time = p->current_time;
            int arrival_time = p->arrival_time;

            sum_turnaround += current_time - arrival_time;
        }
        current = current->next;

    }

    int turnaround_time = (sum_turnaround + (num_processes - 1)) / num_processes;
    printf("Turnaround time %d\n", turnaround_time);

    // Time overhead
    current = head;
    current_time = 0;
    double sum_overhead = 0;
    double max_overhead = 0;
    while (current->next != NULL) {

        Process *p = current->process;

        if (strcmp(p->state, "FINISHED") == 0) {
            int current_time = p->current_time;
            int arrival_time = p->arrival_time;
            int jobtime = p->jobtime;

            int turnaround = current_time - arrival_time;

            double overhead = (double) turnaround / jobtime;

            sum_overhead += overhead;
            if (max_overhead < overhead) {
                max_overhead = overhead;
            }
        }
        current = current->next;
    }


    double avg_overhead = sum_overhead / num_processes;
    printf("Time overhead %.2f %.2f\n", max_overhead, avg_overhead);

    // Makespan
    current = head;
    int makespan = 0;
    while (1) {

        if (current->next->next == NULL) {
            Process *p = current->process;
            makespan = p->current_time;
            break;
        }

        current = current->next;
    }

    printf("Makespan %d\n", makespan);

}