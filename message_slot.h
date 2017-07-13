#ifndef OS_HW5_MESSGE_SLOT_H
#define OS_HW5_MESSGE_SLOT_H

#include <linux/ioctl.h>

/* The major device number. We can't rely on dynamic
 * registration any more, because ioctls need to know
 * it. */
#define MAJOR_NUM 245


/* Set the message of the device driver */
#define IOCTL_SET_CHA _IOW(MAJOR_NUM, 0, unsigned long)

#define DEVICE_RANGE_NAME "message_slot"
#define DEVICE_FILE_NAME "simple_message_slot"
#define SUCCESS 0
#define NUM_OF_MSG 4
#define MSG_LEN 128

typedef struct message_slot_t {
    unsigned long id;
	short channel_index;
	char messages[NUM_OF_MSGS][MSG_LEN];
} MessageSlot;

typedef struct node_t {
    MessageSlot msgs;
    struct node_t *next;
} Node;

typedef struct linked_list_t {
    Node *head;
    int size;
} LinkedList;

Node *create_node(Node *next, unsigned long id);

int destroy_node(Node *node);

/* TODO remove, no need */
int add_msg(Node *node, char *msg);

LinkedList *create_list();

int add(LinkedList *list, Node* node);

Node *pop(LinkedList *list);

Node* get_node_by_id(LinkedList *list, int id);

int destroy_list(LinkedList *list);

#endif //OS_HW5_MESSGE_SLOT_H
