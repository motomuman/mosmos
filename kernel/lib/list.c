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

struct list_item* list_head(struct listctl *list)
{
	if(list_empty(list)) {
		return NULL;
	}
	return list->head;
}

struct list_item* list_next(struct list_item *item)
{
	return item->next;
}
