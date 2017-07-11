#ifndef OS_HW5_LINKED_LIST_H
#define OS_HW5_LINKED_LIST_H


#define NUM_OF_MSGS 4
#define MSG_LEN 128

struct list_node_t {
    /* Values */
    char messages[NUM_OF_MSGS][MSG_LEN];
    unsigned long id;
    short n_of_msgs;
    struct list_node_t *next;
};

typedef struct list_node_t Node;

Node *create_node(Node *next, unsigned long);

int destroy_node(Node *node);

int add_msg(Node *node, char *msg);

struct linked_list_t {
    Node *head;
    int size;
};
typedef struct linked_list_t LinkedList;

LinkedList *create_list(unsigned long);

int add_node(LinkedList *list, Node *node);

int remove_node(LinkedList *list, int index);

Node* search_by_id(LinkedList *list, unsigned long id);

Node *retrive_node(LinkedList *list, int index);


#endif //OS_HW5_LINKED_LIST_H
