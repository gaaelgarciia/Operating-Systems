/*Miguel Marcos Trillo   miguel.marcos.trillo@udc.es*/
/*Gael Garcia Arias      gael.garciaa@udc.es*/


#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/utsname.h>
#include "list.h"
#include "mem.h"
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX 1024
#define MAXBUF 1024
#define MAXTROZOS 100
#define TAMANO 2048


int int_global=0;
char char_global='m';
double double_global=0.2;
int ni_int_global;
char ni_char_global;
double ni_double_global;

//Declaramos estas funciones ya que son utilizadas por otras y no se puede mover su posicion en el codigo
//(por ejemplo Cmd_comand que utiliza processEntry, y esta debe de estar si o si al final)
void processEntry(char *tr[],tListP *L,int *terminado,int *total,char *cmd,tListM *M);  //procesa el comando y realiza el comando correspondiente
void listardirect(char *tr,int lng,int link,int acc,int hid,int reca,int recb,char path[MAXBUF]);

struct Fichero{
    int descriptor;
    int modo;
    char nombre[MAX];
};

struct Fichero TablaFicheros[MAX];

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

int TrocearCadena(char * cadena, char * trozos[]){ //troceamos la cadena y devuelve el nunmero de palabras de la cadena
    int i=1;

    if ((trozos[0]=strtok(cadena," \n\t"))==NULL)
        return 0;
    while ((trozos[i]=strtok(NULL," \n\t"))!=NULL)
        i++;
    return i;
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
            fprintf(stderr,"Error : no existe ese elemento en la lista\n");        }


    }else
        fprintf(stderr, "%s '%s' \n", strerror(3), cmd);
}
void Cmd_comand(char *tr[],tListP L,int terminado,int total,char *cmd,tListM M){
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
                    processEntry(trozos,&L,&terminado,&total,cmd,&M);
                    break;

                }
                b++;
            }
        }
    }
}
void inicio(int *total) {
    const char a[MAX] = "entrada estandar ";
    const char b[MAX] = "salida estandar ";
    const char c[MAX] = "error estandar ";
    TablaFicheros[*total].descriptor=*total;
    TablaFicheros[*total].modo=O_RDWR;
    strncpy(TablaFicheros[*total].nombre, a, sizeof(TablaFicheros[*total].nombre) - 1);
    (*total)++;
    TablaFicheros[*total].descriptor=*total;
    TablaFicheros[*total].modo=O_RDWR;
    strncpy(TablaFicheros[*total].nombre, b, sizeof(TablaFicheros[*total].nombre) - 1);
    (*total)++;
    TablaFicheros[*total].descriptor=*total;
    TablaFicheros[*total].modo=O_RDWR;
    strncpy(TablaFicheros[*total].nombre, c, sizeof(TablaFicheros[*total].nombre) - 1);
    (*total)++;
}

void Cmd_listopen(int df,int *total){
    if(df!=-1){//si el valor de distinto de -1 imprime los procesos padre junto con los ficheros añadadidos a la lista
        OrdenarTabla(*total);
        for (int i = 0; i < *total; i++) {
            printf("Descriptor: %d -> %s ",TablaFicheros[i].descriptor, TablaFicheros[i].nombre);
            OpeningMode(TablaFicheros[i].modo);    }}
    else{
        printf("No se puede listar los ficheros \n");
    }
}


void AnadirFicherosAbiertos(int df, int mode, const char *nombre,int *total){
    //añade el fichero con su descriptor modo y nombre y lo imprime por pantalla
    TablaFicheros[*total].descriptor=df;
    TablaFicheros[*total].modo=mode;
    strncpy(TablaFicheros[*total].nombre, nombre, sizeof(TablaFicheros[*total].nombre) - 1);
    printf("Anadida entrada %d a la tabla de ficheros abiertos \n",TablaFicheros[*total].descriptor);
    (*total)++;
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
        AnadirFicherosAbiertos(df, mode,tr[1],total);  //sino lo añade al historico

    }
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
void Cmd_close (char *tr[],int *total) {
    int df;
    if (tr[1] == NULL || (df = atoi(tr[1])) < 0) { /*no hay parametro*/
        Cmd_listopen(0,total);/*o el descriptor es menor que 0 ,muestra la lista de ficheros*/
        return;
    }


    if (close(df) == -1)  //si no se puede cerrar el fichero muestra error
        perror("Imposible cerrar descriptor");
    else {
        EliminarFicherosAbiertos(df,total);  //sino elimina el fichero
    }
}

char *NombreFicheroDescriptor(int df){
    return TablaFicheros[df-1].nombre;  //obtenemos el nombre del fichero a duplicar
}



