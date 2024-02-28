/*Miguel Marcos Trillo   miguel.marcos.trillo@udc.es*/
/*Gael Garcia Arias      gael.garciaa@udc.es*/

#include "proc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>

#define STOPPED 0
#define RUNNING 1
#define TERMINATED 2
#define STERMINATED 3

char *signalname(int signal){
    if(signal==0)
        return "STOPPED";
    if(signal==1)
        return "RUNNING";
    if(signal==2)
        return "TERMINATED";
    if(signal==3)
        return "STERMINATED";
    return "UNKNOWN";
}

static struct SEN{
    char* nombre;
    int senal;
} sigstrnum[]={
        {"HUP", SIGHUP},
        {"INT", SIGINT},
        {"QUIT", SIGQUIT},
        {"ILL", SIGILL},
        {"TRAP", SIGTRAP},
        {"ABRT", SIGABRT},
        {"IOT", SIGIOT},
        {"BUS", SIGBUS},
        {"FPE", SIGFPE},
        {"KILL", SIGKILL},
        {"USR1", SIGUSR1},
        {"SEGV", SIGSEGV},
        {"USR2", SIGUSR2},
        {"PIPE", SIGPIPE},
        {"ALRM", SIGALRM},
        {"TERM", SIGTERM},
        {"CHLD", SIGCHLD},
        {"CONT", SIGCONT},
        {"STOP", SIGSTOP},
        {"TSTP", SIGTSTP},
        {"TTIN", SIGTTIN},
        {"TTOU", SIGTTOU},
        {"URG", SIGURG},
        {"XCPU", SIGXCPU},
        {"XFSZ", SIGXFSZ},
        {"VTALRM", SIGVTALRM},
        {"PROF", SIGPROF},
        {"WINCH", SIGWINCH},
        {"IO", SIGIO},
        {"SYS", SIGSYS},
/*senales que no hay en todas partes*/
#ifdef SIGPOLL
        {"POLL", SIGPOLL},
#endif
#ifdef SIGPWR
        {"PWR", SIGPWR},
#endif
#ifdef SIGEMT
        {"EMT", SIGEMT},
#endif
#ifdef SIGINFO
        {"INFO", SIGINFO},
#endif
#ifdef SIGSTKFLT
        {"STKFLT", SIGSTKFLT},
#endif
#ifdef SIGCLD
        {"CLD", SIGCLD},
#endif
#ifdef SIGLOST
        {"LOST", SIGLOST},
#endif
#ifdef SIGCANCEL
        {"CANCEL", SIGCANCEL},
#endif
#ifdef SIGTHAW
        {"THAW", SIGTHAW},
#endif
#ifdef SIGFREEZE
        {"FREEZE", SIGFREEZE},
#endif
#ifdef SIGLWP
        {"LWP", SIGLWP},
#endif
#ifdef SIGWAITING
        {"WAITING", SIGWAITING},
#endif
        {NULL,-1},
};    /*fin array sigstrnum */


int ValorSenal(char * sen)  /*devuelve el numero de senial a partir del nombre*/
{
    int i;
    for (i=0; sigstrnum[i].nombre!=NULL; i++)
        if (!strcmp(sen, sigstrnum[i].nombre))
            return sigstrnum[i].senal;
    return -1;
}


char *NombreSenal(int sen)  /*devuelve el nombre senal a partir de la senal*/
{			/* para sitios donde no hay sig2str*/
    int i;
    for (i=0; sigstrnum[i].nombre!=NULL; i++)
        if (sen==sigstrnum[i].senal)
            return sigstrnum[i].nombre;
    return ("SIGUNKNOWN");
}


void createEmptyListC(tListC *P){
    *P=NULL;
}

bool createNodeC(tPosC *p){
    *p=malloc(sizeof(struct tNodeC));
    return *p!=NULL;
}

tPosC buscarProceso(pid_t pid, tListC P){
    tPosC p;
    for(p=firstC(P);p!=NULL && p->data.pid!=pid;p=nextC(p,P));
    return p;
}

bool insertProcess(pid_t pid, char *time, char* cmd, int pri, tListC *P){
    tItemC item;
    item.pid = pid;
    item.time = time;
    setpriority(PRIO_PROCESS,pid,pri);
    item.cmdline = malloc(sizeof(strlen(cmd)) + 1);
    strcpy(item.cmdline, cmd);
    item.signalnum=RUNNING;
    item.signal=NULL;
    return insertItemC(item, P);
}

