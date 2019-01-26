//
// main.c
//
// Descripción: mi_wget texto
//
// fecha:xx
// autor: paco paco
// (c) Copyright
//
// Ejemplo de compilación
// 	gcc main.c -o mi_wget -D DEBUG
//
// Ejemplo de utilización:
//	./mi_wget HTTP localhost 8080 /index.html
//

/*			BIBLIOGRAFIA

http://www.chuidiang.org/clinux/sockets/sockets_simp.php


*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <error.h>
#include <errno.h>

#define PORT						80	// puerto de escucha del servidor web (HTTP)
#define HTTP_REQUEST_PATTERN_GET	"GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: Keep-alive\r\n\r\n"
#define HTTP_REQUEST_PATTERN_HEAD	"HEAD %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: Keep-alive\r\n\r\n"
#define HTTP_REQUEST_PATTERN_DELETE	"DELETE %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: Keep-alive\r\n\r\n"
#define HTTP_REQUEST_PATTERN_PUT 	"PUT %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: Keep-alive\r\n\r\n"
#define HTTP_REQUEST_PATTERN_METHOD_NOT_ALLOWED	"METHOD NOT ALLOWED %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: Keep-alive\r\n\r\n"
#define HTTP_REQUEST_BAD_REQUEST	"BAD REQUEST %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: Keep-alive\r\n\r\n"

#define BUFFSIZE	500

// Mis herramientas de log
#ifdef DEBUG
#define log(fmt, args...)	fprintf(stderr, fmt "\n", ## args)
#else
#define log(fmt, args...)
#endif

void pexit(char *msg)
{

	perror(msg);
	exit(EXIT_FAILURE);

} // de pexit

// Este es el verdadero cliente web (http)
void clienteweb(char *dirIP, int puerto, char *recurso, char *accion)
{

	int sockfd;
	static struct sockaddr_in serv_addr;
	/* Estructura de struct sockaddr_in
		
           struct sockaddr_in {
               sa_family_t    sin_family;  address family: AF_INET 
               in_port_t      sin_port;    port in network byte order 
               struct in_addr sin_addr;    internet address 
           }

           Internet address. 
           struct in_addr {
               uint32_t       s_addr;      address in network byte order 
           }
	*/

	char HTTP_request[200];
	char HTTP_response[BUFFSIZE]; // Pagina web completa
	int bLeidos;

	//SOCKET: abrimos un canal
	//La función socket() devuelve un numero que será el numero de conexión 
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd < 0){
		pexit("socket() fallido");
	}

	// CONNECT: solicitamos una comunicación con un servidor en un puerto
	// connect(sockfd, dirIP, puerto);

	serv_addr.sin_family = AF_INET; //Asignamos el tipo de conexión
	serv_addr.sin_port = htons(puerto); //The htons() function converts the unsigned short integer hostshort from host byte order to network byte order. 
	serv_addr.sin_addr.s_addr = inet_addr(dirIP); //Transforma la direccion ip a bytes

	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <0){
		pexit("coonnect() fallido");
	}

	// WRITE: Realizamos la solicitud (http)
	if(strcmp(dirIP,"localhost")==0 || puerto==8080){
		//TODO se compara con strcmp
		if(strcmp(accion,"GET")==0){
			
			sprintf(HTTP_request, HTTP_REQUEST_PATTERN_GET, recurso, dirIP, "generic server");
			log(" * Enviando %d bytes:\n\n%s\n", (int)strlen(HTTP_request), HTTP_request);
			write(sockfd, HTTP_request, (size_t)strlen(HTTP_request));

		}else if(strcmp(accion,"HEAD")==0){

			sprintf(HTTP_request, HTTP_REQUEST_PATTERN_HEAD, recurso, dirIP, "generic server");
			log(" * Enviando %d bytes:\n\n%s\n", (int)strlen(HTTP_request), HTTP_request);
			write(sockfd, HTTP_request, (size_t)strlen(HTTP_request));

		}else if(strcmp(accion,"PUT")==0){

			sprintf(HTTP_request, HTTP_REQUEST_PATTERN_PUT, recurso, dirIP, "generic server");
			log(" * Enviando %d bytes:\n\n%s\n", (int)strlen(HTTP_request), HTTP_request);
			write(sockfd, HTTP_request, (size_t)strlen(HTTP_request));

		}else if(strcmp(accion,"DELETE")==0){

			sprintf(HTTP_request, HTTP_REQUEST_PATTERN_DELETE, recurso, dirIP, "generic server");
			log(" * Enviando %d bytes:\n\n%s\n", (int)strlen(HTTP_request), HTTP_request);
			write(sockfd, HTTP_request, (size_t)strlen(HTTP_request));

		}else{

			sprintf(HTTP_request, HTTP_REQUEST_PATTERN_METHOD_NOT_ALLOWED, recurso, dirIP, "generic server");
			log(" * Enviando %d bytes:\n\n%s\n", (int)strlen(HTTP_request), HTTP_request);
			write(sockfd, HTTP_request, (size_t)strlen(HTTP_request));

		}
	}else{
		sprintf(HTTP_request, HTTP_REQUEST_BAD_REQUEST, recurso, dirIP, "generic server");
		log(" * Enviando %d bytes:\n\n%s\n", (int)strlen(HTTP_request), HTTP_request);
		write(sockfd, HTTP_request, (size_t)strlen(HTTP_request));
	}
	
	





	// READ / HTTP_RESPONSE: Leemos la respuesta del servidor (http)
	while ((bLeidos = (int)read(sockfd, HTTP_response, sizeof(HTTP_response))) >0) {
		write(STDOUT_FILENO, HTTP_response, bLeidos);
	}

	// CLOSE: cerramos el canal
	close(sockfd);

} // de clienteweb