void Cmd_dup (char * tr[],int *total){
    int df, duplicado;
    char aux[MAX],*p;

    if (tr[1]==NULL || (df=atoi(tr[1]))<0) { /*no hay parametro*/
        Cmd_listopen(-1,total);                 /*o el descriptor es menor que 0,muestra la lista de ficheros*/
        return;
    }

    p=NombreFicheroDescriptor(*total);
    int mode = fcntl(df,F_GETFL);

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
        printf("Available commands: exit bye quit date time pid authors hist command chdir infosys open close listopen dup help create stat list delete deltree "
               "malloc shared mmap read write memdump memfill mem recurse\n");
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
    }else if(strcmp(tr[1],"create")==0){
        printf("Create a file or a directory\n");
    }else if(strcmp(tr[1],"stat")==0){
        printf("List files\n");
    }else if(strcmp(tr[1],"list")==0){
        printf("Directory contents list\n");
    }else if(strcmp(tr[1],"delete")==0){
        printf("Delete empty files or directories\n");
    }else if(strcmp(tr[1],"deltree")==0){
        printf("Delete non-empty files or directories recursively\n");
    }else if(strcmp(tr[1],"malloc")==0){
        printf("allocates (or deallocates) a block malloc memory. Updates the list of memory blocks\n");
    }else if(strcmp(tr[1],"shared")==0){
        printf("allocates (or deallocates) a block shared memory. Updates the list of memory blocks\n");
    }else if(strcmp(tr[1],"mmap")==0){
        printf("maps (or unmaps) a file in memory. Updates the list of memory blocks\n");
    }else if(strcmp(tr[1],"read")==0){
        printf("reads a file into memory\n");
    }else if(strcmp(tr[1],"write")==0){
        printf("writes to a file the contents at a memory address\n");
    }else if(strcmp(tr[1],"memdump")==0){
        printf("dumps the contents of memory to the screen\n");
    }else if(strcmp(tr[1],"memfill")==0){
        printf("fills the memory with one character\n");
    }else if(strcmp(tr[1],"mem")==0){
        printf("shows information on the memory of the process\n");
    }else if(strcmp(tr[1],"recurse")==0) {
        printf("executes a recursive function\n");
    }else{
        fprintf(stderr, "%s '%s' \n", strerror(3), cmd);
    }

}
void Cmd_quit(char *tr[],int *terminado,tListP *L,char *cmd, tListM *M){
    if(tr[1]==NULL){
        *terminado=0;   //hacemos que acabe el bucle en el main y se acabe el programa y eliminamos las dos listas
        deleteList(L);
        deleteListM(M);}
    else{
        fprintf(stderr, "%s '%s' \n", strerror(3), cmd);
    }
}



char LetraTF (mode_t m)
{
    switch (m&S_IFMT) { /*and bit a bit con los bits de formato,0170000 */
        case S_IFSOCK: return 's'; /*socket */
        case S_IFLNK: return 'l'; /*symbolic link*/
        case S_IFREG: return '-'; /* fichero normal*/
        case S_IFBLK: return 'b'; /*block device*/
        case S_IFDIR: return 'd'; /*directorio */
        case S_IFCHR: return 'c'; /*char device*/
        case S_IFIFO: return 'p'; /*pipe*/
        default: return '?'; /*desconocido, no deberia aparecer*/
    }
}
/*la siguiente funcione devuelve los permisos de un fichero en formato rwx----*/
/*a partir del campo st_mode de la estructura stat */



char * ConvierteModo2 (mode_t m)
{
    static char permisos[12];
    strcpy (permisos,"---------- ");

    permisos[0]=LetraTF(m);
    if (m&S_IRUSR) permisos[1]='r';    /*propietario*/
    if (m&S_IWUSR) permisos[2]='w';
    if (m&S_IXUSR) permisos[3]='x';
    if (m&S_IRGRP) permisos[4]='r';    /*grupo*/
    if (m&S_IWGRP) permisos[5]='w';
    if (m&S_IXGRP) permisos[6]='x';
    if (m&S_IROTH) permisos[7]='r';    /*resto*/
    if (m&S_IWOTH) permisos[8]='w';
    if (m&S_IXOTH) permisos[9]='x';
    if (m&S_ISUID) permisos[3]='s';    /*setuid, setgid y stickybit*/
    if (m&S_ISGID) permisos[6]='s';
    if (m&S_ISVTX) permisos[9]='t';

    return permisos;
}

void Directorio_Actual(){
    char path[MAXBUF];
    if(getcwd(path,MAXBUF)==NULL){  //si getcwd no puede guardar el directorio en path devuelve null
        perror("getcwd\n");
    }else{
        printf("Directorio actual %s\n",path);}
}

void formatoCorto(char* arch){
    struct stat s;
    if(lstat(arch,&s)==0){  //obtenemos informarcion del archivo o directorio para determinar si existe
        printf("%5lld %s\n",(long long) s.st_size, arch); //Muestra el archivo y el tamaño
    } else{
        perror("stat\n");
    }
}

