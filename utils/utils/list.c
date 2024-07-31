#include "list.h"
#include "../alloc/alloc.h"

void initList(List* list) {
    list->head = NULL;
    list->tail = NULL;
}

void pushBack(List* list, void* data) {
    ListNode* newNode = (ListNode*) myMalloc(sizeof(ListNode));
    newNode->data = data;
    newNode->next = newNode->prev = NULL;

    if (!list->head) {
        list->head = list->tail = newNode;
    } else {
        newNode->prev = list->tail;
        list->tail->next = newNode;
        list->tail = newNode;
    }
}

void pushFront(List* list, void* data) {
    ListNode* newNode = (ListNode*) myMalloc(sizeof(ListNode));
    newNode->data = data;
    newNode->prev = newNode->next = NULL;

    if (!list->head) {
        list->head = list->tail = newNode;
    } else {
        newNode->next = list->head;
        list->head->prev = newNode;
        list->head = newNode;
    }
}

void* popFront(List* list) {
    if (!list->head) {
        return NULL;
    }

    void* data = list->head->data;
    ListNode* temp = list->head;
    list->head = list->head->next;

    if (!list->head) {
        list->tail = NULL;
    } else {
        list->head->prev = NULL;
    }

    myFree(temp);
    return data;
}

void* popBack(List* list) {
    if (!list->head) {
        return NULL;
    }

    void* data = list->tail->data;
    ListNode* temp = list->tail;
    list->tail = list->tail->prev;

    if (!list->tail) {
        list->head = NULL;
    } else {
        list->tail->next = NULL;
    }
    
    myFree(temp);
    return data;
}

void freeList(List* list) {
    ListNode* current = list->head;
    
    while (current) {
        ListNode* temp = current;
        current = current->next;
        myFree(temp->data);
        myFree(temp);
    }

    myFree(list);
}