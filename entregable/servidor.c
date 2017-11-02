#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

struct sInfoCli {
	int nSocket;
	char ip[INET_ADDRSTRLEN];
};

int vecCli[200];
int cantCliActuales = 0,salaLlena=0,cantMinCli;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void intHandler (int num);

int esPuertoValido(int puerto){
		if(puerto<1024 || puerto>65535){
			puts("Error. El puerto debe ser un entero entre 1025 y 65535\n");
			puts(" La ejecucion debe ser del siguiente estilo: ./server {puerto server} {cantidad de clientes en la sala}");
			printf(". Ejemplo de ejecucion: ./server 3500 3");
			exit(0);
	}
}

void enviarAlMismo(char *mensaje,int actual){
	int i;
	for(i = 0; i < cantCliActuales; i++) {
		if(vecCli[i] == actual) {
				if(send(vecCli[i],mensaje,strlen(mensaje),0) < 0) {
					perror("Server: error en send");
					continue;
			}
		}					
	}
}

void enviarATodos(char *mensaje,int actual){
	int i;
	for(i = 0; i < cantCliActuales; i++) {
		if(vecCli[i] != actual) {
			if(send(vecCli[i],mensaje,strlen(mensaje),0) < 0) {
				perror("Server: error en send");
				continue;
			}
		}
	}
}

void salirServer(){
	int i;
	printf("\nCerrando el servidor...\n");
	enviarATodos("SALIR_DEL_SERVER",-1);
	for(i=0 ; i<cantCliActuales;i++){
		close(vecCli[i]);
	}
	exit(0);
}

void *threadRecvMensaje(void *sock){
	struct sInfoCli cliente = *((struct sInfoCli *)sock);
	char mensaje[1024];
	int bytesRecv,i,j;
	pthread_mutex_lock(&mutex2);
	while((bytesRecv = recv(cliente.nSocket,mensaje,1024,0)) > 0) {
		mensaje[bytesRecv] = '\0';
		enviarATodos(mensaje,cliente.nSocket);
		memset(mensaje,'\0',sizeof(mensaje));
	}
	pthread_mutex_lock(&mutex);
	printf("El cliente: %s se ha desconectado\n",cliente.ip);
	for(i = 0; i < cantCliActuales; i++) {
		if(vecCli[i] == cliente.nSocket) {
			j = i;
			while(j < cantCliActuales-1) {
				vecCli[j] = vecCli[j+1];
				j++;
			}
		}
	}
	cantCliActuales--;
	printf("\nServer: cantidad de clientes en la sala: %d\n",cantCliActuales);
	if(cantCliActuales==1&&salaLlena==1){salirServer();}
	pthread_mutex_unlock(&mutex);
}

void *autorizacion (void * sock){
	struct sInfoCli cliente = *((struct sInfoCli *)sock);
	int bytesRecv;
	char mensaje [20];

	puts("Server: verificando autorizacion para entrar a la sala.");
	if(salaLlena==1&&cantCliActuales==cantMinCli){
		strcpy(mensaje,"AUT_LLENA");
		enviarAlMismo(mensaje,cliente.nSocket);
		memset(mensaje,'\0',sizeof(mensaje));	
	}
	while (cantCliActuales!=cantMinCli&&salaLlena==0) {
		strcpy(mensaje,"AUT_NO");
		enviarAlMismo(mensaje,cliente.nSocket);
		memset(mensaje,'\0',sizeof(mensaje)); 
	}
		puts("Server: Enviando autorizacion a los clientes");
		salaLlena=1;
		strcpy(mensaje,"AUT_SI");
		enviarAlMismo(mensaje,cliente.nSocket);
		memset(mensaje,'\0',sizeof(mensaje));
		pthread_mutex_unlock(&mutex2);
		pthread_exit(NULL);


} 
int main(int argc,char *argv[])
{
	struct sockaddr_in serverAddr,cliAddr;
	int serverSock,cliSock,nPuerto,bytesRecv;
	socklen_t cliAddr_size;
	pthread_t sendt,recvt,aut;
	char mensaje[1024];
	struct sInfoCli cliente;
	char ip[INET_ADDRSTRLEN];
	signal(SIGINT, intHandler);

	if(argc != 3) {
		puts("Error. La ejecucion debe ser del siguiente estilo: ./server {puerto server} {cantidad de clientes en la sala}");
		printf(". Ejemplo de ejecucion: ./server 3500 3");
		exit(1);
	}
	nPuerto = atoi(argv[1]);
	esPuertoValido(nPuerto);
	cantMinCli = atoi(argv[2]);
	serverSock = socket(AF_INET,SOCK_STREAM,0);
	memset(serverAddr.sin_zero,'\0',sizeof(serverAddr.sin_zero));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(nPuerto);
	serverAddr.sin_addr.s_addr= INADDR_ANY;
	//serverAddr.sin_addr.s_addr = inet_addr(INADDR_ANY);
	cliAddr_size = sizeof(cliAddr);

	if(bind(serverSock,(struct sockaddr *)&serverAddr,sizeof(serverAddr)) != 0) {
		perror("Server: error bind");
		exit(1);
	}

	if(listen(serverSock,cantMinCli) != 0) {
		perror("Server: error en listen");
		exit(1);
	}
	puts("Server: esperando que se conecten todos los clientes para comenzar la sala de chat\n");

	while(1) {
		if((cliSock = accept(serverSock,(struct sockaddr *)&cliAddr,&cliAddr_size)) < 0) {
			perror("Server: error en accept");
			exit(1);
		}
		pthread_mutex_lock(&mutex);
		inet_ntop(AF_INET, (struct sockaddr *)&cliAddr, ip, INET_ADDRSTRLEN);
		printf("La ip %s se ha conectado\n",ip);
		cliente.nSocket = cliSock;
		strcpy(cliente.ip,ip);
		vecCli[cantCliActuales] = cliSock;
		cantCliActuales++;
		pthread_create(&aut,NULL,autorizacion,&cliente);
		pthread_create(&recvt,NULL,threadRecvMensaje,&cliente);
		pthread_mutex_unlock(&mutex);

	}
	return 0;
}

void intHandler (int num){
	int i;
	printf("\nCerrando el servidor...\n");
	enviarATodos("SALIR_DEL_SERVER",-1);
	for(i=0 ; i<cantCliActuales;i++){
		close(vecCli[i]);
	}
	exit(0);
}