void formatoLargo(char *arch,int link,int acc){
    struct stat s;
    struct passwd *p;
    struct group *g;
    ssize_t size, num;
    char *time,*usuario,*grupo;
    char symlink[MAX] = "";


    if(lstat(arch,&s)==0){ //Si el formato es largo
        if(acc==1){
            time=strtok(ctime(&s.st_atime),"\n"); //Fecha de acceso
        } else
            time= strtok(ctime(&s.st_mtime),"\n"); //Fecha de modificación
        usuario=((p= getpwuid(s.st_uid))==NULL? "Desconocido":p->pw_name);  //nombre propietario
        grupo=((g=getgrgid(s.st_gid))==NULL? "Desconocido": g->gr_name);  //nombre grupo
        printf("%s\t%ld (%ld) %5s %5s %10s %10lld  %s\n",time, (long )s.st_nlink, (long )s.st_ino, usuario, grupo, ConvierteModo2(s.st_mode),(long long ) s.st_size, arch); //Mostrar estadísticas

        if(link==1 && LetraTF(s.st_mode)=='l'){ //Comprueba si el archivo es enlace simbólico
            size = s.st_size+1;  // tamaño para guardar la ruta de destino del enlace
            num = readlink(arch, symlink, size);  //lee la ruta de destino y devuelve el numero de bytes leidos
            if(num >= 0){  //si los bytes son menores que 0 hay un error de lectura
                printf(" %s-> %s\n",arch, symlink);  //imprime el archivo junto con su ruta de destino del enlace simbólico
            }else {
                perror("Acceso no permitido al link\n");
            }
        }
    } else{
        perror("stat\n");
    }
}

void printDir(char **tot,int link,int lng,int acc,int hid,int p){
    char path[MAXBUF];
    getcwd(path,MAXBUF);
    printf("**********%s\n",path);  //imprime el directorio de trabajo
    for(int i = 0; i < p; i++){
        if(hid==1 || tot[i][0]!='.') {  //comprueba si hay archivos ocultos y los printea en el formato especificado
            if (lng == 1) {
                formatoLargo(tot[i], link, acc);
            } else {
                formatoCorto(tot[i]);
            }
        }
    }
}

void printRDir(char **tot,int link,int lng,int acc,int reca,int recb,int hid,int p){
    int x=0;
    char path[MAXBUF];
    getcwd(path,MAXBUF);
    struct stat s;
    while(x<p){
        if(lstat(tot[x],&s)==0){
            if(strcmp(tot[x],".")!=0 && strcmp(tot[x],"..")!=0 && LetraTF(s.st_mode)=='d')  //comprueba que no sea ni directorio actual ni padre y que sea propiamente un directorio
                listardirect(tot[x],lng,link,acc,hid,reca,recb,path);
        }
        x++;
    }
}
void listardirect(char *tr,int lng,int link,int acc,int hid,int reca,int recb,char path[MAXBUF]){
    int p=0;
    struct dirent *arch;
    char **tot = NULL; //utilizamos un puntero a un puntero para no tener problemas de que borre nombres previos
                        //al hacer realloc
    DIR *carpeta;
    carpeta= opendir(tr);  //abrimos el directorio y leemos su contenido

    if(carpeta != NULL) {  //si el directorio es no nulo
        if (chdir(tr)!=0){  //cambiamos el directorio de trabajo actual para el directorio pasado por parametro
            printf("Acceso no permitido al directorio %s: %s\n",tr,strerror(errno));
            return;
        }
        while ((arch = readdir(carpeta)) != NULL){ //mientras que haya mas archivos o subdirectorios
            char *name = arch->d_name;

            if(arch->d_name[0] == '.'){
                if(!hid)
                    continue;
            }

            if(p == 0)
                tot = (char **)malloc(sizeof(char *));
            else{
                char **temp = (char **)realloc(tot, (p + 1) * sizeof(char *));
                tot = temp;
            }

            tot[p] = (char *)malloc(strlen(name) + 1);
            strcpy(tot[p], name);
            p++;
        }

        if(reca && recb){ //si estan los dos lo hacemos como hace el shell de referencia
            printRDir(tot, link, lng, acc, reca, recb, hid, p);
            printDir(tot, link, lng, acc, hid, p);
        }
        else {
            if(recb){
                printRDir(tot, link, lng, acc, reca, recb, hid, p);
            }

            printDir(tot, link, lng, acc, hid, p);

            if(reca){
                printRDir(tot, link, lng, acc, reca, recb, hid, p);
            }
        }

        for(int i = 0; i < p; i++)
            free(tot[i]);
        free(tot);

        closedir(carpeta);  //cerramos el directorio
        chdir(path); //volvemos de nuevo al directorio de trabajo inicial
    }
    else{
        printf("Acceso no permitido al directorio %s: %s\n",tr, strerror(errno));
        return;
    }
}

void Cmd_create(char *tr[]){
    FILE *fp;
    if(tr[1]==NULL){
        Directorio_Actual();
    }else{
        if(strcmp(tr[1],"-f")==0){
            if(tr[2]==NULL){
                Directorio_Actual();
            }else{
                fp= fopen(tr[2],"w");  //creamos un archivo con permisos de escritura
                if(fp==NULL){
                    perror("Error al crear el archivo\n");
                }else{
                    fclose(fp);}
            }

        }else if(mkdir(tr[1],S_IRWXU)==-1){ //comprueba que se puede crear el directorio con permisos de escritura lectura y ejecucion del propietario
            printf("Creacion no permitida del directorio %s: %s\n",tr[1],strerror(errno));             }
    }
}

