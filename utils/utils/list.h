#pragma once

#include <stdio.h>

//Некий шаблонный список, который позволяет сохранять указатели на ресурсы. 
//НА УКАЗАТЕЛИ!!! НЕ КОПИРУЕТ ДАННЫЕ!!!

//Нода листа. Без комментариев.
typedef struct ListNode {
    void* data;
    struct ListNode* next;
    struct ListNode* prev;
} ListNode;

//Лист. Без комментариев.
typedef struct List {
    struct ListNode* head;
    struct ListNode* tail;
} List;

//Ининциализирует список
void initList(List* list);

//Добавляет в конец списка УКАЗАТЕЛЬ на данные.
void pushBack(List* list, void* data);


//Добавляет в начало списка УКАЗАТЕЛЬ на данные.
void pushFront(List* list, void* data);

//Удаляет ListNode из начала списка и возвращает УКАЗАТЕЛЬ на данные, которые были вытащены из удалённого элеимента.
void* popFront(List* list);

//Удалаяет ListNode из конца списка и возвращает УКАЗАТЕЛЬ на данные, которые были вытащены из удалённого элеимента.
void* popBack(List* list);

//Удаляет вообще всё: не только данные, необходимые для работы списка, но и данные, которые лежат внутри.
void freeList(List* list);