/*Miguel Marcos Trillo   miguel.marcos.trillo@udc.es*/
/*Gael Garcia Arias      gael.garciaa@udc.es*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/utsname.h>
#include "list.h"
#include <errno.h>
#define MAX 1024
#define MAXBUF 1024
#define MAXTROZOS 100



int TrocearCadena(char * cadena, char * trozos[]);  //troceamos la cadena y devuelve el nunmero de palabras de la cadena
void processEntry(char *tr[],tListP *L,int *terminado,int *total,char *cmd);  //procesa el comando y realiza el comando correspondiente

struct Fichero{
    int descriptor;
    int modo;
    char nombre[MAX];
};
struct Fichero TablaFicheros[MAX];

void OrdenarTabla(int total){
	for(int i = 0; i < (total - 1); i++){
		for(int j = i+1; j < total; j++){
			if(TablaFicheros[i].descriptor > TablaFicheros[j].descriptor){
				struct Fichero temporal = TablaFicheros[i];
				TablaFicheros[i] = TablaFicheros[j];
				TablaFicheros[j] = temporal;
			}	
		}
	}
}

void OpeningMode(int mode){
    if(mode & O_CREAT)
        printf("O_CREAT\n");
    else if(mode & O_EXCL)
        printf("O_EXCL\n");
    else if(mode & O_RDONLY)
        printf("O_RDONLY\n");
    else if(mode & O_WRONLY)
        printf("O_WRONLY\n");
    else if(mode & O_RDWR)
        printf("O_RDWR\n");
    else if(mode & O_APPEND)
        printf("O_APPEND\n");
    else if(mode & O_TRUNC)
        printf("O_TRUNC\n");
    else
        printf("\n");
}

int TrocearCadena(char * cadena, char * trozos[]){
    int i=1;

    if ((trozos[0]=strtok(cadena," \n\t"))==NULL)
        return 0;
    while ((trozos[i]=strtok(NULL," \n\t"))!=NULL)
        i++;
    return i;
}

void AnadirFicherosAbiertos(int df, int mode, const char *nombre,int *total){
    //añade el fichero con su descriptor modo y nombre y lo imprime por pantalla
    TablaFicheros[*total].descriptor=df;
    TablaFicheros[*total].modo=mode;
    strncpy(TablaFicheros[*total].nombre, nombre, sizeof(TablaFicheros[*total].nombre) - 1);
    printf("Anadida entrada %d a la tabla de ficheros abiertos \n",TablaFicheros[*total].descriptor);
    (*total)++;
}

void EliminarFicherosAbiertos(int df,int *total){
    for (int i = 0; i < (*total); i++) {
        if (TablaFicheros[i].descriptor == df) {
            //si el descriptor coincide con uno de la lista recorre desde esa posicion
            //hasta el final para borrarlo y que los elementos posteriores retrocedan una posicion
            printf("Eliminada la entrada %d de la tabla de ficheros abiertos \n",TablaFicheros[i].descriptor);
            for (int j = i; j < (*total) - 1; j++) {
                TablaFicheros[j] = TablaFicheros[j + 1];
            }
            (*total)--;
            break;
        }
    }
}

char *NombreFicheroDescriptor(int df){
    return TablaFicheros[df-1].nombre;  //obtenemos el nombre del fichero a duplicar
}

void Cmd_authors(char *tr[], char *cmd) {

    if(tr[1]==NULL){
        printf("Miguel Marcos Trillo: miguel.marcos.trillo@udc.es\nGael Garcia Arias: gael.garciaa@udc.es\n");
    }else if(strcmp(tr[1],"-l")==0){
       printf("miguel.marcos.trillo@udc.es\ngael.garciaa@udc.es\n");
   }else if(strcmp(tr[1],"-n")==0){
       printf("Miguel Marcos Trillo\nGael Garcia Arias\n");
   }else{
       fprintf(stderr, "%s %s \n", strerror(3), cmd);
   }
}

void Cmd_pid(char *tr[], char *cmd){
    if(tr[1] == NULL)
        printf("Pid de shell: %d\n",getpid());
    else if(strcmp(tr[1], "-p") == 0)
        printf("Pid del padre del shell: %d\n", getppid());
    else
        fprintf(stderr, "%s '%s' \n", strerror(3), cmd);
}

void Cmd_chdir(char *tr[],char *cmd) {
    char path[MAXBUF];
    if (tr[1] == NULL) {
        if(getcwd(path,MAXBUF)==NULL){  //si getcwd no puede guardar el directorio en path devuelve null
            perror("getcwd");
        }else{
        printf("Directorio actual %s\n",path);}
    }
    else if (tr[2]==NULL) {
        if (chdir(tr[1]) == -1) {
            printf("No se puede cambiar al directorio %s: %s\n", tr[1], strerror(errno));
            //Si no se pude cambiar de directorio, porque no existe o no se tienen permisos, muestra un error
        } else {
            chdir(tr[1]); //sino se cambia al directorio solicitado
            printf("Cambio del directorio actual %s\n", tr[1]);
        }

    }else{
        fprintf(stderr, "%s '%s' \n", strerror(3), cmd);
    }
}

void Cmd_date(char *tr[],char *cmd) {
   if(tr[1]==NULL){ time_t *t;
    struct tm *b;
    t= malloc(sizeof(time_t));
    time(t);
    b= localtime(t);
    printf("%d/%d/%d\n",b->tm_mday,b->tm_mon+1,b->tm_year+1900);
    free(t);}
   else{
       fprintf(stderr, "%s '%s' \n", strerror(3), cmd);
   }
}

void Cmd_time(char *tr[],char *cmd){
    if(tr[1]==NULL){time_t *t;
    struct tm *b;
    t= malloc(sizeof(time_t));
    time(t);
    b= localtime(t);
    printf("%d:%d:%d\n",b->tm_hour,b->tm_min,b->tm_sec);
    free(t);}
    else{
        fprintf(stderr, "%s '%s' \n", strerror(3), cmd);
    }
}

void Cmd_hist(char *tr[],tListP L,char *cmd){
    int n;
    if(tr[1]==NULL){
        n=0;
        for(tPos i= first(L);i!=NULL;i= nextP(i,L)){  //recorre la lista para mostrar todos los comandos de la lista
            printf("number %d: comand %s\n",n, getItemP(i,L));
            n++;
        }
    }else if(strcmp(tr[1],"-c")==0){
        deleteList(&L);}  //borra la lista con los comandos
    else if (*tr[1]=='-'){  //si se pone un numero se imprimira los n primeros comandos de la lista
            char *a=strtok(tr[1],"-");
            int z= atoi(a);
            if(z>=0){
            n=0;
                for(tPos i= first(L);n<z;i=nextP(i,L)){
                    if(i==NULL)
                        continue;
                    printf("number %d: comand %s\n",n, getItemP(i,L));
                    n++;

                }
            }else{
                fprintf(stderr,"Error : no existe ese elemento en la lista\n");
        }
    }else
        fprintf(stderr, "%s '%s' \n", strerror(3), cmd);
}

void Cmd_comand(char *tr[],tListP L,int terminado,int total,char *cmd){
    tPos p;
    char *trozos[MAX];
    int a = atoi(tr[1]),b;
    if(tr[1]==NULL){
        fprintf(stderr, "%s '%s' \n", strerror(3), cmd);
    }else{
       if(a >= 0 ){
           b=0;  //recorremos la lista hasta que el numero solicitado coincida con el elemento de la lista que tiene esa posicion asignada
           for(p=first(L);b<=a;p= nextP(p,L)){
               if(b==a){
                   printf("Comando %s\n", getItemP(p,L));
                   TrocearCadena(getItemP(p,L),trozos);  //vuelve a trocear la cadena para procesar ese comando
                   processEntry(trozos,&L,&terminado,&total,cmd);
                   break;

               }
               b++;
           }
       }
    }
}

void Cmd_listopen(int df,int *total){
    if(df!=-1){  //si el valor de distinto de -1 imprime los procesos padre junto con los ficheros añadadidos a la lista
        printf("Descriptor: 0 -> entrada estandar O_RDWR\nDescriptor: 1 -> salida estandar O_RDWR\nDescriptor: 2 -> error estandar O_RDWR\n");
    	OrdenarTabla(*total);
    for (int i = 0; i < *total; i++) {
        printf("Descriptor: %d -> %s ",TablaFicheros[i].descriptor, TablaFicheros[i].nombre);
        OpeningMode(TablaFicheros[i].modo);
        
    }}
    else{
        printf("No se puede listar los ficheros \n");
    }
}

void Cmd_open (char * tr[],int *total){
    int i, df, mode = 0;

    if (tr[1] == NULL) { /*no hay parametro*/
        Cmd_listopen(0,total);  //imprime la lista de ficheros añadidos
        return;
    }
    for (i = 2; tr[i] != NULL; i++)
        if (!strcmp(tr[i], "cr")) mode  |= O_CREAT;
        else if (!strcmp(tr[i], "ex")) mode |= O_EXCL;
        else if (!strcmp(tr[i], "ro")) mode |= O_RDONLY;
        else if (!strcmp(tr[i], "wo")) mode |= O_WRONLY;
        else if (!strcmp(tr[i], "rw")) mode |= O_RDWR;
        else if (!strcmp(tr[i], "ap")) mode |= O_APPEND;
        else if (!strcmp(tr[i], "tr")) mode |= O_TRUNC;
        else break;

    if ((df = open(tr[1], mode, 0777)) == -1)  //si no se puede abrir el fichero muestra error
        perror("Imposible abrir fichero");
    else {
        AnadirFicherosAbiertos(df,mode,tr[1],total);  //sino lo añade al historico

    }
}

