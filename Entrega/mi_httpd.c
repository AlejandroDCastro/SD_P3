//  Proyecto Servidor HTTP para Sistemas Distribuidos
//	Compilación: gcc mi_httpd.c -o mi_httpd -D DEBUG

/*
*/

#include <stdio.h>	
#include <errno.h>  
#include <stdlib.h>	
#include <string.h>	
#include <ctype.h>	
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h> 
#include <fcntl.h> 

#define SERWEB_VERSION_NAME                 "mínima"


#define MAX_REQUEST_SIZE                    40480
#define MAX_PATH_SIZE                       2560
#define BUFSIZE                             80960

// Mis herramientas de log
#ifdef DEBUG
#define log(fmt, args...)   fprintf(stderr, fmt "\n", ## args)
#else
#define log(fmt, args...)
#endif

void LanzarServicio();
int AtenderProtocoloHTTP(int);
int ObtenerSocket();
int ConectarConCliente();
void rellenarCamposConf(FILE*, char*, int, char*, int);
void error500(char[],char[]);
void error403(char[],char[]);
void error404(char[],char[]);
void error405(char[],char[]);
void error505(char[],char[]);
void mensaje200(char[],char[]);
void mensaje200Head(char[],char[]);
void error201(char[],char[]);

char *metodo, *version,*document,*document_root, *uri;
int topeclientes=10;
int puerto=8080;
FILE *archivo;

int main(int argc, char *argv[])
{
    
    
    
    unsigned int long_dir_cliente;
    
    
    
    FILE *archivoconf;
    
    document_root=malloc(1024);
    strcpy(document_root,"./sd");
    uri=malloc(1024);
    strcpy(uri,"/index.html");

    //Leemos el archivo .conf
    if(argc>1){

        if(strcmp(argv[1],"-c")==0){
            archivoconf = fopen(argv[2],"r"); //Leemos el archivo 
            if(archivoconf==NULL){
                printf("No existe el archivo .conf");
            }else{
                rellenarCamposConf(archivoconf,document_root,topeclientes,uri,puerto);
            }
        }
    }


    LanzarServicio();
    return 1;
}   /* de main */


void error500(char date[],char respuesta[]) {
    fprintf(stderr, "Error de lectura del mensaje\n\r");
    strcat(respuesta,"HTTP/1.1 500 Internal Server Error\n");
    strcat(respuesta, "Connection: close\n\r");
    strcat(respuesta, "Content-Length: 77");
    strcat(respuesta, "\n\r");
    strcat(respuesta, "Content-Type: txt/html\n\r");
    strcat(respuesta, "Server: Servidor SD\n\r");
    strcat(respuesta, "Date: ");
    strcat(respuesta, date);
    strcat(respuesta, "\n\r");
    strcat(respuesta, "Cache-control: max-age=0, no-cache\n\r");
    strcat(respuesta, "\n\r");
    strcat(respuesta, "<html> <title>Error 500</title>\n<h1> Error 500: Error Interno. </h1> </html>");
}


void error403(char date[],char respuesta[]) {
    strcat(respuesta,"HTTP/1.1 403 Forbidden\r\n");
    
    strcat(respuesta, "Content-Length: 81");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "Connection: close\r\n");
    strcat(respuesta, "Content-Type: txt/html\r\n");
    strcat(respuesta, "Server: Servidor SD\r\n");
    strcat(respuesta, "Date: ");
    strcat(respuesta, date);
    strcat(respuesta, "\r\n");
    strcat(respuesta, "Cache-control: max-age=0, no-cache");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "<html> <title>Error 403</title>\n<h1> Error 403: Acceso Denegado. </h1> </html>");
    strcat(respuesta, "\r\n");
    
    printf("Ha entrado en el error 403 Forbidden...\n");
}


void mensaje201(char date[],char respuesta[]) {
    strcat(respuesta, "HTTP/1.1 201 CREATED\r\n");

    strcat(respuesta, "Content-Length: 67");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "Connection: close\r\n");
    strcat(respuesta, "Content-Type: txt/html\r\n");
    strcat(respuesta, "Server: Servidor SD\r\n");
    strcat(respuesta, "Date: ");
    strcat(respuesta, date);
    strcat(respuesta, "\r\n");
    strcat(respuesta, "Cache-control: max-age=0, no-cache");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "\r\n"); 
    strcat(respuesta, "<html> <title>201 Created</title>\n<h1> File created.</h1> </html>");
    strcat(respuesta, "\r\n"); 

    printf("Ha entrado en el 201 Created...\n");
}


