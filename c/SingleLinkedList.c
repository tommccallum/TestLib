#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "SingleLinkedList.h"



// Adds the element to the front
SingleLinkedList* single_linked_list_add_front(SingleLinkedList* head, void* data) {
    SingleLinkedList* new_node = (SingleLinkedList*) malloc(sizeof(SingleLinkedList));
    new_node->data = data;
    new_node->next = head;
    return new_node;
}

SingleLinkedList* single_linked_list_add_back(SingleLinkedList* head, void* data) {
    SingleLinkedList* new_node = (SingleLinkedList*) malloc(sizeof(SingleLinkedList));
    new_node->data = data;
    new_node->next = NULL;

    if ( head == NULL ) {
        return new_node;
    } else {
        SingleLinkedList* runner =  head;
        while ( runner->next ) {
            runner = runner->next;
        }
        runner->next = new_node;
        return head;
    }
}

SingleLinkedList* single_linked_list_remove_front(SingleLinkedList* head) {
    if ( head == NULL ) {
        return head;
    }
    SingleLinkedList* new_head = head->next;
    if ( head->data ) {
        free(head->data);
    }
    head->next = NULL;
    free(head);
    return new_head;
}

// Althought we don't have to return anything we do so that it is the 
// same for all remove functions.
SingleLinkedList* single_linked_list_remove_back(SingleLinkedList* head) {
    SingleLinkedList* runner =  head;
    if ( runner == NULL ) {
        // CASE LEN == 0
        return NULL;
    }
    if ( runner->next == NULL ) {
        // CASE LEN == 1
        free(runner->data);
        free(runner);
        return NULL;
    }
    // CASE LEN > 1
    while ( runner->next && runner->next->next) {
        runner = runner->next;
    }
    // so runner->next->next fails so runner->next is the last item
    free(runner->next->data);
    free(runner->next);
    runner->next = NULL;
    return head;
}

SingleLinkedList* single_linked_list_remove_item(SingleLinkedList* head, void* data) {
    SingleLinkedList* runner =  head;
    if ( runner == NULL ) {
        // CASE 0
        return NULL;
    }
    if ( runner->data == data ) {
        // CASE first
        head = runner->next;
        free(runner->data);
        free(runner);
        return head;
    }
    SingleLinkedList* prev = runner;
    runner = runner->next;
    while ( runner && runner->next ) {
        if ( runner->data == data ) {
            // we want to delete runner
            prev->next = runner->next;
            runner->next = NULL;
            free(runner->data);
            free(runner);
            return head;
        }
        prev = runner;
        runner = runner->next;
    }
    assert(runner != NULL);
    if ( runner->data == data ) {
        // CASE last
        prev->next = NULL;
        free(runner->data);
        free(runner);
        return head;
    }
    return head;
}

void single_linked_list_for_each(SingleLinkedList* head, single_linked_list_visitor fn) {
    SingleLinkedList* runner = head;
    while ( runner ) {
        fn(runner->data);
        runner = runner->next;
    }
}

void single_linked_list_for_each_mutate(SingleLinkedList* head, single_linked_list_modifier fn) {
    SingleLinkedList* runner = head;
    while ( runner ) {
        fn(&(runner->data));
        runner = runner->next;
    }
}

SingleLinkedList* single_linked_list_remove_all(SingleLinkedList* head) {
    while ( head != NULL) {
        head = single_linked_list_remove_front(head);
    }
    return head;
}

SingleLinkedList* single_linked_list_remove_all_backwards(SingleLinkedList* head) {
    while ( head != NULL) {
        head = single_linked_list_remove_back(head);
    }
    return head;
}