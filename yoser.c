#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define AUTOOK 12345
#define AUTONO 98765

struct client_info {
	int sockno;
	char ip[INET_ADDRSTRLEN];
};
int clients[100];
int n = 0,salaLlena=0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void intHandler (int num);
void sendtoall2(char *msg,int curr)
{
	int i;
	for(i = 0; i < n; i++) {
	//puts("sendtoall2 FOR");	
			if(send(clients[i],msg,strlen(msg),0) < 0) {
				perror("sending failure");
				continue;
			}
					
	}
}
void sendtoall(char *msg,int curr)
{
	int i;
	pthread_mutex_lock(&mutex);
	for(i = 0; i < n; i++) {
		if(clients[i] != curr) {
			if(send(clients[i],msg,strlen(msg),0) < 0) {
				perror("sending failure");
				continue;
			}
		}
	}
	pthread_mutex_unlock(&mutex);
}
void salirServer(){
	int i;
	printf("\nStopping server\n");
	sendtoall2("SV_EXIT",-1);
	for(i=0 ; i<n;i++){
		close(clients[i]);
	}
	exit(0);
}

void *recvmg(void *sock)
{
	struct client_info cl = *((struct client_info *)sock);
	char msg[500];
	int len;
	int i;
	int j;
	while((len = recv(cl.sockno,msg,500,0)) > 0) {
		msg[len] = '\0';
		sendtoall(msg,cl.sockno);
		memset(msg,'\0',sizeof(msg));
	}
	pthread_mutex_lock(&mutex);
	printf("%s disconnected\n",cl.ip);
	for(i = 0; i < n; i++) {
		if(clients[i] == cl.sockno) {
			j = i;
			while(j < n-1) {
				clients[j] = clients[j+1];
				j++;
			}
		}
	}
	n--;
	printf("\nN: %d\n",n);
	if(n==1&&salaLlena==1){salirServer();}
	pthread_mutex_unlock(&mutex);
}

void *autorizacion (void * sock)
{
	
	struct client_info cl = *((struct client_info *)sock);
	int len;
	char msg [3];//no autorizado
	puts("server: verificando autorizacion");
	while (n!=3) {
		strcpy(msg,"no");
		sendtoall2(msg,cl.sockno);
		memset(msg,'\0',sizeof(msg)); 
	}
		puts("enviando autorizacion");
		salaLlena=1;
		strcpy(msg,"si");
		sendtoall2(msg,cl.sockno);
		memset(msg,'\0',sizeof(msg));
		pthread_mutex_unlock(&mutex2);


} 
int main(int argc,char *argv[])
{
	struct sockaddr_in my_addr,their_addr;
	int my_sock;
	int their_sock;
	socklen_t their_addr_size;
	int portno;
	pthread_t sendt,recvt,aut;
	char msg[500];
	int len;
	struct client_info cl;
	char ip[INET_ADDRSTRLEN];
	signal(SIGINT, intHandler);

	
	
	if(argc > 2) {
		printf("too many arguments");
		exit(1);
	}
	portno = atoi(argv[1]);
	my_sock = socket(AF_INET,SOCK_STREAM,0);
	memset(my_addr.sin_zero,'\0',sizeof(my_addr.sin_zero));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(portno);
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	their_addr_size = sizeof(their_addr);

	if(bind(my_sock,(struct sockaddr *)&my_addr,sizeof(my_addr)) != 0) {
		perror("binding unsuccessful");
		exit(1);
	}

	if(listen(my_sock,5) != 0) {
		perror("listening unsuccessful");
		exit(1);
	}

	while(1) {
		if((their_sock = accept(my_sock,(struct sockaddr *)&their_addr,&their_addr_size)) < 0) {
			perror("accept unsuccessful");
			exit(1);
		}
		pthread_mutex_lock(&mutex);
		inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);
		printf("%s connected\n",ip);
		cl.sockno = their_sock;
		strcpy(cl.ip,ip);
		clients[n] = their_sock;
		n++;
		pthread_create(&aut,NULL,autorizacion,&cl);
		pthread_mutex_lock(&mutex2);
		pthread_create(&recvt,NULL,recvmg,&cl);
		pthread_mutex_unlock(&mutex);

	}
	return 0;
	}

void intHandler (int num){
	int i;
	printf("\nStopping server\n");
	sendtoall("SV_EXIT",-1);
	for(i=0 ; i<n;i++){
		close(clients[i]);
	}
	exit(0);
}