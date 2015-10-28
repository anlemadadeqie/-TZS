#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>
#include <signal.h>
#include <pthread.h>

#define LISTEN_NUM 15
#define MYPORT  4500
#define REGSINER1 1			//注册名判断
#define REGSINER2 2			//注册名传递
#define LOGIN 3				//登陆判断
#define ALL 4				//全体消息
#define ONE 5				//私聊
#define ONLINE 6			//查看在线用户
#define ADD 7                      

int sock_fd;				//套接字
int team[100] = {0};			//最多记录100个链接到服务器的客户端
int size = 0;				//记录客户端的个数，数组的索引
typedef struct
{
	char username[20];			//在线用户名
	int sock_id;				//在线用户的套接口关键字
}online;

typedef struct online_node
{
	online data;
	struct online_node *next;
}onlnode,*onllist;

typedef struct
{
	int flag;			//标记
	char mess[1000];		//消息内容
	char revname[20];		//接收消息的用户名
	char username[20];		//存储用户名		
	char password[20];		//存储密码
}msg;

typedef struct msg_node
{
	struct msg_node *next;
	msg data;
}lnode, *linklist;
linklist head;				//全局变量,保存注册用户
onllist onlhead;			//保存在线用户

char * now_time ()   
{   
	struct tm *ptm; 
	long ts;
	static	char times[20];
        int y,m,d,h,n,s; 
	ts   =   time(NULL); 
	ptm   =   localtime(&ts); 

	y   =   ptm-> tm_year+1900;     //年 
	m   =   ptm-> tm_mon+1;             //月 
	d   =   ptm-> tm_mday;               //日 
	h   =   ptm-> tm_hour;               //时 
	n   =   ptm-> tm_min;                 //分 
	s   =   ptm-> tm_sec;                 //秒
	sprintf(times,"%02d-%02d-%02d-%02d:%02d:%02d",y,m,d,h,n,s);
	return times;
} 

void sys_logs(int type)
{
	FILE *fp;
	char *p;
	char str[100];
	fp = fopen("sys_logs.txt","a+");
	p = now_time();
	memset(str,'\0',sizeof(str));
	if(type == 1) { //用户名已存在，注册失败
		sprintf(str,"%s 用户名已存在，注册失败",p); 
	} else if(type == 2) {  //注册成功
		sprintf(str,"%s 注册成功",p);
	} else if(type == 3) { //密码输入正确
		sprintf(str,"%s 密码输入正确",p);
	} else if(type == 4){ //密码输入错误
		sprintf(str,"%s 密码输入错误",p);
	} else if(type == 5) { //用户名存在
    		sprintf(str,"%s 用户名存在",p);
	} else if(type == 6) { //无此用户
    		sprintf(str,"%s 无此用户",p);
	} else if(type == 7) { //断开连接
		sprintf(str,"%s 断开连接",p);
	} else if(type == 8) { //连接成功
		sprintf(str,"%s 连接成功",p);
	} else if(type == 9) { //启动服务器
		sprintf(str,"%s 启动服务器",p);
	}else if(type == 10) { //关闭服务器
		sprintf(str,"%s 关闭服务器",p);
	}
	fprintf(fp,"%s\n",str);
	fclose(fp);

}
/*从文件中读取注册用户信息*/
void read_user(void)
{
	FILE *fp;
	fp = fopen("user.txt","rt");
	if(!fp)
	{
		printf("read file error\n");
		exit(1);
	}
	else
	{
		linklist p,pnew;
		head = (linklist)malloc(sizeof(lnode));
		p=head;
		while(!feof(fp))
		{
			pnew = (linklist)malloc(sizeof(lnode));
			fscanf(fp,"%s %s\n",pnew->data.username,pnew->data.password);
			p->next = pnew;
			p= pnew;
		}
		p->next = NULL;
		fclose(fp);
	}
	
}


/*登陆之后创建在线用户的链表*/
void create_online(void)
{
	onllist p,pnew;
	onlhead= (onllist)malloc(sizeof(onlnode));
	p=onlhead;
	p->next = NULL;
}


