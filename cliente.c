#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define PORT 3550
/* El Puerto Abierto del nodo remoto */
#define MAXDATASIZE 100
/* El nmero mximo de datos en bytes */

int main(int argc, char *argv[]){

int fd, numbytes;
char cad[100];
char buf2[100];
char buf3[100];
int num2,num3;
pid_t pid;
/* ficheros descriptores */
char buf[MAXDATASIZE];
/* en donde es almacenar el texto recibido */
struct hostent *he;
/* estructura que recibir informacin sobre el nodo remoto */
struct sockaddr_in server;
/* informacin sobre la direccin del servidor */
//int port=argv...............
//int nick = argv.........

if (argc !=2) {
	/* esto es porque nuestro programa slo necesitar un
	argumento, (la IP) */
	printf("Uso: %s <Direccin IP>\n",argv[0]);
	exit(-1);
}

if ((he=gethostbyname(argv[1]))==NULL){
	/* llamada a gethostbyname() */
	printf("gethostbyname() error\n");
	exit(-1);
}

if ((fd=socket(AF_INET, SOCK_STREAM, 0))==-1){
	/* llamada a socket() */
	printf("socket() error\n");
	exit(-1);
}

server.sin_family = AF_INET;
server.sin_port = htons(PORT);
/* htons() es necesaria nuevamente ;-o */
server.sin_addr = *((struct in_addr *)he->h_addr);
/*he->h_addr pasa la informacin de "*he" a "h_addr" */
bzero(&(server.sin_zero),8);

if(connect(fd, (struct sockaddr *)&server,sizeof(struct sockaddr))==-1){
	/* llamada a connect() */
	printf("connect() error\n");
	exit(-1);
}

///MENSAJE DE BIENVENIDA******************************************
if ((numbytes=recv(fd,buf,MAXDATASIZE,0)) == -1){
	/* llamada a recv() */
	printf("Error en recv() \n");
	exit(-1);
}
	buf[numbytes]='\0';
	printf("Mensaje del Servidor: %s\n",buf);

///POR SI TENGO ALGUN MENSAJE ESPERANDO??**************************?????
if ((num3=recv(fd,buf3,MAXDATASIZE,MSG_DONTWAIT)) != -1){
	buf3[num3]='\0';
	printf("Mensaje del user: %s\n",buf3);
}

//printf("Ingrese un mensaje: nickname\n");

////while(strcmp(cad,"/salir")!=0){

	pid = fork();

	if (pid == 0){
		while ((num2=recv(fd,buf2,MAXDATASIZE,0)) != -1){
		buf2[num2]='\0';
		printf("Mensaje del user: %s\n",buf2);
		}
	}

	else if (pid > 0){
//	else{
//	if(cad[0]!='\0'){
		if(strcmp(cad,"/salir")!=0){
		printf("Ingrese un mensaje: nickname\n");
		fflush(stdin);
		scanf("%s",cad);
		send(fd,cad,strlen(cad),0);
		//sleep(100000);
		cad[0]='\0';
		}
//	}

	}



////}


close(fd);
exit(0);
return 0;
}



