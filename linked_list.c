#include <stdlib.h>
#include <stdio.h>
#include "linked_list.h"

Node *create_node(Node *next, unsigned long id) {
    Node *node = (Node *) malloc(sizeof(Node));
    if (!node) {
        printf("Error creating a new node.\n");
        return NULL;
    }
    node->id = id;
    node->n_of_msgs = 0;
    node->next = next;
    return node;
}

int destroy_node(Node *node) {
    if (node == NULL) {
        printf("Error destroying node.");
        return 0;
    }
    node->next = NULL;
    free(node);
    return 1;
}

LinkedList *create_list(unsigned long id) {
    LinkedList *list = (LinkedList *) malloc(sizeof(*list));
    if (!list) {
        printf("Error creating a list.\n");
        return NULL;
    }
    list->head = create_node(NULL, id);
    list->size = 0;
}

int add_node(LinkedList *list, Node *node) {
    if (list == NULL || node == NULL) {
        printf("Error adding node to list.");
        return 0;
    }
    Node *p = list->head;
    while (p->next != NULL) { p = p->next; }
    p->next = node;
    list->size++;
    return 1;
}

int remove_node(LinkedList *list, int index) {
    if (list == NULL || index > list->size) {
        printf("Error deleting node.");
        return 0;
    }
    Node *p = list->head;
    int j = 0;
    while (j < index - 1) {
        p = p->next;
        ++j;
    }
    p->next = p->next->next;
    return 1;
}


Node *retrive_node(LinkedList *list, int index) {
    if (list == NULL || index > list->size) {
        printf("Error retrieving node.");
        return 0;
    }
    Node *p = list->head;
    int j = 0;
    while (j < index) {
        p = p->next;
        ++j;
    }
    return p;
}

Node *search_by_id(LinkedList *list, unsigned long id) {
    if (list == NULL) {
        printf("Error searching node.");
        return 0;
    }
    Node *p = list->head;
    while (p->next != NULL) {
        if (p->id == id)
            return p;
        p = p->next;
    }
}