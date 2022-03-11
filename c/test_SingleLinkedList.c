#include <stdio.h>
#include <stdlib.h>
#include "SingleLinkedList.h"
#include <assert.h>

typedef struct _data {
    int num;
} Node;

void print_node(void* data) {
    if ( data ) {
        printf("%d-", ((Node*) data)->num);
    }
}

void inc_node(void* data) {
    if ( data ) {
        ((Node*) data)->num++;
    }
}

void set_node(void** data) {
    if ( ((Node*)*data)->num == 7 ) {
        free(*data);
        *data = NULL;
        *data = (Node*) malloc(sizeof(Node));
        ((Node*)*data)->num = 13;
    }
}

int main() {
    SingleLinkedList* list=NULL;

    for( int ii=0; ii < 10; ii++ ) {
        Node* node = (Node*) malloc(sizeof(Node));
        node->num = ii;
        list = single_linked_list_add_back(list, node);
    }
    single_linked_list_for_each(list, print_node);
    list = single_linked_list_remove_all(list);
    assert(list == NULL);

    for( int ii=0; ii < 10; ii++ ) {
        Node* node = (Node*) malloc(sizeof(Node));
        node->num = ii;
        list = single_linked_list_add_back(list, node);
    }
    single_linked_list_for_each(list, print_node);
    list = single_linked_list_remove_all_backwards(list);

    // Test removing each item in turn
    for(int jj=0; jj < 10; jj++ ) {
        Node* sample_node;
        for( int ii=0; ii < 10; ii++ ) {
            Node* node = (Node*) malloc(sizeof(Node));
            node->num = ii;
            if ( ii == jj ) {
                sample_node = node;
            }
            list = single_linked_list_add_back(list, node);
        }
        list = single_linked_list_remove_item(list, sample_node);
        single_linked_list_for_each(list, print_node);
        printf("\n");
        list = single_linked_list_remove_all(list);
        assert(list == NULL);
    }

    for( int ii=0; ii < 10; ii++ ) {
        Node* node = (Node*) malloc(sizeof(Node));
        node->num = ii;
        list = single_linked_list_add_back(list, node);
    }
    single_linked_list_for_each(list, inc_node);
    single_linked_list_for_each(list, print_node);
    list = single_linked_list_remove_all(list);
    printf("\n");

    for( int ii=0; ii < 10; ii++ ) {
        Node* node = (Node*) malloc(sizeof(Node));
        node->num = ii;
        list = single_linked_list_add_back(list, node);
    }
    single_linked_list_for_each_mutate(list, set_node);
    single_linked_list_for_each(list, print_node);
    list = single_linked_list_remove_all(list);

    return 0;
}