void Cmd_Stat(char *tr[]){
    int lng=0;int link=0;int acc=0;int i;int total=0;

    if(tr[1]==NULL){
        Directorio_Actual();
    }else{
        for(i=1;tr[i]!=NULL;i++){
            if(!(strcmp(tr[i],"-long"))){
                lng=1;
            }else if(!(strcmp(tr[i],"-link"))){
                link=1;
            }else if(!(strcmp(tr[i],"-acc"))){
                acc=1;
            }else{
                break;
            }
        }
        while(tr[i]!=NULL){
            if(lng==1){
                formatoLargo(tr[i],link,acc);
            }else{
                formatoCorto(tr[i]);
            }
            i++;
            total++;
        }
        if(total==0){
            Directorio_Actual();
        }

    }
}

void Cmd_list(char *tr[]) {
    int reca = 0, recb = 0, hid = 0, lng = 0, link = 0, acc = 0, i;
    char path[MAXBUF];
    int total=0;
    getcwd(path,MAXBUF);
    if (tr[1] == NULL) {
        printf("%s\n",path);
    } else {
        for (i = 1; tr[i] != NULL; i++) {
            if (!(strcmp(tr[i], "-long"))) {
                lng = 1;
            } else if (!(strcmp(tr[i], "-link"))) {
                link = 1;
            } else if (!(strcmp(tr[i], "-acc"))) {
                acc = 1;
            } else if (!(strcmp(tr[i], "-reca"))) {
                reca = 1;
            } else if (!(strcmp(tr[i], "-recb"))) {
                recb = 1;
            } else if (!(strcmp(tr[i], "-hid"))) {
                hid = 1;
            } else {
                break;
            }
        }
        while(tr[i]!=NULL) {
            listardirect(tr[i], lng, link, acc, hid, reca, recb, path);
            i++;
            total++;
        }
        if(total==0){
            Directorio_Actual();
        }
    }


}
void Cmd_delete(char *tr[]){
    struct stat s;
    if(tr[1]==NULL){
        Directorio_Actual();
    }else{
        for(int x=1;tr[x]!=NULL;x++){
            if(lstat(tr[x],&s)==0) {  //comprobamos que existe el archivo o directorio
                if (LetraTF(s.st_mode) == 'd') {  //comprobamos si es directorio
                    if (rmdir(tr[x]) == -1) {  //lo borramos y en caso de que no se puede imprimimos error
                        printf("Borrado no permitido de %s: %s\n", tr[x], strerror(errno));
                    }
                } else {
                    if (unlink(tr[x]) == -1)  //borramos el arhivo y en caso de que no se pueda imprimimos error
                        printf("Borrado no permitido de %s: %s\n", tr[x], strerror(errno));
                }
            }
            else{  //si se produce un error
                printf("Borrado no permitido de %s: %s\n", tr[x], strerror(errno));
            }

        }
    }
}
void DeleteRecDir(const char *tr,char path[MAXBUF]){
    DIR *carpeta;
    struct dirent *arch;
    struct stat s;
    char subdir[MAX/2];

    carpeta =opendir(tr); //accedemos al directorio

    if(carpeta == NULL){  //si no se puede abrir
        printf("Acceso no permitido al directorio %s: %s\n",tr, strerror(errno));
        return;
    }

    if (chdir(tr)!=0){  //cambia el directorio de trabajo actual en el directorio especificado
        printf("Acceso no permitido al directorio %s: %s\n",tr,strerror(errno));
        return;
    }

    while ((arch= readdir(carpeta))!=NULL){
        if(strcmp(arch->d_name,"./")!=0 && strcmp(arch->d_name,".")!=0 && strcmp(arch->d_name,"..")!=0){  //comprueba que no sea directorio padre ni raiz ni actual
            snprintf(subdir,sizeof (subdir),"%s/%s",tr,arch->d_name);  //guardamos el subdirectorio o archivo
            if(lstat(subdir,&s)==0) {  //obtenemos informacion para saber si existe
                if (LetraTF(s.st_mode) == 'd') {  //si es un directorio
                    DeleteRecDir(subdir,path);  //realizamos la recursividad
                } else {
                    if (unlink(subdir) == -1)   //elimina el archivo
                        printf("Eliminacion no permitida del fichero %s: %s\n", arch->d_name, strerror(errno));

                }
            }else{  //si se produce un error
                printf("Borrado no permitido de %s: %s\n",arch->d_name, strerror(errno));
            }
        }
    }

    closedir(carpeta);  //cerramos el directorio
    chdir(path); //volvemos al directorio de trabajo inicial

    if(rmdir(tr)==-1){  //elimina el propio directorio
        printf("Borrado no permitido de %s: %s\n",tr,strerror(errno));
    }
}
void Cmd_deltree(char *tr[]){
    struct stat s;
    char path[MAXBUF];
    getcwd(path,MAXBUF);
    if(tr[1]==NULL){
        printf("%s\n",path);
    }else{
        for(int x=1;tr[x]!=NULL;x++){
            if(lstat(tr[x],&s)==0) {
                if (LetraTF(s.st_mode) == 'd') {  //si es un directorio llamamos a la funcion para eliminar recursivamente
                    DeleteRecDir(tr[x],path);
                }else {  //si es un arhivo lo eliminamos
                    if (unlink(tr[x]) == -1) {
                        printf("Borrado no permitido de %s: %s\n", tr[x], strerror(errno));
                    }
                }
            }
            else{  //si se produce un error
                printf("Borrado no permitido de %s: %s\n", tr[x], strerror(errno));
            }
        }
    }
}

