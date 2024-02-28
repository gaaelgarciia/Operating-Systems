/*Miguel Marcos Trillo   miguel.marcos.trillo@udc.es*/
/*Gael Garcia Arias      gael.garciaa@udc.es*/

#include "list.h"

void CreateList(tListP *L){
    *L = LNULL;
}

bool createNode(tPos *p){
    *p = malloc(sizeof(struct tNode));
    return (*p) != LNULL;
}

bool insertElement(tItemL d, tListP *L){
    tPos q, r;

    if(!createNode(&q)){
        return false;
    }
    else{
        q->str = malloc((strlen(d) + 1) * sizeof (char));
        if(!q->str){
            free(q);
            return false;
        }
        strcpy(q->str, d);
        q->next = LNULL;
        if(*L == LNULL){
            *L = q;
        }

        else{
            r = last(*L);
            r->next = q;
        }
        return true;
    }
}

tItemL getItemP(tPos p, tListP L){
    return p->str;
}

tPos first(tListP L){
    return L;
}

tPos last(tListP L){
    tPos p;
    for(p = L; p->next != LNULL; p = p->next);
    return p;
}

tPos nextP(tPos p, tListP L){
    return p->next;
}

void RemoveElement(tPos p, tListP *L){
    *L =p->next;
    free(p->str);
    free(p);
}

void deleteList(tListP *L){
    while (first(*L) != NULL) {
        RemoveElement(first(*L), L);
    }
}