/*添加一个在线用户*/
void add_online(char *username,int fd)
{
	onllist s, p;
	p = onlhead;
	while(p->next != NULL)
	{
		p= p->next;
	}
	s =(onllist)malloc(sizeof(onlnode));
	strcpy(s->data.username,username);
	s->data.sock_id = fd;
	s->next = p->next;
	p->next = s;
}


/*删除一个离线用户*/
void del_online(int fd)
{
	onllist s, p;
	p = onlhead;
	while(p->next != NULL &&(p->data.sock_id!=fd))
	{
		
		s =p;
		p= p->next;
	}
	if((p->next == NULL) && (p->data.sock_id!=fd))
	{
		return ;
	}	
	else
	{
		s->next = p->next;
		free(p);
	}
}


/*保存注册用户的信息*/
void save_user(linklist head)
{
	linklist p=head;
	FILE *fp;
	if((fp=fopen("user.txt","wt"))==NULL)
	{
		printf("save error\n");
		exit(1);
	}
	p=p->next;
	while(p!=NULL)
	{
		fprintf(fp,"%s %s\n",p->data.username,p->data.password);
		p=p->next;
	}
	fclose(fp);
}


/*在单链表中插入某一节点*/
void add_newuser(msg new)
{
	linklist s, p,tp;
	p = head;
	tp = head->next;
	s =(linklist)malloc(sizeof(lnode));
	strcpy(s->data.username,new.username);
	strcpy(s->data.password,new.password);
	p->next = s;
	s->next = tp;
	save_user(head);
}


/*判断用户名是否被使用*/
void isused(msg buf,int fd)
{
	linklist s, p;
	p=head;
	char s1[10]="OK";
	char s2[10]="NO";
	while(p->next != NULL)
	{
		if(strcmp(p->data.username,buf.username) == 0)
		{
			send(fd,s2,sizeof(s2),0);
			return;
		}
		p=p->next;
	}
	send(fd,s1,sizeof(s1),0);
}

/*登陆函数*/
void islogin(msg new,int fd)
{
	FILE *fp;
	char s1[10]="success";
	char s2[10]="Failed";
	msg data;
	fp = fopen("user.txt","rt");
	if(fp == NULL)
	{
		printf("read error\n");
		exit(1);
	}
	rewind(fp);
	while(fscanf(fp,"%s %s\n",data.username,data.password)!= EOF)
	{
		printf("%s %s\n",data.username,data.password);
		if(strcmp(data.username,new.username)==0 && strcmp(data.password,new.password)==0)
		{
			add_online(data.username,fd);
			send(fd,s1,sizeof(s1),0);
			return;
		}
	}
	send(fd,s2,sizeof(s2),0);
	return ;
}


void save_msg(char *msg,char *name,char *revname)
{
	FILE *fp;
	char *p;
	p=now_time();
	fp = fopen(revname,"at+");
	if(!fp)
	{
		printf("add error");
		exit(1);
	}
	fprintf(fp,"%s\t%s\t%s\n",p,name,msg);
	fclose(fp);
	fp = fopen("sys_logs.txt","a+");
	fprintf(fp,"%s %s对%s发出消息:%s\n",p,revname,name,msg);
	fclose(fp);
	
}
void save_msg1(char *msg,char *name,char *revname)
{
	FILE *fp;
	char *p;
	p=now_time();
	fp = fopen(revname,"at+");
	if(!fp)
	{
		printf("add error");
		exit(1);
	}
	fprintf(fp,"%s\t%s\t%s\n",p,name,msg);
	fclose(fp);
	fp = fopen("sys_logs.txt","a+");
	fprintf(fp,"%s %s对%s发出消息:%s\n",p,name,revname,msg);
	fclose(fp);
	
}


void print()
{
	linklist p = head;
	while(p->next != NULL)
	{
		
		printf("%s %s\n",p->data.username,p->data.password);
	}
}


/*发送给全部成员*/				
void send_all(msg buf)
{
	FILE *fp;
	char *q;
	onllist s, p;
	p = onlhead->next;
	q=now_time();
	fp = fopen("sys_logs.txt","a+");
	while(p!= NULL)
	{
		send(p->data.sock_id,&buf,sizeof(buf),0);
		buf.flag=0;
		save_msg1(buf.mess,buf.username,p->data.username);
		fprintf(fp,"%s %s收到%s发出消息:%s\n",q,p->data.username,buf.username,buf.mess);
		p = p->next;	
	}
	fclose(fp);
}