/* Función auxiliar que convierte una cadena en un void* */

void* cadtop(char *adress){
    void *p;
    sscanf(adress, "%p", &p);
    return p;
}


char* horaactual(){
    int byteEsc=0;
    time_t t;
    struct tm *tm;
    char* date = malloc (50);
    t= time(NULL);
    tm = localtime(&t);
    byteEsc=strftime(date, 50, "%b %d %H:%M", tm);
    if(byteEsc!=0)
        return date;
    else{
        printf("Error en la fecha\n");
        free(date);
        return "NULL";
    }
}

/* Funciones para asignar bloques de memoria */

void Allocate_Malloc(char *size,tListM *M){
    size_t tam;
    char *time;
    void * p;
    tam=(size_t) strtoul(size,NULL,10);
    if(tam == 0){
        printf ("No se asignan bloques de 0 bytes\n");
        return;
    }
    p=malloc(tam);
    time = horaactual();
    if(insertarNodoMalloc(p,tam,time, M))
        printf("%ld bytes asignados en %p\n", tam, p);
    else
        perror("No se puede hacer malloc\n");
}

void * ObtenerMemoriaShmget (key_t clave, size_t tam,tListM *M)
{
    void * p;
    int aux,id,flags=0777;
    struct shmid_ds s;
    char *time;

    if (tam)     /*tam distito de 0 indica crear */
        flags=flags | IPC_CREAT | IPC_EXCL;
    if (clave==IPC_PRIVATE)  /*no nos vale*/
    {errno=EINVAL; return NULL;}
    if ((id=shmget(clave, tam, flags))==-1)
        return (NULL);
    if ((p=shmat(id,NULL,0))==(void*) -1){
        aux=errno;
        if (tam)
            shmctl(id,IPC_RMID,NULL);
        errno=aux;
        return (NULL);
    }
    shmctl (id,IPC_STAT,&s);
    time=horaactual();
    insertarNodoShared(p, s.shm_segsz, time,clave,M);
    return (p);
}
void Allocate_Shared (char *tr[],tListM *M)
{
    key_t cl;
    size_t tam;
    void *p;

    if (tr[1]==NULL || tr[2]==NULL) {
        printf("*****Lista de bloques shared asignados al proceso %d\n",getpid());
        showListShared(*M);
        return;
    }

    cl=(key_t)  strtoul(tr[1],NULL,10);
    tam=(size_t) strtoul(tr[2],NULL,10);
    if (tam==0) {
        printf ("No se asignan bloques de 0 bytes\n");
        return;
    }
    if ((p=ObtenerMemoriaShmget(cl,tam,M))!=NULL)
        printf ("Asignados %lu bytes en %p\n",(unsigned long) tam, p);
    else
        printf ("Imposible asignar memoria compartida clave %lu:%s\n",(unsigned long) cl,strerror(errno));
}


void * MapearFichero (char * fichero, int protection,tListM *M)
{
    int df, map=MAP_PRIVATE,modo=O_RDONLY;
    struct stat s;
    void *p;
    char *time;

    if (protection&PROT_WRITE)
        modo=O_RDWR;
    if (stat(fichero,&s)==-1 || (df=open(fichero, modo))==-1)
        return NULL;
    if ((p=mmap (NULL,s.st_size, protection,map,df,0))==MAP_FAILED)
        return NULL;
    time=horaactual();
    insertarNodoMmap (p,s.st_size,time,df,fichero,M);
    return p;
}

void Allocate_Mmap(char *arg[],tListM *M)
{
    char *perm;
    void *p;
    int protection=0;

    if (arg[0]==NULL)
    {
        printf("*****Lista de bloques mmap asignados al proceso %d\n",getpid());
        showListMmap(*M);
        return;}
    if ((perm=arg[1])!=NULL && strlen(perm)<4) {
        if (strchr(perm,'r')!=NULL) protection|=PROT_READ;
        if (strchr(perm,'w')!=NULL) protection|=PROT_WRITE;
        if (strchr(perm,'x')!=NULL) protection|=PROT_EXEC;
    }
    if ((p=MapearFichero(arg[0],protection,M))==NULL)
        perror ("Imposible mapear fichero");
    else
        printf ("fichero %s mapeado en %p\n", arg[0], p);
}

/* Funciones para desasignar bloques de memoria */

void Deallocate_Mmap (char* file,tListM *M){
    tPosM p = buscarNodoMmap(file, *M);
    if (p == NULL){
        printf("Fichero %s no mapeado\n",file);
        return;
    }
    if(munmap(p->data.adress,p->data.size)==-1){
        printf("Error al borrar el Mmap %s:%s\n",file,strerror(errno));
        return;
    }
    deleteAtPositionM(p, M);
}

int do_DeallocateDelkey (key_t clave){  //funcion modificada para que devuelva -1 en caso de que no pueda liberar la memoria compartida asociada a una clave
    int id;
    if (clave==IPC_PRIVATE){
        printf ("delkey necesita clave_valida\n");
        return -1;
    }
    if ((id=shmget(clave,0,0666))==-1){
        perror ("shmget: imposible obtener memoria compartida");
        return -1;
    }
    if (shmctl(id,IPC_RMID,NULL)==-1){
        perror ("shmctl: imposible eliminar memoria compartida\n");
    return -1;}
    return 0;
}