bool insertItemC(tItemC d, tListC *P){
    tPosC q,r;
    if(!createNodeC(&q)){
        return false;
    }else{
        q->data=d;
        q->next=NULL;
        if(*P==NULL){
            *P = q;
        }else {
            r = lastC(*P);
            r->next=q;
        }
        return true;
    }
}

tPosC nextC(tPosC p, tListC P){
    if(p!= NULL){
        return(p->next);
    }else{
        return NULL;
    }
}

tPosC firstC(tListC P){
    return P;
}

tPosC lastC(tListC P){
    tPosC q;
    for(q=P; q->next!=NULL;q=q->next);
    return q;
}

void deleteTerminatedProcess(tListC *P){
    tPosC p;
    for(p=firstC(*P);p!=NULL;p=nextC(p,*P)){
        updateProcess(&p->data,0);
        if(p->data.signalnum==TERMINATED)
            deleteAtPositionC(p,P);
    }
}

void deleteTerminatedSignProcess(tListC *P){
    tPosC p;
    for(p=firstC(*P);p!=NULL;p=nextC(p,*P)){
        updateProcess(&p->data,0);
        if(p->data.signalnum==STERMINATED)
            deleteAtPositionC(p,P);
    }
}

void deleteProcess(tPosC p, tListC *P){
    updateProcess(&p->data, 1);
    deleteAtPositionC(p, P);
}

void deleteAtPositionC(tPosC p, tListC *P){
    tPosC q;
    if (p == *P){
        *P = (*P)->next;
    }
    else if (p->next == NULL){
        for (q = *P; q->next->next != NULL; q = q->next);
        q->next = NULL;
    }
    else{
        q = p->next;
        p->data = q->data;
        p->next = q->next;
        p = q;
    }
    free(p->data.time);
    free(p->data.cmdline);
    if(p->data.signal!=NULL){
        free(p->data.signal);}
    free(p);
}

void deleteListC(tListC *P){
    while(firstC(*P)!=NULL){
        deleteProcess(firstC(*P), P);
    }
}

tItemC getItemC(tPosC p, tListC P){
    return (p->data);
}

void updateProcess(tItemC *item, int flag){
    int status;
    int wp;
    wp = waitpid(item->pid, &status, flag ? 0 : WNOHANG | WUNTRACED | WCONTINUED);
    if(wp==item->pid){
        if(item->signal==NULL){
            item->signal=malloc(sizeof(int));}
        if(WIFSIGNALED(status)){
            item->signalnum = STERMINATED;
            *item->signal = WTERMSIG(status);
        }
        else if(WIFEXITED(status)){
            item->signalnum = TERMINATED;
            *item->signal = WEXITSTATUS(status);
        }
        else if(WIFSTOPPED(status)){
            item->signalnum = STOPPED;
            *item->signal = WSTOPSIG(status);

        }
        else if(WIFCONTINUED(status)){
            item->signalnum = RUNNING;
            if(item->signal!=NULL) {
                free(item->signal);
                item->signal = NULL;
            }
        }
    }
}

void updateItem(tItemC item, tPosC p, tListC *P){
    p->data=item;
}

void mostrarProceso(tPosC p, tListC *P){
    tItemC item = p->data;
    updateProcess(&item,0);
    updateItem(item,p,P);
    char sig[50];
    if(item.signalnum==STOPPED || item.signalnum==STERMINATED){
        sprintf(sig," Señal: %s ", NombreSenal(*item.signal));}
    else if(item.signalnum==TERMINATED){
        sprintf(sig," Valor: %d ", *item.signal);}
    else{
        sprintf(sig," ");}
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    printf("%d\t%s p = %d %s %s (%s) %s",(int) item.pid, pw->pw_name, getpriority(PRIO_PROCESS,item.pid), item.time, signalname(item.signalnum),sig,item.cmdline);
    printf("\n");
}


void mostrarListaProcesos(tListC P){
    tPosC p;
    for(p=firstC(P);p!=NULL;p=nextC(p, P)){
        mostrarProceso(p, &P);
    }
}
