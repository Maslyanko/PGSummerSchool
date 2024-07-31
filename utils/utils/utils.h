#pragma once 

#define FILE_TYPE 1
#define DIRECTORY_TYPE 2

//Просто штука, которая позволяет из нескольких наименований директорий/файлов для создания полного пути до файла/директории.
//number - кол-во строк.
//type - тип, который может быть либо файлом (FILE_TYPE), либо директорией (DIRECTORY_TYPE).
char* makeFullName(int number, int type, ...);