void Cmd_close (char *tr[],int *total) {
    int df;
    if (tr[1] == NULL || (df = atoi(tr[1])) < 0) { /*no hay parametro*/
        Cmd_listopen(0,total);/*o el descriptor es menor que 0 ,muestra la lista de ficheros*/
        return;
    }
    if (close(df) == -1)  //si no se puede cerrar el fichero muestra error
        perror("Imposible cerrar descriptor");
    else {
        EliminarFicherosAbiertos(df,total);
        //sino elimina el fichero
    }
}

void Cmd_dup (char * tr[],int *total){
    int df, duplicado;
    char aux[MAX],*p;

    if (tr[1]==NULL || (df=atoi(tr[1]))<0) { /*no hay parametro*/
        Cmd_listopen(-1,total);                 /*o el descriptor es menor que 0,muestra la lista de ficheros*/
        return;
    }

    p=NombreFicheroDescriptor(*total);
   int mode= fcntl(df,F_GETFL);
    if((duplicado=dup(df))==-1){  //si no se puede duplicar muestra error
        printf("Imposible duplicar descriptor\n");
    }else {
        //sino obtiene el nombre del duplicado y lo añade a la lista
        sprintf(aux, "dup %d (%s)", df, p);
        AnadirFicherosAbiertos(duplicado, mode, aux, total);
    }
}