void error405(char date[],char respuesta[]) {
    strcat(respuesta, "HTTP/1.1 405 method not allowed\r\n");
    
    strcat(respuesta, "Content-Length: 120");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "Connection: close\r\n");
    strcat(respuesta, "Content-Type: txt/html\r\n");
    strcat(respuesta, "Server: Servidor SD\r\n");
    strcat(respuesta, "Date: ");
    strcat(respuesta, date);
    
    strcat(respuesta, "Cache-control: max-age=0, no-cache");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "\r\n"); 
    strcat(respuesta,"<html> <title>Error 405</title>\n<h1> Error 405: Método no permitido. </h1> </html>");
      
    printf("Entra en el error 405 method not allowed...\n");
}


void error404(char date[],char respuesta[]) {
    strcat(respuesta, "HTTP/1.1 404 not found\r\n");
    
    strcat(respuesta, "Content-Length: 96");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "Connection: close\r\n");
    strcat(respuesta, "Content-Type: txt/html\r\n");
    strcat(respuesta, "Server: Servidor SD\r\n");
    strcat(respuesta, "Date: ");
    strcat(respuesta, date);
    strcat(respuesta, "\r\n");
    strcat(respuesta, "Cache-control: max-age=0, no-cache");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "<html> <title>Error 404</title>\n<h1> 404 ERROR: File not found </h1> </html>");
    strcat(respuesta, "\n\r");

    printf("Ha entrado al error 404...\n");
}


void error505(char date[],char respuesta[]) {
    strcat(respuesta, "HTTP/1.1 505, HTTP version not supported\r\n");          
    
    strcat(respuesta, "Content-Length: 90");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "Connection: close\r\n");
    strcat(respuesta, "Content-Type: txt/html\r\n");
    strcat(respuesta, "Server: Servidor SD\r\n");
    strcat(respuesta, "Date: ");
    strcat(respuesta, date);
    strcat(respuesta, "\r\n");
    strcat(respuesta, "Cache-control: max-age=0, no-cache");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "\r\n");                  
    strcat(respuesta, "<html> <title>Error 500</title>\n<h1> Error 505: Versión de HTTP no soportada </h1> </html>");
}


void mensaje200(char date[],char respuesta[]) {
    strcat(respuesta, "HTTP/1.1 200 OK\r\n");
    
    strcat(respuesta, "Content-Length: 106");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "Connection: close\r\n");
    strcat(respuesta, "Content-Type: txt/html\r\n");
    strcat(respuesta, "Server: Servidor SD\r\n");
    strcat(respuesta, "Date: ");
    strcat(respuesta, date);
    strcat(respuesta, "\r\n");
    strcat(respuesta, "Cache-control: max-age=0, no-cache");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "\r\n");
    
    char* linea;
    size_t length=0;
    ssize_t read;

    //Bucle hasta final de archivo
    while(read=getline(&linea,&length,archivo)!=-1){
        strcat(respuesta,linea);
    }

    
    if(linea){
        free(linea);
    }
    strcat(respuesta,"\r\n");
    printf("Ha entrado en el 200 OK...\n");
}

void mensaje200Head(char date[],char respuesta[]) {
    strcat(respuesta, "HTTP/1.1 200 OK\r\n");
    
    strcat(respuesta, "Content-Length: 90");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "Connection: close\r\n");
    strcat(respuesta, "Content-Type: txt/html\r\n");
    strcat(respuesta, "Server: Servidor SD\r\n");
    strcat(respuesta, "Date: ");
    strcat(respuesta, date);
    strcat(respuesta, "\r\n");
    strcat(respuesta, "Cache-control: max-age=0, no-cache");
    strcat(respuesta, "\r\n");
    strcat(respuesta, "\r\n");
    
    strcat(respuesta,"\r\n");
    printf("Ha entrado en el 200 OK...\n");
}

