/* Declare what kind of code we want from the header files
   Defining __KERNEL__ and MODULE allows us to access kernel-level
   code not usually available to userspace programs. */
#pragma clang diagnostic push
#pragma ide diagnostic ignored "CannotResolve"
#undef __KERNEL__
#define __KERNEL__ /* We're part of the kernel */
#undef MODULE
#define MODULE     /* Not a permanent part, though. */

/* ***** Example w/ minimal error handling - for ease of reading ***** */

//Our custom definitions of IOCTL operations
#include "message_slot.h"

#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <asm/uaccess.h>    /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h!*/
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <errno.h>

MODULE_LICENSE("GPL");

#define PRINT_ERROR(s) printk("[Error] - %s, %s\n", s, strerror(errno));

struct chardev_info {
    spinlock_t lock;
};

static LinkedList *global_list = NULL;

/***************** message slot functions ********************/

Node *create_node(int id) {
    Node *node = (Node *) kmalloc(sizeof(Node), GFP_KERNEL);
    if (!node) {
    	PRINT_ERROR("Failed to create node");
        return NULL;
    }
    node->msgs.id = id;
    node->msgs.channel_index = -1;
    node->next = NULL;
    return node;
}

int destroy_node(Node *node){
	if(node == NULL){
		PRINT_ERROR("Null pointer argument");
		return -1;
	}
	node->next = NULL;
	kfree(node);
	return 0;
}

/* TODO remove, no need */
int add_msg(Node *node, char *msg){
	if(node == NULL || msg == NULL){
		PRINT_ERROR("Null pointer argument");
		return -1;
	} else if(node->msgs.channel_index < 0 && 3 < node->msgs.channel_index){
		PRINT_ERROR("Can't add message");
		return -1;
	}
	if(strlen(msg) > MSG_LEN)
		strncpy(node.msgs.messages[node.msgs.channel_index], msg, MSG_LEN);
	else
		strcpy(node.msgs.messages[node.msgs.channel_index], msg);
	return 0;
}

LinkedList *create_list(){
	LinkedList *list = (LinkedList *) kmalloc(sizeof(*list), GFP_KERNEL);
	if(list == NULL){
		PRINT_ERROR("Failed to create list");
        return NULL;	
	}
	list->head 	= NULL;
	list.size  	= 0;
	return list;
}

int add(LinkedList *list, Node* node){
	if(list == NULL || node == NULL){
		PRINT_ERROR("Null pointer argument");
		return -1;
	}
	node->next = list->head;
	list->head = node;
	list.size++;
	return 0;
}

int pop(LinkedList *list){
	if(list == NULL){
		PRINT_ERROR("Null pointer argument");
		return -1;
	} else if(0 == list.size){
		PRINT_ERROR("Empty list");
		return -1;
	}

	Node *p = list->head;
	list->head = p->next;
	list.size--;
	if(0 > destroy_node(p)){
		PRINT_ERROR("Failed to destory node");
		return -1;
	}
	return 0; 
}

Node* get_node_by_id(LinkedList *list, int id){
	if(list == NULL){
		PRINT_ERROR("Null pointer argument");
		return NULL;	
	}
	Node *p = list->head;
	for(; p.next != NULL; p = p.next){
		if(p.msgs.id == id)
			return p;
	}
	return NULL;
}

int destroy_list(LinkedList *list){
	if(list == NULL){
		PRINT_ERROR("Null pointer argument");
		return -1;	
	}
	while(pop(list) != -1);
	kfree(list);
	return 0;
}

/***************** Drive *****************/

// static int dev_open_flag = 0; /* used to prevent concurent access into the same device */
static struct chardev_info device_info;
static unsigned long flags; // for spinlock

/***************** char device functions *********************/

/* process attempts to open the device file */
static int device_open(struct inode *inode, struct file *file) {
    printk("Device_open(%p)\n", file);

    /*
     * We don't want to talk to two processes at the same time
     */
    spin_lock_irqsave(&device_info.lock, flags);

    int id = file->f_inode->i_ino;
    Node *p = get_node_by_id(global_list,id);
    if(p == NULL){
    	p = create_node(id);
    	if(p == NULL){
    		PRINT_ERROR("Unable to open device");
    		spin_unlock_irqrestore(&device_info.lock, flags);
    		return -EBUSY;
    	}
    	if(add(global_list, p) < 0){
    		PRINT_ERROR("Failed to add file to list");
    		spin_unlock_irqrestore(&device_info.lock, flags);
    		return -EBUSY;	
    	}
        printk("Device_opened (%p)\n", file);
    } else {
        printk("The device is already open(%p)\n", file);
    }
    spin_unlock_irqrestore(&device_info.lock, flags);
    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) {
    printk("device_release(%p,%p)\n", inode, file);
    return SUCCESS;
}

/* a process which has already opened
   the device file attempts to read from it */