void Cmd_infosys(char *tr[],char *cmd){
    struct utsname uts;

    if (tr[1]==NULL){
        uname(&uts);  //obtiene informacion del sistema
        printf("Machine: %s\n", uts.machine);  //hardware de la maquina
        printf("Release: %s\n", uts.release);  //lanzamiento del kernel
        printf("Sysname: %s\n", uts.sysname);  //sistema operativo
        printf("Nodename: %s\n", uts.nodename);  //nombre del nodo
        printf("Version: %s\n", uts.version);  //version del kernel

    }else{
        fprintf(stderr, "%s '%s' \n", strerror(3), cmd);
    }
}

void Cmd_help(char *tr[],char *cmd){
    if(tr[1]==NULL){
        printf("Available commands: authors,pid,chdir,date,time,hist,comand N,open,close,dup,listopen,infosys,help,quit,bye,exit\n");
    }else if (strcmp(tr[1], "authors") == 0) {
            printf("Prints the names and logins of the program authors\n");
        } else if (strcmp(tr[1], "pid") == 0) {
            printf("Prints the pid of the process executing the shell\n");
        } else if (strcmp(tr[1], "chdir") == 0) {
            printf("Changes the current working directory\n");
        } else if (strcmp(tr[1], "date") == 0) {
            printf("Prints the current date\n");
        } else if (strcmp(tr[1], "time") == 0) {
            printf("Prints the current time \n");
        } else if (strcmp(tr[1], "hist") == 0) {
            printf("Shows/clears the historic of commands\n");
        } else if (strcmp(tr[1], "comand") == 0) {
            printf("Repeats command number N\n");
        } else if (strcmp(tr[1], "open") == 0) {
            printf("Opens a file and adds it \n");
        } else if (strcmp(tr[1], "close") == 0) {
            printf("Closes the df file descriptor and eliminates the corresponding item from\n"
                   "the list\n");
        } else if (strcmp(tr[1], "dup") == 0) {
            printf("Duplicates the df file descriptor\n");
        } else if (strcmp(tr[1], "listopen") == 0) {
            printf("Lists the shell open files\n");
        } else if (strcmp(tr[1], "infosys") == 0) {
            printf("rints information on the machine running the shell\n");
        } else if (strcmp(tr[1], "help") == 0) {
            printf("displays a list of available commands\n");
        } else if (strcmp(tr[1], "quit") == 0 || strcmp(tr[1], "exit") == 0 || strcmp(tr[1], "bye") == 0) {
            printf("Ends the shell\n");
        }else{
        fprintf(stderr, "%s '%s' \n", strerror(3), cmd);
        }
}

