/*Miguel Marcos Trillo   miguel.marcos.trillo@udc.es*/
/*Gael Garcia Arias      gael.garciaa@udc.es*/

#ifndef LIST_H_
#define LIST_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define LNULL NULL

typedef char *tItemL;

typedef struct tNode *tPos;

struct tNode{
    tItemL str;
    tPos next;
};

typedef tPos tListP;

void CreateList(tListP *L);
bool createNode(tPos *p);
bool insertElement(tItemL d, tListP *L);
tItemL getItemP(tPos, tListP L);
tPos first(tListP L);
tPos last(tListP L);
tPos nextP(tPos p, tListP L);
void RemoveElement(tPos p, tListP *L);
void deleteList(tListP *L);

#endif

