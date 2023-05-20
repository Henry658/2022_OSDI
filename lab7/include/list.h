#ifndef _LIST_H
#define _LIST_H

/* struct list_head - Head and node of a doubly-linked list
 * @prev: pointer to the previous node in the list
 * @next: pointer to the next node in the list
 */
typedef struct list_head {
	struct list_head *next, *prev;
}list_head_t;

/* LIST_HEAD - Declare list head and initialize it
 * @head: name of the new object
 */
#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

/* INIT_LIST_HEAD() - Initialize empty list head
 * @head: pointer to list head
 */
static inline void INIT_LIST_HEAD(struct list_head *head){
	head->next = head;
	head->prev = head;
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

/* list_add_tail() - Add a list node to the end of the list
 * @node: pointer to the new node
 * @head: pointer to the head of the list
 */
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

/* list_empty() - Check if list head has no nodes attached
 * @head: pointer to the head of the list
 */
static inline int list_empty(const struct list_head *head){
	return head->next == head;
}

#define list_for_each(pos, head) \ 
	for (pos = (head)->next; !list_is_head(pos, (head)); pos = pos->next)

//https://hackmd.io/@sysprog/linux-macro-containerof
	
#define offsetof(TYPE, MEMBER) ((uint64_t) &((TYPE *)0)->MEMBER)

// given member's address, struct type and member's name. return struct address

/* container_of() - Calculate address of object that contains address ptr
 * @ptr: pointer to member variable
 * @type: type of the structure containing ptr
 * @member: name of the member variable in struct @type
 *
 * Return: @type pointer of object containing ptr
 */
#define container_of(ptr, type, member) ({                          \
        const typeof( ((type *)0)->member ) *__pmember  = (ptr);     \
        (type *)( (char *)__pmember  - offsetof(type,member));      \
        })

/* list_entry() - Calculate address of entry that contains list node
 * @node: pointer to list node
 * @type: type of the entry containing the list node
 * @member: name of the list_head member variable in struct @type
 */
#define list_entry(ptr, type, member) container_of(ptr, type, member)


#endif