//Rellenamos los valores leyendo el archivo.conf
void rellenarCamposConf(FILE* archivoconf, char* document_root, int topeclientes, char* uri, int puerto){

    char* linea;
    size_t length=0;
    ssize_t read;

    //Bucle hasta final de archivo
    while(read=getline(&linea,&length,archivoconf)!=-1){
        char *aux;

        aux = strtok(linea," ");
        if(strcmp(aux,"DocumentRoot")==0){
            aux = strtok(NULL,"\n");
            strcpy(document_root,aux);
        }else if(strcmp(aux,"DirectoryIndex")==0){
            aux = strtok(NULL,"\n");
            strcpy(uri,aux);
        }else if(strcmp(aux,"MaxClients")==0){
            aux = strtok(NULL,"\n");
            topeclientes = atoi(aux);
        }else if(strcmp(aux,"Listen")==0){
            aux = strtok(NULL,"\n");
            puerto = atoi(aux);
        }
    }

    fclose(archivoconf);
    if(linea){
        free(linea);
    }

}

int AtenderProtocoloHTTP(int sd)
{
    char buffer[MAX_REQUEST_SIZE] = "\0"; // Aqui esta almacenado el mensaje que envia el cliente
    long bLeidos = 0;
    int  result;
    char *metodo, *protocolo, *recurso;
    char respuesta[1024]; //Aqui almacenamos lo que devolvemos al cliente
    char date[80], tamanyo[100];
    int n, enviados, recibidos,size;
    int proceso,i;
    time_t tiempo;
    struct tm *tm1; 
    struct stat file_stats;
    long tambody;
    char body[MAX_REQUEST_SIZE];

    // HTTP REQUEST: leemos la cabecera (debe ser siempre menor que MAX_REQUEST_SIZE)
    bLeidos = read(sd, buffer, sizeof(buffer)-1);


    //No ha leido nada
    if(bLeidos < 0){
        error500(date,respuesta);
        n = strlen(respuesta); //calcula el numero de caracteres 
        enviados = write(sd, respuesta, n); //escribimos la respuesta
    }

    buffer[bLeidos] = '\0';
    log("Recibidos %ld bytes: \n%s",bLeidos,buffer);

    // Tratar la solicitude
    for(i=0; i<bLeidos-3;i++){
        if(buffer[i]=='\r' && buffer[i+1]=='\n' && buffer[i+2]=='\r' && buffer[i+3]=='\n'){
            tambody = bLeidos-(i+4);
            memcpy(body,&buffer[i+4],tambody);
            //break;
        }
    }
    printf("--------------------------\n");
    printf("%s\n",body);
    printf("--------------------------\n");
    metodo = strtok(buffer, " ");
    uri = strtok(NULL," ");    
    protocolo = strtok(NULL, "\n\r");
    
    tiempo = time(NULL);
    tm1 = localtime(&tiempo);
    strftime(date, 80, "%H:%M:%S, %A de %B de %Y", tm1);





    if(strcmp(metodo,"GET")==0){
        //Es un GET
        strcat(document_root,uri);
        archivo = fopen(document_root,"r");
        if(strcmp(protocolo,"HTTP/1.1")==0){
            

            //No encontramos el archivo
            if(archivo==NULL){
                error404(date,respuesta);
            }else{
                mensaje200(date,respuesta);
                printf("Ha entrado en el 200 OK...\n");
            }
            
        }else{
            //Version no soportada
            error505(date,respuesta);
        }
        
    }else if(strcmp(metodo,"HEAD")==0){
        //Es un HEAD

        strcat(document_root,uri);
        archivo = fopen(document_root,"r");
        if(strcmp(protocolo,"HTTP/1.1")==0){
            

            //No encontramos el archivo
            if(archivo==NULL){
                error404(date,respuesta);

            }else{
                mensaje200Head(date,respuesta);
            }
        }else{
            //Version no soportada
            error505(date,respuesta);
        }
        
    }else if(strcmp(metodo,"DELETE")==0){
        //Es un DELETE
        int aux = -1;
        
        strcat(document_root,uri);


        if(strcmp(protocolo,"HTTP/1.1")==0){
            aux = remove(document_root);

            if(aux!=0){
                error404(date,respuesta);
            }else{
                mensaje200(date,respuesta);
            }
        }else{
            //Version no soportada
            error505(date,respuesta);
        }
        
    }else if(strcmp(metodo,"PUT")==0){
        strcat(document_root,uri);

        int fd;
        fd = open(document_root, O_CREAT | O_WRONLY);
        
        


        if(strcmp(protocolo,"HTTP/1.1")==0){
            

            if(fd==-1){
                error403(date,respuesta);
            }else{
                mensaje201(date,respuesta);

                write(fd,body,tambody);
                strcpy(buffer,respuesta);
            }
        }else{
            //Version no soportada
            error505(date,respuesta);
        }

        
    }else{ 
        error405(date,respuesta);
    }


    
    printf("%s\n",respuesta);
    n = strlen(respuesta);
    enviados = write(sd,respuesta,n);

    return enviados;
}   /* de AtenderProtocoloHTTP */