void Cmd_quit(char *tr[],int *terminado,tListP *L,char *cmd){
    if(tr[1]==NULL){
        *terminado=0;   //hacemos que acabe el bucle en el main y se acabe el programa y eliminamos la lista
        deleteList(L);
    }
    else
        fprintf(stderr, "%s '%s' \n", strerror(3), cmd);
}

void printPrompt(){
    printf(":~$ ");
}

void readEntry(char *cmd,char *tr[],char *cadena) {
    fgets(cadena, MAX, stdin);
    strcpy(cmd,cadena);
    strtok(cmd,"\n");  //el fgets siempre guarda una nueva linea por lo que la borramos
    TrocearCadena(cadena,tr);
}

void GuardaHist(char * entrada, char *tr[],tListP *L){
    if (tr[0]!=NULL){
        insertElement(entrada, L);
    }
}

void processEntry(char *tr[],tListP *L,int *terminado ,int *total,char *cmd){
    int n=0;
    if (tr[0] != NULL) {
        if (strcmp(tr[0], "authors") == 0) {
            Cmd_authors(tr,cmd);
        } else if (strcmp(tr[0], "pid") == 0) {
            Cmd_pid(tr,cmd);
        } else if (strcmp(tr[0], "chdir") == 0) {
            Cmd_chdir(tr,cmd);
        } else if (strcmp(tr[0], "date") == 0) {
            Cmd_date(tr,cmd);
        } else if (strcmp(tr[0], "time") == 0) {
            Cmd_time(tr,cmd);
        } else if (strcmp(tr[0], "hist") == 0) {
            Cmd_hist(tr,*L,cmd);
        } else if (strcmp(tr[0], "comand") == 0) {
            Cmd_comand(tr,*L,*terminado,*total,cmd);
        } else if (strcmp(tr[0], "open") == 0) {
            Cmd_open(tr,total);
        } else if (strcmp(tr[0], "close") == 0) {
            Cmd_close(tr,total);
        } else if (strcmp(tr[0], "dup") == 0) {
            Cmd_dup(tr,total);
        } else if (strcmp(tr[0], "listopen") == 0) {
            Cmd_listopen(n,total);
        }
        else if (strcmp(tr[0], "infosys") == 0) {
            Cmd_infosys(tr,cmd);
        } else if (strcmp(tr[0], "help") == 0) {
            Cmd_help(tr,cmd);
        } else if (strcmp(tr[0], "quit") == 0 || strcmp(tr[0], "bye") == 0 || strcmp(tr[0], "exit") == 0) {
            Cmd_quit(tr,terminado,L,cmd);
        }
        else{
            fprintf(stderr, "%s '%s' \n", strerror(3), cmd); }
    }
}

int main() {
char *tr[MAXTROZOS];
char cmd[MAX];
char cadena[MAX];
tListP L;
int terminado=1;
int total=0;
    CreateList(&L);
    while(terminado!=0) {
        printPrompt();
        readEntry(cmd,tr,cadena);
        GuardaHist(cmd, tr,&L);
        processEntry(tr,&L,&terminado,&total,cmd);

    }
    return 0;
}