void Deallocate_Shared_Delkey(char *tr,tListM *M){
    key_t clave = (key_t) strtoul(tr,NULL,10);
    if(do_DeallocateDelkey(clave)!=-1) {
        tPosM p = buscarNodoShared(clave, *M);
        if (p == NULL) {
            return;
        }
        shmdt(p->data.adress); //Desvincula el bloque de memoria compartida del espacio de direcciones del proceso
    }
}
void Deallocate_Shared_Free(char *tr,tListM *M){
    key_t clave= (key_t ) strtoul(tr,NULL,10);
    tPosM p= buscarNodoShared(clave,*M);
    if (p == NULL) {
        printf("No hay bloque de esa clave mapeado en el proceso\n");
        return;
    }
   tItemM item= getItemM(p,*M);
    if((shmget(item.key, 0, 0))!= -1){ //Verifica si la memoria compartida aosciada a la clave existe
        if(shmdt(item.adress)==-1){  //Desvincula el bloque de memoria compartida del espacio de direcciones del proceso
            printf("ERROR\n");
            return;
        }
    }
    printf("Bloque de memoria compartida en %p (key %d) ha sido eliminado\n", item.adress,item.key);
    deleteAtPositionM(p,M);
}

void Deallocate_Malloc(char *size,tListM *M){
    size_t tam;
    tam=(size_t) strtoul(size,NULL,10);
    if(tam==0){
        printf("No se designan bloques de 0 bytes \n");
        return;
    }else{
    tPosM pos = buscarNodoMalloc(tam, *M);
    if (pos == NULL){
        printf("No hay bloque de ese tamano asignado con malloc\n");
        return;
    }
    deleteAtPositionM(pos, M);}
}

void Cmd_Malloc(char *tr[],tListM *M){
    if(tr[1]==NULL){
        printf("*****Lista de bloques asignados malloc para el proceso %d\n",getpid());
        showListMalloc(*M);}
    else if(!strcmp(tr[1],"-free") ){
        if(tr[2]==NULL){
        printf("*****Lista de bloques asignados malloc para el proceso %d\n",getpid());
            showListMalloc(*M);
            }
        else
            Deallocate_Malloc(tr[2],M);

    }else
        Allocate_Malloc(tr[1],M);

}

void Cmd_Shared(char *tr[],tListM *M){
    if(tr[1]==NULL){
        printf("*****Lista de bloques asignados shared para el proceso %d\n",getpid());
        showListShared(*M);}
    else if(!strcmp(tr[1],"-create")){
        Allocate_Shared(tr+1,M);}
    else if(!strcmp(tr[1],"-free")){
        if(tr[2]==NULL){
            printf("*****Lista de bloques asignados shared para el proceso %d\n",getpid());
            showListShared(*M);
        }else{
        Deallocate_Shared_Free(tr[2],M);}}
    else if(!strcmp(tr[1],"-delkey")){
        if(tr[2]==NULL){
            printf("Delkey necesita clave_valida\n");
        }else {
            Deallocate_Shared_Delkey(tr[2],M);
        }
    }

}

void Cmd_Mmap(char *tr[],tListM *M){
    if(tr[1]==NULL){
        printf("*****Lista de bloques asignados mmap para el proceso %d\n",getpid());
        showListMmap(*M);}
    else if(!strcmp(tr[1],"-free")){
        if(tr[2]==NULL){printf("*****Lista de bloques asignados mmap para el proceso %d\n",getpid());
            showListMmap(*M);}
        else{
            Deallocate_Mmap(tr[2],M);
        }
    }else{
        Allocate_Mmap(tr+1,M);
    }
}

ssize_t LeerFichero (char *f, void *p, size_t cont)
{
    struct stat s;
    ssize_t  n;
    int df,aux;

    if (stat (f,&s)==-1 || (df=open(f,O_RDONLY))==-1)
        return -1;
    if (cont==-1)   /* si pasamos -1 como bytes a leer lo leemos entero*/
        cont=s.st_size;
    if ((n=read(df,p,cont))==-1){
        aux=errno;
        close(df);
        errno=aux;
        return -1;
    }
    close (df);
    return n;
}

void do_I_O_read (char *ar[])
{
    void *p;
    size_t cont=-1;
    ssize_t n;
    if (ar[1]==NULL || ar[2]==NULL || ar[3]==NULL){
        printf ("faltan parametros\n");
        return;
    }
    p=cadtop(ar[2]);  /*convertimos de cadena a puntero*/
    if (ar[3]!=NULL)
        cont=(size_t) atoll(ar[3]);

    if ((n=LeerFichero(ar[1],p,cont))==-1)
        perror ("Imposible leer fichero");
    else
        printf ("leidos %lld bytes de %s en %p\n",(long long) n,ar[1],p);
}

