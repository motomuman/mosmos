#ifndef _LIST_H_
#define _LIST_H_

#include "types.h"

struct list_item {
	struct list_item *next;
};

struct listctl {
	uint16_t count;
	struct list_item *head;
	struct list_item *tail;
};

void list_init(struct listctl *list);
int list_empty(struct listctl *list);
struct list_item *list_popfront(struct listctl *list);
void list_pushfront(struct listctl *list, struct list_item *item);
void list_pushback(struct listctl *list, struct list_item *item);
struct list_item* list_head(struct listctl *list);
struct list_item* list_next(struct list_item *item);
void list_insert(struct listctl *list, struct list_item *prev_item, struct list_item *new_item);
void list_remove(struct listctl *list, struct list_item *prev_item);

#endif
