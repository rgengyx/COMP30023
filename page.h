typedef struct page{
    int number;
    struct page* next;
} page_t;

/* function declaration */
void insert_page(page_t * head, int number);