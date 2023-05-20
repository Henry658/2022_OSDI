#ifndef _LIST_H
#define _LIST_H

typedef struct list_head {
	struct list_head *next, *prev;
}list_head_t;


#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)
	
static inline void INIT_LIST_HEAD(struct list_head *list){
	list->next = list;
	list->prev = list;
}

static inline void __list_add(struct list_head *new,
			       struct list_head *prev,
			       struct list_head *next){
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void list_add(struct list_head *new,
                            struct list_head *head){
	__list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new,
                                 struct list_head *head){
	__list_add(new, head->prev, head);
}                             

static inline void __list_del(struct list_head * prev,
                              struct list_head * next){
	next->prev = prev;
	prev->next = next;
}

static inline void list_del_entry(struct list_head *entry){
	__list_del(entry->prev, entry->next);
}

static inline int list_is_head(const struct list_head *list,
                               const struct list_head *head){
	return list == head;
}

static inline int list_empty(const struct list_head *head){
	return head->next == head;
}

#define list_for_each(pos, head) \ 
	for (pos = (head)->next; !list_is_head(pos, (head)); pos = pos->next)

#endif
