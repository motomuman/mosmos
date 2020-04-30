#include "list.h"
#define NULL 0

void list_init(struct listctl *list)
{
	list->head = NULL;
	list->tail = NULL;
	list->count = 0;
	return;
}

int list_empty(struct listctl *list) {
	return list->count == 0;
}

struct list_item *list_popfront(struct listctl *list)
{
	if(list_empty(list)) {
		return NULL;
	}
	list->count--;
	struct list_item *first = list->head;
	list->head = first->next;
	first->next = NULL;
	return first;
}

void list_pushfront(struct listctl *list, struct list_item *item)
{
	if(list_empty(list)) {
		item->next = NULL;
		list->head = item;
		list->tail = item;
		list->count++;
		return;
	}
	item->next = list->head;
	list->head = item;
	list->count++;
	return;
}

void list_pushback(struct listctl *list, struct list_item *item)
{
	item->next = NULL;
	if(list_empty(list)) {
		list->head = item;
		list->tail = item;
		list->count++;
		return;
	}
	list->tail->next = item;
	list->tail = list->tail->next;
	list->count++;
	return;
}

// Insert new_item after prev_item
void list_insert(struct listctl *list, struct list_item *prev_item, struct list_item *new_item)
{
	if(prev_item == NULL) {
		return;
	}

	struct list_item *post_item = prev_item->next;
	prev_item->next = new_item;
	new_item->next = post_item;
	list->count++;
	return;
}

// Remove item after prev_item
void list_remove(struct listctl *list, struct list_item *prev_item)
{
	if(prev_item == NULL) {
		return;
	}

	struct list_item *remove_item = prev_item->next;
	if(remove_item == NULL) {
		return;
	}

	prev_item->next = remove_item->next;
	remove_item->next = NULL;

	if(prev_item->next == NULL) {
		list->tail = prev_item;
	}

	list->count--;
	return;
}


struct list_item* list_head(struct listctl *list)
{
	return list->head;
}

struct list_item* list_next(struct list_item *item)
{
	return item->next;
}
