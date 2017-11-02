#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define AUTOOK 12345
#define AUTONO 98765

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int my_sock;


void serverSalir(){
	close(my_sock);
	exit(0);

}
void *autorizacion (void * sock)
{
	int len;
	int their_sock = *((int *)sock);
	char rta [4]="no";//no autorizado
	puts("Esperando autorizacion del server para entrar a la sala. Por favor, espere.");
	while (strcmp(rta,"si")!=0) {
		if((len = recv(their_sock,rta,3,0)) > 0) {
			rta[len] = '\0';
		}

	}
	puts("\nBienvenido a la sala. Escriba:\n");
	pthread_mutex_unlock(&mutex);
	bzero(rta,sizeof(rta));
	fflush(stdout);
	pthread_exit(NULL);

} 
void *recvmg(void *sock)
{
	int their_sock = *((int *)sock);
	char msg[500];
	int len;
	while((len = recv(their_sock,msg,500,0)) > 0) {
		msg[len] = '\0';
		if(strcmp(msg,"SV_EXIT")==0){
			serverSalir();
		}
		fputs(msg,stdout);
		bzero(msg,sizeof(msg));
		fflush(stdout);
	}

}

int main(int argc, char *argv[])
{
	struct sockaddr_in their_addr;
	//int my_sock;
	int their_sock;
	int their_addr_size;
	int portno;
	pthread_t sendt,recvt,aut;
	char msg[500];
	char username[100];
	char res[600];
	char ip[INET_ADDRSTRLEN];
	int len;
	char nickname[600];

	if(argc != 4) {
		puts("Error. La ejecucion debe ser del siguiente estilo: ./cliente {ip/hostname} {puerto server} {nickname}");
		printf(". Ejemplo de ejecucion: ./cliente localhost 3500 tomas ");
		exit(1);
	}
	portno = atoi(argv[2]);
	strcpy(username,argv[1]);
	strcpy(nickname,argv[3]);
	my_sock = socket(AF_INET,SOCK_STREAM,0);
	memset(their_addr.sin_zero,'\0',sizeof(their_addr.sin_zero));
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(portno);
	their_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(my_sock,(struct sockaddr *)&their_addr,sizeof(their_addr)) < 0) {
		perror("connection not esatablished");
		exit(1);
	}
	inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);
	pthread_create(&aut,NULL,autorizacion,&my_sock);
	pthread_mutex_lock(&mutex);
	pthread_mutex_lock(&mutex);
	//printf("connected to %s, start chatting\n",ip);
	pthread_create(&recvt,NULL,recvmg,&my_sock);
	fflush(stdin);
	strcpy(nickname,argv[3]);
	printf("\n");
	while(fgets(msg,500,stdin) > 0) {
		strcpy(res,nickname);
		strcat(res,":");
		strcat(res,msg);
		len = write(my_sock,res,strlen(res));
		if(len < 0) {
			perror("message not sent");
			exit(1);
		}
		printf("%s\n",res);
		bzero(msg,sizeof(msg));
		bzero(res,sizeof(res));
	}
	pthread_join(recvt,NULL);
	close(my_sock);
	pthread_mutex_unlock(&mutex);

}