ssize_t EscribirFichero (char *f, void *p, size_t cont,int overwrite)
{
    ssize_t  n;
    int df,aux, flags=O_CREAT | O_EXCL | O_WRONLY;

    if (overwrite)
        flags=O_CREAT | O_WRONLY | O_TRUNC;

    if ((df=open(f,flags,0777))==-1)
        return -1;

    if ((n=write(df,p,cont))==-1){
        aux=errno;
        close(df);
        errno=aux;
        return -1;
    }
    close (df);
    return n;
}

void do_I_O_write(char* tr[]){
    int overwrite = 0;
    void* p;
    size_t cont = -1;
    ssize_t  n;

    if(tr[1]==NULL){
        printf("faltan parametros\n");
        return;
    }
    if(!strcmp (tr[1], "-o")){
        overwrite++;
    }
    if(tr[1+overwrite]==NULL || tr[2+overwrite]==NULL || tr[3+overwrite]==NULL){
        printf ("faltan parametros\n");
        return;
    }
    p=cadtop(tr[2+overwrite]);
    if(tr[3+overwrite]!=NULL)
        cont = (size_t) strtoul(tr[3+overwrite],NULL,10);

    if ((n=EscribirFichero(tr[1+overwrite],p,cont, overwrite))==-1)
        perror ("Imposible escribir fichero");
    else
        printf ("escritos %lld bytes de %p en %s\n",(long long) n,p, tr[1+overwrite]);
}

void Cmd_io (char *tr[]){

    if(!strcmp((tr[0]), "read")){
        do_I_O_read(tr);
        return;
    }
    if(!strcmp((tr[0]),"write")){
        do_I_O_write(tr);
        return;
    }

}
void variables(){
    int x=0; char y='m'; double z=0.2;
    static int m=0; static char n='m'; static double k=0.2;
    static int ni_int; static char ni_char; static double ni_double;
    printf("Variables locales: %25p, %25p, %25p\n", &x, &y, &z);
    printf("Variables globales:%25p, %25p, %25p\n",&int_global, &char_global,&double_global);
    printf("Var (N.I.) globales:%24p, %25p, %25p\n",&ni_int_global,&ni_char_global,&ni_double_global);
    printf("Variables estáticas:%24p, %25p, %25p\n", &m, &n, &k);
    printf("Var (N.I.) estáticas:%23p, %25p, %25p\n",&ni_int,&ni_char,&ni_double);
}
void direccion_funciones(){
    printf("Funciones programa: %24p, %25p, %25p\n", Cmd_list, processEntry, Cmd_quit);
    printf("Funciones librería: %24p, %25p, %25p\n",malloc,printf, strcmp);
}

void Do_pmap (void) /*sin argumentos*/
{ pid_t pid;       /*hace el pmap (o equivalente) del proceso actual*/
    char elpid[32];
    char *argv[4]={"pmap",elpid,NULL};

    sprintf (elpid,"%d", (int) getpid());
    if ((pid=fork())==-1){
        perror ("Imposible crear proceso");
        return;
    }
    if (pid==0){
        if (execvp(argv[0],argv)==-1)
            perror("cannot execute pmap (linux, solaris)");

        argv[0]="procstat"; argv[1]="vm"; argv[2]=elpid; argv[3]=NULL;
        if (execvp(argv[0],argv)==-1)/*No hay pmap, probamos procstat FreeBSD */
            perror("cannot execute procstat (FreeBSD)");

        argv[0]="procmap",argv[1]=elpid;argv[2]=NULL;
        if (execvp(argv[0],argv)==-1)  /*probamos procmap OpenBSD*/
            perror("cannot execute procmap (OpenBSD)");

        argv[0]="vmmap"; argv[1]="-interleave"; argv[2]=elpid;argv[3]=NULL;
        if (execvp(argv[0],argv)==-1) /*probamos vmmap Mac-OS*/
            perror("cannot execute vmmap (Mac-OS)");
        exit(1);
    }
    waitpid (pid,NULL,0);
}
void Memory_options(int blocks, int funcs, int vars, int pmap,tListM *M){
    if(vars){
        variables();}

    if(funcs){
        direccion_funciones();}

    if(blocks){
        printf("*****Lista de bloques asignados al proceso %d\n",getpid());
        showList(*M);
    }
    if(pmap){
        Do_pmap();}

}

void Cmd_mem(char *tr[],tListM *M){
    int blocks = 0, funcs = 0, vars = 0, pmap = 0;
    if(tr[1]==NULL){
        blocks = 1;
        funcs = 1;
        vars = 1;
        Memory_options(blocks, funcs, vars, pmap,M);
        return;
    }
    if(!strcmp(tr[1], "-blocks")){
        blocks = 1;
    }
    else if(!strcmp(tr[1], "-funcs")){
        funcs = 1;
    }
    else if(!strcmp(tr[1], "-vars")){
        vars = 1;

    }
    else if(!strcmp(tr[1], "-all")){
        blocks = 1;
        funcs = 1;
        vars = 1;
    }
    else if(!strcmp(tr[1], "-pmap")){
        pmap = 1;
    }else{
        printf("Opcion %s no contemplada\n",tr[1]);
    }
    Memory_options(blocks, funcs, vars, pmap, M);
}