// Punto de entrada a la aplicación
int main(int argc, char*argv[]) // argv[0] es el nombre de la aplicacion argv[0]!=
{

	char *protocol;
	char *host;
	int port;
	char *resource;
	char *host_ip;
	char *accion;
	struct servent *sp; // para convertir Protocolo en puerto
	struct hostent *host_struct; // para convertir DNS en IP

	// verificar los argumentos de entrada
	if(argc != 6) {
		fprintf(stderr, "Uso: %s PROTOCOLO HOST PORT RESOURCE\n", argv[0]);
		return EXIT_FAILURE;
	}

	// Extraemos la informacion de los parametros de entrada
	protocol = argv[1];
	host = argv[2];
	port = atoi(argv[3]);
	resource = argv[4];
	accion = argv[5];

	// Convertimos el PROTOCOLO en su puerto si NO hay puerto (puerto 0)

	if ((protocol[0]!='\0') && (port==0)){
		sp = getservbyname(protocol,"tcp");
		if(!sp){
			fprintf(stderr, "No se ha podido encontrar un puerto valido para el protoclo%s\n",protocol);
			return EXIT_FAILURE;
		}
		port = ntohs(sp->s_port);

	}

	// Convertimos el DNS (host) en su equivalente IP(host_ip)

	host_struct = gethostbyname(host);

	if(host_struct != NULL){
		host_ip = inet_ntoa(*(struct in_addr *) host_struct->h_addr);
	}


	// Trazar la información
	log(" * protocolo = <%s>",protocol);
	log(" * host = <%s>", host);
	log(" * port = <%d>", port);
	log(" * recurso = <%s>", resource);
	log(" * IP = <%s>", host_ip);
	log(" * Accion = <%s>",accion);

	// Comprobamos si lo tenemos todo
	if(!port || !host_ip || resource[0]=='\0'){
		fprintf(stderr, "URL no valida\n");
		return EXIT_FAILURE;
	}

	// Invocar a nuestro verdadero cliente  web
	clienteweb(host_ip, port, resource,accion);

	// Salimos de la aplicación
	return EXIT_SUCCESS;


} // de main
