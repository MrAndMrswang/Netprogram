#include<stdio.h>  
#include<stdlib.h>  
#include<string.h>  
#include<sys/epoll.h>  
#include<sys/socket.h>  
#include<unistd.h>
#include<arpa/inet.h>  

#define BUF_SIZE 100
#define EPOLL_SIZE 50
using namespace std;
void error_handling(char* buf);

int main(int argc, char* argv[])
{
	int serv_sock,clnt_sock;
	struct sockaddr_in serv_adr,clnt_adr;
	socklen_t adr_sz;
	int str_len,i;
	char buf[BUF_SIZE];

	struct epoll_event* ep_events;
	struct epoll_event event;

	int epfd,event_cnt;

	if(argc!=2)
		error_handling("argc less!");
	
	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_adr,0,sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));
	
	if(bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock,5)==-1)
		error_handling("listen() error");

	epfd = epoll_create(256);
	if(epfd < 0)  
	{  
	    error_handling("epoll_creat() error");
	    exit(5);  
	} 
	ep_events = (struct epoll_event*) malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
	
	event.events = EPOLLIN;
	event.data.fd = serv_sock;
	if(epoll_ctl(epfd,EPOLL_CTL_ADD,serv_sock,&event)==-1)
		error_handling("epoll_ctl() error");

	while(1)
	{
		event_cnt = epoll_wait(epfd, ep_events,EPOLL_SIZE, -1);
		if(event_cnt == -1)
		{
			error_handling("epoll_wait() error");
			break;
		}	
		for(i=0;i<event_cnt;i++)
		{
			if(ep_events[i].data.fd == serv_sock)
			{
				adr_sz = sizeof(clnt_adr);
				clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr,&adr_sz);
				event.events = EPOLLIN;
				event.data.fd = clnt_sock;
				epoll_ctl(epfd,EPOLL_CTL_ADD,clnt_sock,&event);
				printf("serv say : connect client : %d\n",ep_events[i].data.fd);
				

			}
			else
			{
				str_len = read(ep_events[i].data.fd,buf,BUF_SIZE);
				if(str_len = 0)
				{
					epoll_ctl(epfd,EPOLL_CTL_DEL,ep_events[i].data.fd,NULL);
					close(ep_events[i].data.fd);
					printf("serv say : closed client : %d",ep_events[i].data.fd);
				}
				else
				{
					write(ep_events[i].data.fd,buf,str_len);
				}
			}

		}
	
	}
	close (serv_sock);
	close(epfd);
	return 0;
}
void error_handling(char *buf)
{
	puts("serv say : ");
	puts(buf);
	exit(1);
}