void LlenarMemoria (void *p, size_t cont, unsigned char byte)
{
    unsigned char *arr=(unsigned char *) p;
    size_t i;


    for (i=0; i<cont;i++)
        arr[i]=byte;
}

void Cmd_memfill(char *tr[]){
    void *p;
    size_t size;
    int byte;
    if(tr[1] == NULL ){
        return;
    }
    p=cadtop(tr[1]);
    if(p == NULL){
        printf("Direccion de memoria no valida\n");
        return;
    }
    if(tr[2]==NULL){
        size=(size_t) 128; //en caso de que no se le pase un tamano se pondra por defecto 128
        byte = 'A'; // y el byte 'A'
    }
    else{
        size = (size_t) strtoul(tr[2],NULL,10);
        if(tr[3] == NULL)
            byte = 'A'; //en caso de que no se especifique el caracter se asignara 'A'
        else
            byte = atoi(tr[3]);
    }
    printf("Llenando %ld bytes de memoria con el byte %c(%02X) a partir de la direccion %p\n",size,byte, byte,p);
    LlenarMemoria(p,size,byte);

}
void Cmd_memdump(char *tr[]){
    int cont, i;
    void *p;
    unsigned char* byte;
    if(tr[1]==NULL){
        return;
    }
    p=cadtop(tr[1]);
    if(p == NULL){
        printf("Direccion de memoria no valida\n");
        return;
    }
    if(tr[2]==NULL) //en caso de que no se le pase un tamano de lectura se toma 100
        cont=25;
    else
        cont = (int) strtoul(tr[2],NULL,10);
    for(i = 0; i< cont; i++){
        byte = p+i;
        if(isprint(*byte)) //con esta funcion comprobamos que sea un espacio imprimible
            printf("%2c ",*byte);
        if((i+1) % 25 == 0  || i == cont-1){ //con este for lo que hacemos es que se imprima su codigo ascii en hexadecimal debajo de su representacion en caracter
            printf("\n");
            for(int j = i - 24; j <= i; j++){
                printf("%02X ",*(unsigned char*)(p + j));
            }
            printf("\n");
        }
    }
    printf("\n");
}

void Recursiva (int n)
{
    char automatico[TAMANO];
    static char estatico[TAMANO];

    printf ("parametro:%3d(%p) array %p, arr estatico %p\n",n,&n,automatico, estatico);

    if (n>0)
        Recursiva(n-1);
}
void Cmd_recurse(char *tr[]){
    if(tr[1]!=NULL){
        Recursiva(atoi(tr[1]));
    }
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
void processEntry(char *tr[],tListP *L,int *terminado ,int *total,char *cmd,tListM *M){
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
            Cmd_comand(tr,*L,*terminado,*total,cmd,*M);
        } else if (strcmp(tr[0], "open") == 0) {
            Cmd_open(tr,total);
        } else if (strcmp(tr[0], "close") == 0) {
            Cmd_close(tr,total);
        } else if (strcmp(tr[0], "dup") == 0) {
            Cmd_dup(tr,total);
        } else if (strcmp(tr[0], "listopen") == 0) {
            Cmd_listopen(n,total);
        } else if (strcmp(tr[0], "infosys") == 0) {
            Cmd_infosys(tr,cmd);
        } else if (strcmp(tr[0], "help") == 0) {
            Cmd_help(tr,cmd);
        } else if (strcmp(tr[0], "quit") == 0 || strcmp(tr[0], "bye") == 0 || strcmp(tr[0], "exit") == 0) {
            Cmd_quit(tr,terminado,L,cmd, M);
        }else if(strcmp(tr[0],"create")==0){
            Cmd_create(tr);
        }else if(strcmp(tr[0],"stat")==0){
            Cmd_Stat(tr);
        }else if(strcmp(tr[0],"list")==0){
            Cmd_list(tr);
        }else if(strcmp(tr[0],"delete")==0){
            Cmd_delete(tr);
        }else if(strcmp(tr[0],"deltree")==0){
            Cmd_deltree(tr);
        }else if(strcmp(tr[0],"malloc")==0){
            Cmd_Malloc(tr,M);
        }else if (strcmp(tr[0],"shared")==0){
            Cmd_Shared(tr,M);
        }else if(strcmp(tr[0],"mmap")==0){
            Cmd_Mmap(tr,M);
        }else if(strcmp(tr[0],"read")==0 || strcmp(tr[0],"write")==0){
            Cmd_io(tr);
        }else if(strcmp(tr[0],"memfill")==0){
            Cmd_memfill(tr);
        }else if(strcmp(tr[0],"memdump")==0){
            Cmd_memdump(tr);
        }else if(strcmp(tr[0],"mem")==0){
            Cmd_mem(tr,M);
        }else if(strcmp(tr[0],"recurse")==0){
            Cmd_recurse(tr);
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
    tListM M;
    int terminado=1;
    int total=0;
    CreateList(&L);
    createEmptyListM(&M);
    inicio(&total);
    while(terminado!=0) {
        printPrompt();
        readEntry(cmd,tr,cadena);
        GuardaHist(cmd, tr,&L);
        processEntry(tr,&L,&terminado,&total,cmd,&M);

    }
    return 0;
}
