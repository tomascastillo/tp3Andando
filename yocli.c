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

void autorizacion (void * sock)
{
	int len;
		//Comienzo la comunicacion con el cliente, voy a recibir el mensaje (o comando) que envie el mismo y guardarlo en "msg"
		//aca me pregunta si puede entrar a la sala

	//Si no existen la cantidad de clientes necesaria, le envio un AUTONO al cliente y me quedo esperando a q haya mas clientes, sino le mando un AUTOOK
	//if(strcmp(msg,"au")==0);
	int their_sock = *((int *)sock);
	char msg [3]="no";//no autorizado
	while (strcmp(msg,"si")!=0) {
		strcpy(msg,"au");
		write(their_sock,msg,strlen(msg));
		memset(msg,'\0',sizeof(msg));

	if((len = recv(their_sock,msg,3,0)) > 0) {
		msg[len] = '\0';
		puts(msg);
		memset(msg,'\0',sizeof(msg));
	}		
	}

} 
void *recvmg(void *sock)
{
	int their_sock = *((int *)sock);
	char msg[500];
	int len;
	while((len = recv(their_sock,msg,500,0)) > 0) {
		msg[len] = '\0';
		fputs(msg,stdout);
		memset(msg,'\0',sizeof(msg));
	}
}
void recvmg2(void *sock)
{
	int their_sock = *((int *)sock);
	char msg[500];
	int len;
	while((len = recv(their_sock,msg,500,0)) > 0) {
		msg[len] = '\0';
		memset(msg,'\0',sizeof(msg));
	}
}
int main(int argc, char *argv[])
{
	struct sockaddr_in their_addr;
	int my_sock;
	int their_sock;
	int their_addr_size;
	int portno;
	pthread_t sendt,recvt;
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
	autorizacion(&my_sock);
	printf("connected to %s, start chatting\n",ip);
	pthread_create(&recvt,NULL,recvmg,&my_sock);
	while(fgets(msg,500,stdin) > 0) {
		strcpy(res,username);
		strcat(res,":");
		strcat(res,msg);
		len = write(my_sock,res,strlen(res));
		if(len < 0) {
			perror("message not sent");
			exit(1);
		}
		memset(msg,'\0',sizeof(msg));
		memset(res,'\0',sizeof(res));
	}
	pthread_join(recvt,NULL);
	close(my_sock);

}