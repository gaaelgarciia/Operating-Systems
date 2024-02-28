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
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <dirent.h>
#define MAX 1024
#define MAXBUF 1024
#define MAXTROZOS 100
#define MAXF 50000


//Declaramos estas funciones ya que son utilizadas por otras y no se puede mover su posicion en el codigo
//(por ejemplo Cmd_comand que utiliza processEntry, y esta debe de estar si o si al final)
void processEntry(char *tr[],tListP *L,int *terminado,int *total,char *cmd);  //procesa el comando y realiza el comando correspondiente
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
        printf("Available commands: exit bye quit date time pid authors hist command chdir infosys open close listopen dup help create stat list delete deltree\n");
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
    }else if(strcmp(tr[0],"create")==0){
        printf("Create a file or a directory\n");
    }else if(strcmp(tr[0],"stat")==0){
        printf("List files\n");
    }else if(strcmp(tr[0],"list")==0){
        printf("Directory contents list\n");
    }else if(strcmp(tr[0],"delete")==0){
        printf("Delete empty files or directories\n");
    }else if(strcmp(tr[0],"deltree")==0){
        printf("Delete non-empty files or directories recursively\n");
    }else{
        fprintf(stderr, "%s '%s' \n", strerror(3), cmd);
    }

}
void Cmd_quit(char *tr[],int *terminado,tListP *L,char *cmd){
    if(tr[1]==NULL){
        *terminado=0;   //hacemos que acabe el bucle en el main y se acabe el programa y eliminamos la lista
        deleteList(L);}
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
        } else if (strcmp(tr[0], "infosys") == 0) {
            Cmd_infosys(tr,cmd);
        } else if (strcmp(tr[0], "help") == 0) {
            Cmd_help(tr,cmd);
        } else if (strcmp(tr[0], "quit") == 0 || strcmp(tr[0], "bye") == 0 || strcmp(tr[0], "exit") == 0) {
            Cmd_quit(tr,terminado,L,cmd);
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
    inicio(&total);
    while(terminado!=0) {
        printPrompt();
        readEntry(cmd,tr,cadena);
        GuardaHist(cmd, tr,&L);
        processEntry(tr,&L,&terminado,&total,cmd);

    }
    return 0;
}