#pragma once

#include "../utils/hashtable.h"

//Штука, которая сохраняет переменные. Представляет из себя хэш-таблицу, в которой лежат пары:
//ключ - Variable.

//Типы, которые можно сохранить.
typedef enum TYPE {
    INT,
    STRING,
} TYPE;

//Хранит данные об переменной.
typedef struct Variable {
    void* value;    //Само значение переменной.
    TYPE type;      //Тип переменной.
    char* name;     //Имя
} Variable;

//Сама таблица для хранения переменных.
extern HashTable* SVC_TABLE;

//Копирует name и value и сохраняет в SVC_TABLE.
int registerVariable(char* name, TYPE type, void* value);

//Копирует содержимое value в значение переменной name.
int changeVariable(char* name, void* value);

//Возвращает копию переменной name.
Variable* getVariable(char* name);

//Возвращает тип переменной name.
TYPE getTypeOfVariable(char* name);

//Возвращает размер в байтах переменной name (без учёта '\0' для типа STRING).
size_t getSizeOfVariable(char* name);

//Возвращает кол-во зарегестрированных переменных.
size_t getNumberOfVariables();

//Возвращает список переменных. Список содержит копии.
Variable* getVariableList();

//Удаляет переменную.
void deleteVariable(char* name);

//Очищает все данные, лежащие внутри.
void finishCVS();