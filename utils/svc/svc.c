#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "../alloc/alloc.h"
#include "svc.h"
#include "../utils/hashtable.h"

HashTable* SVC_TABLE = NULL;

int registerVariable(char* name, TYPE type, void* value) {
    if (!SVC_TABLE) {
        SVC_TABLE = createHashTable(DEFAULT_HASHTABLE_SIZE, RESIZE_HASHTABLE_SCALE, HASHTABLE_BUCKET_CAPACITY);
    }

    if (getDataElement(SVC_TABLE, name)){
        return -1;
    }

    Variable *newValue = (Variable*)myMalloc(sizeof(Variable));
    newValue->name = (char*)myMalloc(strlen(name) + 1);
    strcpy(newValue->name, name);
    newValue->type = type;

    switch (type) {
    case INT:
        newValue->value = (int*)myMalloc(sizeof(int));
        *(int*)newValue->value = *(const int*)value;
        break;
    case STRING:
        newValue->value = (char*)myMalloc(strlen((const char*)value) + 1);
        strcpy((char*)newValue->value, (const char*)value);
        break;
    default:
        myFree(newValue);
        return -1;
    }

    insert(SVC_TABLE, newValue->name, newValue);

    return 0;
}

int changeVariable(char* name, void* newValue) {
    if (!SVC_TABLE) {
        return -1;
    }

    Variable* variable = getDataElement(SVC_TABLE, name);

    if (!variable) {
        return -1;
    }

    switch (variable->type) {
        case INT:
            *(int *)variable->value = *(const int *)newValue;
            break;
        case STRING:
            myFree(variable->value);
            variable->value = (char *)myMalloc(strlen((const char *)newValue) + 1);
            strcpy((char *)variable->value, (const char *)newValue);
            break;
        default:
            return -1;
    }

    return 0;
}

Variable* getVariable(char* name) {
    if (!SVC_TABLE) {
        return NULL;
    }

    Variable* variable = getDataElement(SVC_TABLE, name);

    if (!variable) {
        return NULL;
    }

    Variable* variableCopy = (Variable*) myMalloc(sizeof(Variable));

    switch (variable->type) {
        case INT:
            variableCopy->type = INT;

            variableCopy->value = (char*) myMalloc(sizeof(int));
            *(int*)variableCopy->value = *(int*)variable->value;

            break;
        case STRING:
            variableCopy->type = STRING;

            variableCopy->value = (char*) myMalloc(strlen((char*)variable->value) + 1);
            strcpy((char*)variableCopy->value, (char*)variable->value);

            break;
        default:
            myFree(variableCopy);
            return NULL;
    }

    variableCopy->name = (char*) myMalloc(strlen(variable->name) + 1);
    strcpy(variableCopy->name, variable->name);

    return variableCopy;
}

TYPE getTypeOfVariable(char *name) {
    if (!SVC_TABLE) {
        return -1;
    }

    Variable* variable = getDataElement(SVC_TABLE, name);

    if (!variable) {
        return -1;
    }

    return variable->type;
}

size_t getSizeOfVariable(char* name) {
    if (!SVC_TABLE) {
        return -1;
    }

    Variable *variable = getDataElement(SVC_TABLE, name);

    if (!variable) {
        return -1;
    }

    if (variable->type == STRING) {
        return strlen((char *)variable->value);
    }

    return sizeof(int);
}

size_t getNumberOfVariables() {
    return SVC_TABLE? SVC_TABLE->count: 0;
}

Variable* getVariableList() {
    if (!SVC_TABLE) {
        return NULL;
    }

    Variable* variables = (Variable*) myMalloc(SVC_TABLE->count * sizeof(Variable));
    HashTableEntry* entries = getAllElements(SVC_TABLE);

    for (int i = 0; i < SVC_TABLE->count; ++i) {
        variables[i].type = ((Variable*)entries[i].data)->type;
        variables[i].name = (char*) myMalloc(strlen(((Variable*)entries[i].data)->name) + 1);
        strcpy(variables[i].name, ((Variable*)entries[i].data)->name);

        switch (variables[i].type) {
            case INT:

                variables[i].value = myMalloc(sizeof(int));
                *(int*)variables[i].value = *(int*)((Variable*)entries[i].data)->value;

                break;
            case STRING:

                variables[i].value = (char*) myMalloc(strlen(((Variable*)entries[i].data)->value) + 1);
                strcpy(variables[i].value, ((Variable*)entries[i].data)->value);

                break;
            default:
                myFree(variables[i].name);
                myFree(variables);
                myFree(entries);
                return NULL;
        }
    }

    myFree(entries);
    
    return variables;
}

void deleteVariable(char *name) {
    if (!SVC_TABLE) {
        return;
    }

    removeElement(SVC_TABLE, name);
}

void finishCVS() {
    if (!SVC_TABLE) {
        return;
    }

    freeTable(SVC_TABLE);
}