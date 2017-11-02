#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int cliSock;

void serverSalir(){
	close(cliSock);
	exit(0);
}

int hostnameToIp(char * hostname , char* ip){
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
     
    return 1;
}

void *autorizacion (void * sock){
	int bytesRecv;
	int serverSock = *((int *)sock);
	char rta [4]="no";
	puts("Esperando autorizacion del server para entrar a la sala. Por favor, espere.");
	while (strcmp(rta,"si")!=0) {
		if((bytesRecv = recv(serverSock,rta,3,0)) > 0) {
			rta[bytesRecv] = '\0';
		}

	}
	puts("\nBienvenido a la sala. Ingrese un mensaje:\n");
	pthread_mutex_unlock(&mutex);
	bzero(rta,sizeof(rta));
	fflush(stdout);
	pthread_exit(NULL);
} 

void *threadRecvMensaje(void *sock){
	int serverSock = *((int *)sock);
	char mensaje[1024];
	int bytesRecv;
	while((bytesRecv = recv(serverSock,mensaje,1024,0)) > 0) {
		mensaje[bytesRecv] = '\0';
		if(strcmp(mensaje,"SV_EXIT")==0){
			serverSalir();
		}
		fputs(mensaje,stdout);
		bzero(mensaje,sizeof(mensaje));
		fflush(stdout);
	}

}

int main(int argc, char *argv[])
{
	struct sockaddr_in serverAddr;
	int serverSock,serverAddr_size,nPuerto,bytesRecv;
	pthread_t sendt,recvt,aut;
	char mensaje[1024];
	char hostname[128],serverIp[128];
	char res[1100];
	char ip[INET_ADDRSTRLEN];
	char nickname[64];

	if(argc != 4) {
		puts("Error. La ejecucion debe ser del siguiente estilo: ./cliente {ip/hostname} {puerto server} {nickname}");
		printf(". Ejemplo de ejecucion: ./cliente localhost 3500 tomas ");
		exit(1);
	}
	nPuerto = atoi(argv[2]);
	strcpy(hostname,argv[1]);
	if(hostnameToIp(hostname,serverIp)==1){
		strcpy(serverIp,hostname);
	}
	strcpy(nickname,argv[3]);
	cliSock = socket(AF_INET,SOCK_STREAM,0);
	memset(serverAddr.sin_zero,'\0',sizeof(serverAddr.sin_zero));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(nPuerto);
	//serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_addr.s_addr = inet_addr(serverIp);

	
	if(connect(cliSock,(struct sockaddr *)&serverAddr,sizeof(serverAddr)) < 0) {
		perror("Cliente: error. Conexion no establecida");
		exit(1);
	}
	inet_ntop(AF_INET, (struct sockaddr *)&serverAddr, ip, INET_ADDRSTRLEN);
	pthread_create(&aut,NULL,autorizacion,&cliSock);
	pthread_mutex_lock(&mutex);
	pthread_mutex_lock(&mutex);
	//printf("connected to %s, start chatting\n",ip);
	pthread_create(&recvt,NULL,threadRecvMensaje,&cliSock);
	fflush(stdin);
	strcpy(nickname,argv[3]);
	//printf("\n");
	while(fgets(mensaje,1024,stdin) > 0) {
		strcpy(res,nickname);
		strcat(res,":");
		strcat(res,mensaje);
		bytesRecv = write(cliSock,res,strlen(res));
		if(bytesRecv < 0) {
			perror("Error: mensaje no enviado");
			exit(1);
		}
		printf("%s\n",res);
		bzero(mensaje,sizeof(mensaje));
		bzero(res,sizeof(res));
	}
	pthread_join(recvt,NULL);
	close(cliSock);
	pthread_mutex_unlock(&mutex);

}