/*发送给一个人*/
void send_one(msg buf)
{
	FILE *fp;
	char *q;
	onllist s, p;
	p = onlhead->next;
	q=now_time();
	fp = fopen("sys_logs.txt","a+");
	while(p!= NULL && strcmp(p->data.username,buf.revname)!=0)
	{
		p = p->next;	
	}
	char temp[40]= "    此用户不在线";
	if(p!=NULL)
	{
		 send(p->data.sock_id,&buf,sizeof(buf),0);
		 fprintf(fp,"%s %s收到%s发出消息:%s\n",q,buf.username,buf.revname,buf.mess);
		 save_msg(buf.mess,buf.username,buf.revname);
	}
	else
	{
		p = onlhead->next;
		while(p!= NULL && strcmp(p->data.username,buf.username)!=0)
		{
			p = p->next;	
		}
		if(p!=NULL)
		{
			send(p->data.sock_id,temp,sizeof(temp),0);
		}
	}
	fclose(fp);
}


/*查看在线用户*/
void search_online(msg buf)
{
	onllist s, p;
	int id;
	p = onlhead->next;
	s = p;
	while(p!= NULL && strcmp(p->data.username,buf.username)!=0)
	{
		p = p->next;	
	}
	if(p!=NULL)
	{
		id = p->data.sock_id;
	}
	while(s!=NULL)
	{
		buf.flag=20;
		strcpy(buf.username,s->data.username);
		send(id,&buf,sizeof(buf),0);
		s= s->next;
	}
	
}


/*绑定地址和嵌套字*/
void ready(void)
{
	pid_t pid;
	sock_fd = socket(AF_INET,SOCK_STREAM,0);
	if(sock_fd<0)
	{
		perror("socket");
		exit(1);
	}
	struct sockaddr_in serv_addr;		//绑定端口
	serv_addr.sin_family = AF_INET;	
	serv_addr.sin_port = htons(MYPORT);					
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);			
	if(bind(sock_fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
	{
		perror("bind");
		exit(1);
	}
	if(listen(sock_fd,LISTEN_NUM)<0)		//监听套接字
	{
		perror("listen");
		exit(1);
	}
}


/*新连接产生后处理的函数*/
void *thread(void *p)
{
	int fd=*(int *)p;
	printf("pthread=%d\n",fd);
	team[size]=fd;
	printf("team=%d\n",team[size]);
	size++;
	while(1)
    	{	
		msg recv_buf;
		if(recv(fd,&recv_buf, sizeof(recv_buf),0)==0)
		{
			//返回0,表示TCP另一端断开链接
          		//有客户端退出
			printf("the %d quit\n",fd);
			del_online(fd);
			close(fd);
			return;
		}
		switch(recv_buf.flag)
		{
			case REGSINER1:
				isused(recv_buf,fd);
				break;
			case  REGSINER2:
				add_newuser(recv_buf);
				break;
			case LOGIN:
				islogin(recv_buf,fd);			
				break;
			case ALL:
				send_all(recv_buf);
				break;
			case ONE:
				send_one(recv_buf);
				break;
			case ONLINE:
				search_online(recv_buf);
				break;

		}
  	}
}


void service(void)
{

	while(1)
	{
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_fd = accept(sock_fd,(struct sockaddr *)&client_addr,&client_len);		//接受连接请求,产生连接套接字			
		printf("accept a new client,is %d\n",client_fd);
		if(client_fd<0)
		{
			printf("error to connect\n");
          		continue;//继续循环，处理连接
		}
		printf("a new connecter is %d\n",client_fd);
		pthread_t pid;
       		pthread_create(&pid,0,thread,&client_fd);
	}	
}


/*关闭服务器的socket*/
void sig_close()
{
	sys_logs(10);
	close(sock_fd);
	printf("服务器已经关闭\n");
	save_user(head);
	exit(0);
}


int main()
{
	sys_logs(9);
	signal(SIGINT,sig_close);//当产生前面参数的信号时,调用后面的处理函数(返回值为正值的一个函数)
	ready();
	read_user();
	create_online();
	service();
	return 0;
}	
