/*Miguel Marcos Trillo   miguel.marcos.trillo@udc.es*/
/*Gael Garcia Arias      gael.garciaa@udc.es*/

#ifndef S0P1_MEM_H
#define S0P1_MEM_H
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

typedef struct tNodeM* tPosM;

typedef struct tItemM{
    void* adress;
    size_t size;
    char* time;
    char* type;
    key_t key;
    char* name;
    int df;
}tItemM;

struct tNodeM{
    tItemM data;
    tPosM next;
};

typedef tPosM tListM;

void createEmptyListM (tListM *M);

bool createNodeM(tPosM *p);

tPosM buscarNodoMalloc(size_t size, tListM M);

tPosM buscarNodoShared(key_t clave, tListM M);

tPosM buscarNodoMmap(char* file, tListM M);

bool insertarNodoMalloc(void *p, size_t size, char *time, tListM *M);

bool insertarNodoShared(void *p, size_t size, char* time, key_t clave, tListM *M);

bool insertarNodoMmap(void *p, size_t size, char* time, int df, char* fichero, tListM *M);

bool insertItemM(tItemM d, tListM *M);

void showList(tListM M);

void showListMalloc(tListM M);

void showListShared(tListM M);

void showListMmap(tListM M);

void deleteListM(tListM *M);

void deleteAtPositionM(tPosM p, tListM *M);

tPosM lastM(tListM M);

tPosM firstM(tListM M);

tPosM nextM(tPosM p, tListM M);

tItemM getItemM(tPosM p, tListM M);


#endif //S0P1_MEM_H