static ssize_t device_read(	struct file *file, 		/* see include/linux/fs.h 					*/
							char __user * buffer,	/* buffer to be filled with data 			*/
						    size_t length,			/* length of the buffer 					*/
						    loff_t * offset) {		/* read doesnt really do anything (for now) */
	if(buffer == NULL || file == NULL || offset == NULL){
		PRINT_ERROR("Null pointer argument");
		return -EINVAL;
	}
	int i;
	printk("device_read(%p,%d)\n", file, length);
	/*
     * We don't want to talk to two processes at the same time
     */
	spin_lock_irqsave(&device_info.lock, flags);
	
	/* Find message slot in the list */
	Node *p = get_node_by_id(global_list, file->f_inode->i_ino);
	if(p == NULL){
		PRINT_ERROR("No message slot available");
		spin_unlock_irqrestore(&device_info.lock, flags);
		return -EINVAL;	
	}

	/* Check if messge slot channel is valid */
	if(p.msgs.channel_index < 0){
		PRINT_ERROR("Message slot channel isn't initialized");
		spin_unlock_irqrestore(&device_info.lock, flags);
		return -EINVAL;	
	}

	/* Read buffer from user */
	length = length > MSG_LEN ? MSG_LEN : length;
	for(i = 0; i < length; ++i){
		if(0 > put_user(p->msgs.messages[p.msgs.channel_index][i], 
						buffer + i)){
			PRINT_ERROR("Failed reading data from user");
			spin_unlock_irqrestore(&device_info.lock, flags);
			return -EINVAL;	
		}
	}

	/* Unlock spin lock */
    spin_unlock_irqrestore(&device_info.lock, flags);
	return (ssize_t) i;
}

/* somebody tries to write into our device file */
static ssize_t device_write(struct file *file,			/* see include/linux/fs.h 					*/
							const char __user * buffer, /* buffer to be filled with data 			*/
							size_t length, 				/* length of the buffer 					*/
							loff_t * offset) {			/* read doesnt really do anything (for now) */
	if(buffer == NULL || file == NULL || offset == NULL){
		PRINT_ERROR("Null pointer argument");
		return -EINVAL;
	}
	int i;
	printk("device_write(%p,%d)\n", file, length);
	/*
     * We don't want to talk to two processes at the same time
     */
	spin_lock_irqsave(&device_info.lock, flags);

	/* Find message slot in the list */
	Node *p = get_node_by_id(global_list, file->f_inode->i_ino);
	if(p == NULL){
		PRINT_ERROR("No message slot available");
		spin_unlock_irqrestore(&device_info.lock, flags);
		return -EINVAL;	
	}

	/* Check if messge slot channel is valid */
	if(p.msgs.channel_index < 0){
		PRINT_ERROR("Message slot channel isn't initialized");
		spin_unlock_irqrestore(&device_info.lock, flags);
		return -EINVAL;	
	}

	/* Wrtie buffer to user */
	length = length > MSG_LEN ? MSG_LEN : length;
	for(i = 0; i < MSG_LEN; ++i){
		if(i < length) {
			if(0 > get_user(p->msgs.messages[p.msgs.channel_index][i], 
							buffer + i)){
				PRINT_ERROR("Failed write data to user");
				spin_unlock_irqrestore(&device_info.lock, flags);
				return -EINVAL;
			}
		} else {
			p->msgs.messages[p.msgs.channel_index][i] = 0;
		}
	}

	/* Unlock spin lock */
	spin_unlock_irqrestore(&device_info.lock, flags);
	
	/* return the number of input characters used */
	return (ssize_t) i;
}

//----------------------------------------------------------------------------
static long device_ioctl(struct file *file,			/* struct inode*  inode */
						 unsigned int ioctl_num,		/* The number of the ioctl */
						 unsigned long ioctl_param){/* The parameter to it */
    Node *p;
    /* Switch according to the ioctl called */
    if (IOCTL_SET_CHA == ioctl_num && ioctl_param < 4) {
    	spin_lock_irqsave(&device_info.lock, flags);
        
        p = get_node_by_id(global_list, file->f_inode->i_ino);
    	if(p == NULL){
			PRINT_ERROR("No message slot available");
			spin_unlock_irqrestore(&device_info.lock, flags);
			return -EINVAL;	
		}

		p.msgs.channel_index = (short) ioctl_param;
		printk("message_slot, ioctl: Set channel_index to: %ld\n", ioctl_param);
    	spin_unlock_irqrestore(&device_info.lock, flags);
    	return SUCCESS;
    }

    return -EINVAL;
}

/************** Module Declarations *****************/

/* This structure will hold the functions to be called
 * when a process does something to the device we created */

struct file_operations Fops = {
        .read = device_read,
        .write = device_write,
        .unlocked_ioctl= device_ioctl,
        .open = device_open,
        .release = device_release,
};

/* Called when module is loaded.
 * Initialize the module - Register the character device */
static int __init message_slot_init(void) {
    unsigned int rc = 0;
    /* init dev struct*/
    memset(&device_info, 0, sizeof(struct chardev_info));
    spin_lock_init(&device_info.lock);

    /* Register a character device. Get newly assigned major num */
    rc = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops /* our own file operations struct */);

    /*
     * Negative values signify an error
     */
    if (rc < 0) {
        printk(KERN_ALERT, "%s failed with %d\n", "Sorry, registering the character device ", MAJOR_NUM);
        return -1;
    }

    printk("Registeration is a success. The major device number is %d.\n", MAJOR_NUM);
    printk("If you want to talk to the device driver,\n");
    printk("you have to create a device file:\n");
    printk("mknod /dev/%s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM);
    printk("You can echo/cat to/from the device file.\n");
    printk("Dont forget to rm the device file and rmmod when you're done\n");

    return SUCCESS;
}

/* Cleanup - unregister the appropriate file from /proc */
static void __exit message_slot_cleanup(void)
{
    /* 
     * Unregister the device 
     * should always succeed (didnt used to in older kernel versions)
     */

    spin_lock_irqsave(&device_info.lock, flags);
    destroy_list(global_list);
    spin_unlock_irqrestore(&device_info.lock, flags);
    
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);

    printk("Unregistered device %d\n", MAJOR_NUM);
}

module_init(message_slot_init);
module_exit(message_slot_cleanup);

#pragma clang diagnostic pop