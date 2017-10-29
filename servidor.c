#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#define MY_PORT_ID 6081
#define MAXLINE 256
#define MAXSIZE 512   //ACK??????????????????????????????????????????????????????????????????????????????????

pthread_t tid[100];//MAXIMO DE THREADS POSIBLES
void * procesoThread (void * sockid2);
char buf [100];
int numbytes;

//Procedimiento principal del servidor
int main(int argc, char *argv[]){//Parmetros:N cli y puerto a utilizar

//Defino las variables que voy a utilizar
int se, sc, sockid, cont=0, sockid2, j=0, newsd, pid, clilen, myport, cant_max_con, sin_size;
struct sockaddr_in my_addr, client_addr;



//int numbytes;
//char buf[100];//MAXDATASIZE

//Verificacion de parametros
//if(argc!=3){
//printf("Server: Cantidad de parametros incorrecta\n");
//exit(0);
//}



cant_max_con = 10;//atoi(argv[2]);

//Creo el socket que va a "escuchar" las conexiones, si no puedo hacerlo muestro un mensaje de error
//Verifico puerto

myport = 3550;//atoi(argv[1]);
printf("Puerto: %d\n",myport);
if(myport<1025 || myport>65535){
	printf("Cliente: Puerto no vlido (%d)\n",myport);
	exit(0);
}
if ((sockid = socket(AF_INET,SOCK_STREAM,0)) < 0){
	printf("Servidor: Error en la creacin del socket: %d\n", errno); exit(0);
}
else {
	printf("Creacion del socket: OK\n");
}
//Ahora debo "avisarle" al SO sobre el socket abierto y que se asocie al servidor, lo configuro con la estructura de tipo "sockaddr_in"
//Luego con el comando bind le asigno la configuracion en "my_addr" al socket creado anteriormente, verifico que se haya hecho
//correctamente y si hubo algun error lo informo
printf("Servidor: Configurando el socket local\n");
bzero((char *) &my_addr,sizeof(my_addr));
my_addr.sin_family = AF_INET;
my_addr.sin_port = htons(myport);
my_addr.sin_addr.s_addr = htons(INADDR_ANY);
if (bind(sockid ,(struct sockaddr *) &my_addr,sizeof(my_addr)) < 0){
	printf("Servidor: Error en la configuracion del socket :%d\n", errno); exit(0);
}
//Ahora que tengo el socket listo, lo pongo a escuchar. 
printf("Servidor: Comenzando escucha \n");
if (listen(sockid,cant_max_con) < 0){ //Solo voy a escuchar hasta MAXCON conexiones a la vez
	printf("Servidor: Error en la escucha :%d\n",errno);exit(0);
}
//Dentro del servidor, voy a estar en bucle esperando conexiones, en la que acepte alguna voy a hacer un fork, dejando que el
//Proceso hijo se encargue de la conexion y volviendo al proceso padre a esperar por otras conexiones   
while(1) {
	sin_size=sizeof(struct sockaddr_in);
	/* A continuacin la llamada a accept() */
	while ((sockid2 = accept(sockid,(struct sockaddr *)&client_addr,&sin_size))!=-1) {
	printf("Se obtuvo una conexin desde %s\n",inet_ntoa(client_addr.sin_addr) );
	pthread_create(&(tid[j]), NULL, &procesoThread, &sockid2);
	j++;
	}
}

close(sockid);
exit(0);
}

void * procesoThread(void * sockid2){///, void * buf){
	int * sockidC = (int *)sockid2;
	int sockidC2 = *sockidC;
	int senal = 400;
	puts("hola, soy un thread y recien traido a este mundo");
	send(sockidC2,"Bienvenido a la sala.\n",21,0);
	//si hay algun mensaje enviado antes, puedo enviarlo al cliente
	while(1){
		/* que enviar el mensaje de bienvenida al cliente */


		if((numbytes=recv(sockidC2,buf,100,MSG_DONTWAIT))!=-1){
			buf[numbytes]='\0';
			printf("mje del cli:%s\n",buf);
		}

		if(buf[0]!='\0'){
		send(sockidC2,buf,100,0);
		buf[0]='\0';
		}



	}

///ACA IBA EL BLOQUE DE CODIGO DEL RECV
	pthread_exit(&senal);
	close(sockidC2); /* cierra fd2 */
	return 0;
}
