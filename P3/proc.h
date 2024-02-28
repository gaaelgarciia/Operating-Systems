/*Miguel Marcos Trillo   miguel.marcos.trillo@udc.es*/
/*Gael Garcia Arias      gael.garciaa@udc.es*/

#ifndef S0P1_PROC_H
#define S0P1_PROC_H
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

typedef struct tNodeC* tPosC;

typedef struct tItemC{
    pid_t pid;
    char* cmdline;
    char* time;
    int *signal;
    int signalnum;
    int* priority;
}tItemC;

struct tNodeC{
    tItemC  data;
    tPosC next;
};

typedef tPosC tListC;

void createEmptyListC(tListC *P);

bool createNodeC(tPosC *p);

bool insertProcess(pid_t pid, char *time, char* cmd, int pri, tListC *P);

bool insertItemC(tItemC d, tListC *P);

void showListC(tListC P);

void deleteListC(tListC *P);

void deleteAtPositionC(tPosC p, tListC *P);

void deleteTerminatedProcess(tListC *P);

void deleteTerminatedSignProcess(tListC *P);

void deleteProcess(tPosC p, tListC *P);

void updateProcess(tItemC *item, int flag);

void updateList(tListC *P);

tPosC buscarProceso(pid_t pid, tListC P);

tPosC lastC(tListC P);

tPosC firstC(tListC P);

tPosC nextC(tPosC p, tListC P);

tItemC getItemC(tPosC p, tListC P);

void mostrarProceso(tPosC p, tListC *P);

void mostrarListaProcesos(tListC P);
#endif //S0P1_PROC_H
