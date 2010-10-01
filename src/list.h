/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#ifndef LIST_H_
#define LIST_H_

/* Forward declaration */
struct item;

/** A doubly linked list
 *
 */
typedef struct list {
	/** The first item on the list or NULL if empty. */
	struct item *head;
	
	/** The last item on the list or NULL if empty. */
	struct item *tail;
} list_t;

/** An item of a doubly linked list
 *
 * The item should be first in listable structures.
 *
 */
typedef struct item {
	/** The list that we currently belong to. */
	struct list *list;
	
	/** The next item on the list or NULL if first. */
	struct item *prev;
	
	/** The previous item on the list or NULL if last. */
	struct item *next;
} item_t;

#define for_each(list, member, type) \
	for ((member) = (type *) (list).head; \
		(member) != NULL; \
		(member) = (type *) (member)->item.next)

extern void list_init(list_t *list);
extern void item_init(item_t *item);
extern void list_append(list_t *list, item_t *item);
extern void list_remove(list_t *list, item_t *item);

#endif
