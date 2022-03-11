#ifndef SINGLE_LINKED_LIST
#define SINGLE_LINKED_LIST

typedef struct _SingleLinkedList {
    void* data;                                 // this is type erasure as the programmer has to know what type it is
    struct _SingleLinkedList* next;
} SingleLinkedList;

SingleLinkedList* single_linked_list_add_front(SingleLinkedList* head, void* data);
SingleLinkedList* single_linked_list_add_back(SingleLinkedList* head, void* data);
SingleLinkedList* single_linked_list_remove_front(SingleLinkedList* head);
SingleLinkedList* single_linked_list_remove_back(SingleLinkedList* head);
SingleLinkedList* single_linked_list_remove_item(SingleLinkedList* head, void* data);
SingleLinkedList* single_linked_list_remove_all(SingleLinkedList* head);
SingleLinkedList* single_linked_list_remove_all_backwards(SingleLinkedList* head);

typedef void (*single_linked_list_visitor)(void* item);
typedef void (*single_linked_list_modifier)(void** item);

void single_linked_list_for_each(SingleLinkedList* head, single_linked_list_visitor fn);
void single_linked_list_for_each_mutate(SingleLinkedList* head, single_linked_list_modifier fn);


#endif


