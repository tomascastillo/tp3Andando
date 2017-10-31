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
//pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
void *autorizacion (void * sock)
{
	int len;
		//Comienzo la comunicacion con el cliente, voy a recibir el mensaje (o comando) que envie el mismo y guardarlo en "msg"
		//aca me pregunta si puede entrar a la sala

	//Si no existen la cantidad de clientes necesaria, le envio un AUTONO al cliente y me quedo esperando a q haya mas clientes, sino le mando un AUTOOK
	//if(strcmp(msg,"au")==0);
	int their_sock = *((int *)sock);
	char rta [4]="no";//no autorizado
	//pthread_mutex_lock(&mutex2);
	puts("Esperando autorizacion del server para entrar a la sala. Por favor, espere.");
	//pthread_mutex_lock(&mutex);
	//strcpy(msg,"no");
	while (strcmp(rta,"si")!=0) {
		//strcpy(msg,"au");
		//write(their_sock,msg,strlen(msg));
		//memset(msg,'\0',sizeof(msg));
	//puts("entro en el while");
	//puts(rta);
		if((len = recv(their_sock,rta,3,0)) > 0) {
			//puts("entro en el if");
			rta[len] = '\0';
			//puts(rta);
			//memset(rta,'\0',sizeof(rta));
			bzero(rta,sizeof(rta));
			fflush(stdout);
		}

	//else puts("NO entro en el if");
	///EL PROBLEMA ES Q NO RECIBE NADA
	}
	puts("Bienvenido a la sala. Escriba:");
	pthread_mutex_unlock(&mutex);
//	pthread_mutex_unlock(&mutex2);
	pthread_exit(NULL);

	//pthread_join(main,NULL);

} 
void *recvmg(void *sock)
{
	//pthread_mutex_lock(&mutex2);
	int their_sock = *((int *)sock);
	char msg[500];
	int len;
	//while(jaja==0){sleep;}
	while((len = recv(their_sock,msg,500,0)) > 0) {
		msg[len] = '\0';
		fputs(msg,stdout);
		//memset(msg,'\0',sizeof(msg));
		bzero(msg,sizeof(msg));
		fflush(stdout);
	}
	//pthread_mutex_unlock(&mutex2);

}
/*void recvmg2(void *sock)
{
	int their_sock = *((int *)sock);
	char msg[500];
	int len;
	while((len = recv(their_sock,msg,500,0)) > 0) {
		msg[len] = '\0';
		memset(msg,'\0',sizeof(msg));
	}
}*/
int main(int argc, char *argv[])
{
	struct sockaddr_in their_addr;
	int my_sock;
	int their_sock;
	int their_addr_size;
	int portno;
	pthread_t sendt,recvt,aut;
	char msg[500];
	char username[100];
	char res[600];
	char ip[INET_ADDRSTRLEN];
	int len;

	if(argc > 3) {
		printf("too many arguments");
		exit(1);
	}
	portno = atoi(argv[2]);
	strcpy(username,argv[1]);
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
	//aca hay que preguntarle al servidor si se puede entrar a la sala de chat.
	//send(serv,autorizacion)
	//recv(serv,autorizacion)
	//mientras la respuesta sea NO se puede iniciar, seguir preguntando y no salir del loop
	//autorizacion(&my_sock);
	pthread_create(&aut,NULL,autorizacion,&my_sock);
	//pthread_mutex_lock(&mutex);
	pthread_mutex_lock(&mutex);
	printf("connected to %s, start chatting\n",ip);
	pthread_create(&recvt,NULL,recvmg,&my_sock);
	fflush(stdin);
	strcpy(username,argv[1]);
	printf("\n");
	while(fgets(msg,500,stdin) > 0) {
		strcpy(res,username);
		strcat(res,":");
		strcat(res,msg);
		len = write(my_sock,res,strlen(res));
		if(len < 0) {
			perror("message not sent");
			exit(1);
		}
		bzero(msg,sizeof(msg));
		bzero(res,sizeof(res));
		//memset(msg,'\0',sizeof(msg));
		//memset(res,'\0',sizeof(res));
	}
	pthread_join(recvt,NULL);
	close(my_sock);
	pthread_mutex_unlock(&mutex);

}