int ObtenerSocket()
{
    int                 sfdServidor = -1;   // socket para esperar peticiones
    struct sockaddr_in  addr;
    int                 error;

    // SOCKET: Abrimos un socket donde esperar peticiones de servicio
    log("SOCKET: abrmos un socket...");
    sfdServidor = socket(AF_INET, SOCK_STREAM, 0);
    if(sfdServidor==-1){
        fprintf(stderr, "ERROR [%s] abriendo socket\n",strerror(errno));
        return -1;
    }
    // BIND: Asociamos el puerto en el que deseamos esperar con el socket obtenido
    log("BIND: asociando el socket con el puerto...");
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_port=htons((u_short)puerto);

    error = bind(sfdServidor, (struct sockaddr *)&addr, sizeof(addr));
    if(error < 0){
        fprintf(stderr, "ERROR [%s] en bind\n",strerror(errno));
        close(sfdServidor);
        return -1;
    }
    // LISTEN: Nos ponemos a la escucha en el socket, indicando cuántas atendemos a la vez
    log("LISTEN: indicamos que atienda hasta %d conexiones a la vez", topeclientes);
    error = listen(sfdServidor, topeclientes);
    if(error < 0){
        fprintf(stderr, "ERROR [%s] en listen\n",strerror(errno));
        close(sfdServidor);
        return -1;
    }

    return sfdServidor;
}   // de ObtenerSocket


int ConectarConCliente(int sfdServidor)
{
    int                 sfdCliente = -1;    // socket del hijo. Atiende cada nueva petición
    struct sockaddr_in  cliente;
           socklen_t    tamCliente = sizeof (cliente);
    struct hostent      *host;
    // ACCEPT: Esperamos hasta que llegue una comunicación, para la que nos dan un nuevo socket
    log("ACCEPT: esperando nueva conexion...");
    sfdCliente = accept(sfdServidor, (struct sockaddr *)&cliente, &tamCliente);

    log("Comprobando conexion entrante (resolucion inversa de la IP)");
    host = gethostbyaddr((char *)&cliente.sin_addr, sizeof(cliente.sin_addr),cliente.sin_family);    
    log("Conexion aceptada a [%s] %s: %u",host->h_name, inet_ntoa(cliente.sin_addr),cliente.sin_port);

    return sfdCliente;
}   // de ConectarConCliente

void LanzarServicio(){
    int sfdServidor = -1;   // socket del padre. Escucha nuevas peticiones
    int sfdCliente = -1;    // socket del hijo. Atiende cada nueva petición
    int result;

    //Abrimos un socket en el que escuchamos solicitudes para el servicio
    sfdServidor = ObtenerSocket();

    if(sfdServidor==-1){
        fprintf(stderr, "ERROR: no se ha podido establecer la conexion\n");
        exit(-1);
    }

    //Entramos en un bucle infinito en el que esperamos y atendemos solicitudes
    while (1) {
        // Esperamos hasta que entre una solicitud
        sfdCliente = ConectarConCliente(sfdServidor);
        if(sfdCliente==-1){
            fprintf(stderr, "ERROR: no se ha podido establecer la conexion\n");
            exit(-1);
        }

        // Creamos un nuevo proceso que atienda la solicitud
        switch (fork()) {
            case -1:    // Error, cerramos y salimos
                fprintf(stderr, "ERROR: [%s] ejecutando fork\n",strerror(errno));
                close(sfdCliente);
                close(sfdServidor);
                exit (-1);

            case  0:    // El hijo atiende la nueva solicitud
                log("PROCESO HIJO %d: atendiendo protocolo de aplicacion en socket %d",(int)getpid(),sfdCliente);
                close(sfdServidor);
                result = AtenderProtocoloHTTP(sfdCliente);      // Atendemos el PROTOCOLO concreto
                close(sfdCliente); 
                log("PROCESO HIJO %d: terminando proceso",(int)getpid());                             
                exit (result);

            default:    // El padre se queda a la espera de nuevas conexiones
                log("PROCESO PADRE %d: buscando nueva conexion",(int)getpid());
                close(sfdCliente);
        }
    }   /* de while infinito */
} // Final de lanzar servicio




