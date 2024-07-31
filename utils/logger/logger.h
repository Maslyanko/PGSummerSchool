#pragma once

#include <semaphore.h>

/*   
**    Flags required when creating a logger:
**    
**    LOG_APPEND: 
**     Add new logs to existing logs. 
**
**    LOG_CLEAR: 
**     Write logs to an empty file. The old logs will be deleted.
**
**    =======================================
**    | By default, LOG_APPEND is selected. |
**    =======================================
**
**    LOG_CYCLIC_WRITE: 
**     When the number of logs exceeds a predetermined capacity, the new logs 
**     will be written on top of the old ones.
**     ==========================================================================
**     | If you do not specify this flag, the recording cycle will be disabled. |
**     | The default capacity is 100.                                           |
**     ==========================================================================
*/

#define LOG_APPEND 1
#define LOG_CLEAR 2
#define LOG_CYCLIC_WRITE 4

typedef enum
{
    FATAL,
    ERROR,
    WARNING,
    INFO,
    DEBUG,
} LOG_LEVEL;


/*
**    ====================================
**    |'\n' is the end of line character.|
**    ====================================
*/

//Вспомогательные переменные.
extern sem_t* SEMAPHORE;
extern int NEW_LOGGER_FD;
extern int OLD_LOGGER_FD;
extern int LOGGER_CAPACITY;
extern int DEFAULT_CAPACITY;
extern int CURRENT_SIZE;
extern int IS_CYCLIC;
extern int PID_CREATOR;
extern char* FILE_PATH;
extern char* FRESH_LOGS_NAME;

//Основная функция. Думаю тут всё понятно
void elog(LOG_LEVEL level, const char* message);

//Инициализация логгера. 
//filePath - путь до файл с логами. 
//flags - режим работы логгера. 
//capacity - вместимость лог файла (сколько логов можно записать).
int initLogger(const char* filePath, int flags, int capacity);

//Функция необходимая для правильного завершения работы логгера.
int finishLogger();
