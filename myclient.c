#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>
#include <signal.h>
#include <pthread.h>  
#include  <time.h>


#define MYPORT 4500
#define REGSINER1 1			//注册名判断
#define REGSINER2 2			//注册名传递
#define LOGIN 3				//登陆判断
#define ALL 4				//全体消息
#define ONE 5				//私聊
#define ONLINE 6			//查看在线用户
#define ADD 7

void features(char *name);
int conn_fd;				//套接口描述符

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

struct msg
{

	int	flag;			//标记
	char	mess[1000];		//消息内容
	char	revname[20];		//接收消息的用户名

	char	username[20];		//存储用户名		
	char	password[20];		//存储密码
	
};

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


/*自定义错误处理函数*/
void my_err(const char *err_string, int line)
{
	fprintf(stderr, "line:%d ", line);
	perror(err_string);
	exit(1);
}


void ready(void)
{		
	printf("启动连接\n");
	struct sockaddr_in serv_addr;			//创建套接字
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));	//初始化
	conn_fd = socket(AF_INET,SOCK_STREAM,0);	//设置端口和地址
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port=htons(MYPORT);
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(connect(conn_fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)				//连接服务器端地址
	{
		my_err("connect", __LINE__);
	}
}

/*用户名合法性检查*/
int  check(char *username)
{
		int i = 0;
		int flag =0;
		for(i=0; username[i] != '\0'; i++)
		{
			if (ispunct(username[i]) != 0)
			{
				printf("您输入了非法字符,请重新输入!\n");
				memset(username, 0, sizeof(username));
				flag =1;
				break;
			}
		}

	return flag;
}


/*收消息*/
void* recv_thread(void* p)
{
	char *q;
	q=now_time();
	while(1)
	{
		struct msg buf;
        	if(recv(conn_fd,&buf,sizeof(buf),0)<=0)
		{
            		return;
        	}
		if (buf.flag==20)
		{
			printf("%s\t%s\n",q,buf.username);
		}
		else
        		printf("%s\t%s\n%s\n",q,buf.username,buf.mess);
	}
}



/*用户注册*/
int regsiner()
{	
	char username[20];
	char password1[20];
	char password2[20];
	char buf[10],*p;
	struct msg regser;
		while(1)
		{
			printf("请输入用户名(12位以内):\n");
			memset(username,'\0',sizeof(username));
			setbuf(stdin, NULL);
			scanf("%s",username);
			if (username[0] == '\0')
			{
				printf("用户名不能为空!\n");
				continue;
			}
			if(strlen(username)>12)
			{
				printf("长度不符,请重新输入\n");
				continue;
			}
			int flag;
			flag=check(username);
			while (flag ==1)	
			{
					printf("请输入用户名(12位以内):\n");
					memset(username, '\0', sizeof(username));
					setbuf(stdin, NULL);
					scanf("%s", username);
					if (username[0] == '\0')
					{
						printf("用户名不能为空!\n");
						continue;
					}
					if(strlen(username) > 12)
					{
						printf("长度不符,请重新输入!\n");
						continue;
					}
					flag =check(username);
			}	
			strcpy(regser.username,username);
			regser.flag = REGSINER1;		//判断用户名是否重复
			if (send(conn_fd,&regser,sizeof(regser),0) < 0)
			{
				my_err("send",__LINE__);
			}
			if (recv(conn_fd,buf,10,0) < 0)	//根据服务器返回的消息判断
			{
				my_err("recv",__LINE__);
			}
			if(strcmp(buf,"OK")!=0)
			{
				sys_logs(1);
				printf("账号已经被使用.\n");
				continue;
			}
			else
			{
				break;
			}	
		}
		do{
			do{
				p=getpass("请输入密码(12位以内):\n");
				memset(password1,'\0',sizeof(password1));
				strcpy(password1,p);
			   	if (password1[0] == '\0')
				{
					printf("密码不能输入空格!\n");
						continue;
				}
			  }while(strlen(password1)>12);
			  p=getpass("请再次输入密码(12位以内):\n");
		 	  memset(password2,'\0',sizeof(password2));
			  strcpy(password2,p);
		   }while((strcmp(password1,password2)!=0));
		   regser.flag = REGSINER2;
		   strcpy(regser.password,password1);
		   if (send(conn_fd,&regser,sizeof(regser),0) < 0)
		   {
			my_err("conn_fd",__LINE__);
		   }
		   sys_logs(2);
		   printf("注册成功\n");
		   return 0;
}

void menu();
/*用户登录*/
void login()
{
	char	username[20];
	char	password[20];
	char	buf[10],*p;
	int	i=3;
	while(i>0)
	{
		fflush(stdin);
		printf("请输入用户名:\n");
		memset(username,'\0',sizeof(username));
		scanf("%s", username);
		if (strlen(username)<12)
		{
			p=getpass("请输入密码:\n");
			if (strlen(password)<12)
			{
				printf("正在登录...\n");
				struct msg temp;
				temp.flag=LOGIN;		//登陆判断
				strcpy(temp.username,username);
				strcpy(temp.password,p);
				if (send(conn_fd,&temp,sizeof(temp),0) < 0)
				{
					my_err("send",__LINE__);
				}
				if (recv(conn_fd,buf,sizeof(buf),0) < 0)
				{
					my_err("recv",__LINE__);
				}
				if(strcmp(buf,"success")==0)
				{
					break;
				}
				else
				{
					i--;
					sys_logs(4);
					printf("输入有误,请重新输入\n剩余登录次数%d\n",i);
				}
			}
			else
			{
				sys_logs(4);
				printf("密码长度过长\n剩余登录次数%d\n",--i);
			}
		}
		else
		{
			printf("用户名长度过长\n剩余登录次数%d\n",--i);
			sys_logs(5);		
		}
	}
	if (i!=0)
	{
		features(username);
		sys_logs(3);	
	}
	else
		menu();
	sys_logs(8);
}	

/*关闭客户端的描述符*/
void sig_close()
{
	sys_logs(7);
    	close(conn_fd);
   	exit(0);
}
/* 好友储存*/
void save_add(char *msg,char *name)
{
	FILE *fp;
	char str[40];
	sprintf(str,"%sadd",name);
	fp = fopen(str,"at+");
	fprintf(fp,"%s\n",msg);
	fclose(fp);
}
/*好友遍历*/
int print_add(char *msg,char *name)
{
	FILE *fp,*ff,*fa;
	char *p;
	char str[40];
	char names[20];
	p=now_time();
	sprintf(str,"%sadd",name);
	fa = fopen(str,"a+");
	fclose(fa);
	fp = fopen(str,"rt+");
	while(1)
	{
		if (feof(fp))
		{
			ff=fp;
			if(feof(ff))
				break;
		}
		fscanf(fp,"%s\n",names);
		if (strcmp(names,msg)==0)
		{
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

void features(char *name);

/*储存发送消息的函数*/
void save_msg(char *msg,char *name)
{
	FILE *fp;
	char *p;
	p=now_time();
	fp = fopen(name,"at+");
	if(!fp)
	{
		printf("add error");
		exit(1);
	}
	fprintf(fp,"%s %s %s\n",p,name,msg);
	fclose(fp);
}
char timess[20];
/*读取好友请求*/
int print_Ad(char *name)
{
	FILE *fp,*ff,*fa;
	int i= 0;
	char buf[1000];
	char names[20];
	sprintf(names,"%saddQ",name);
	fp = fopen(names,"r+");
	if(fp == NULL)
	{
		printf("无好友请求\n");
		getchar();
		printf("请按回车继续:\n");
		getchar();
		features(name);
	}
	else
	{
		while(1)
		{
			if (feof(fp))
			{
				ff=fp;
				if(feof(ff))
					break;
			}
			fscanf(fp,"%s\n",buf);
			printf("%s请求添加好友\n",buf);
		}
		fclose(fp);
		printf("请输入要回复的好友请求:\n");
		scanf("%s",timess);
		fp = fopen(names,"r+");
		while(1)
		{
			if (feof(fp))
			{
				ff=fp;
				if(feof(ff))
					break;
			}
			fscanf(fp,"%s\n",buf);
			if (strcmp(buf,timess)==0)
			{
				save_add(name,timess);
				save_add(timess,name);
				return 1;
			}
			else
			{
				printf("无此好友请求\n");
				getchar();
				printf("请按回车继续:\n");
				getchar();
				features(name);
			}		
		}
		fclose(fp);
	}
	return 0;
}

/*删除好友*/
void save_dle(char *name,char *revname)
{
	FILE *fp,*ff;
	char names[20];
	char str[50][20];
	int i=0,j=0;
	sprintf(names,"%sadd",name);
	fp = fopen(names,"rt");
	while(1)
	{
		if (feof(fp))
		{
			ff=fp;
			if(feof(ff))
				break;
		}
		fscanf(fp,"%s\n",str[i]);
		i++;
	}
	fclose(fp);
	if(i==1)
	{
		if (strcmp(str[j],revname)==0)
		{
			remove(names);
		}
		else
		{
			printf("无此好友!\n");
			getchar();
			printf("请按回车继续:\n");
			getchar();
			features(name);
		}
	}
	else
	{
		fp = fopen(names,"wt+");
		while(j<i)
		{
			if (strcmp(str[j],revname)==0)
			{
				j++;
				continue;
			}
			else
			{
				fprintf(fp,"%s\n",str[j]);
				j++;
			}
		}
		fclose(fp);
	}
	
}

/*删除已同意的好友请求*/
void print_dl(char *name)
{
	FILE *fp,*ff;
	char names[20];
	char str[50][20];
	int i=0,j=0;
	sprintf(names,"%saddQ",name);
	fp = fopen(names,"rt");
	while(1)
	{
		if (feof(fp))
		{
			ff=fp;
			if(feof(ff))
				break;
		}
		fscanf(fp,"%s\n",str[i]);
		i++;
	}
	fclose(fp);

	if(i==1)
	{
		if (strcmp(str[j],timess)==0)
		{
			remove(names);
		}
		else
		{
			printf("无此好友!\n");
			getchar();
			printf("请按回车继续:\n");
			getchar();
			features(name);
		}
	}
	else
	{
		fp = fopen(names,"wt+");
		while(j<i)
		{
			if (strcmp(str[j],timess)==0)
			{
				j++;
				continue;
			}
			else
			{
				fprintf(fp,"%s\n",str[j]);
				j++;
			}
		}
		fclose(fp);
	}
	
	
}

/*显示消息记录*/
void print_msg(char *name)
{
	FILE *fp,*ff,*fa;
	char buf[1000];
	char times[20];
	char names[20];
	fp = fopen(name,"rt+");
	if(fp == NULL)
	{
		printf("无消息记录\n");
		getchar();
		printf("请按回车继续:\n");
		getchar();
		features(name);
	}
	else
	{
		while(1)
		{
			if (feof(fp))
			{
				ff=fp;
				if(feof(ff))
					break;
			}
			fscanf(fp,"%s\t%s\t%s\n",times,names,buf);
			printf("%s\n%s\n%s\n",times,names,buf);
		}
		getchar();
		printf("请按回车继续:\n");
		getchar();
	}
	fclose(fp);
}

void print_hy(char *name)
{
	FILE *fp,*ff;
	char names[20];
	char buf[20];
	int i=0,j=0;
	sprintf(names,"%sadd",name);
	fp = fopen(names,"rt");
	if (fp == NULL)
	{
		printf("无好友\n");
		getchar();
		printf("请按回车继续:\n");
		getchar();
		features(name);
	}
	else
	{	
		printf("好友列表:\n");
		while(1)
		{
			if (feof(fp))
			{
				ff=fp;
				if(feof(ff))
					break;
			}
			fscanf(fp,"%s\n",buf);
			printf("\n%s\n",buf);
		}
	}
	getchar();
	printf("请按回车继续:\n");
	getchar();
	fclose(fp);
}


/*登陆成功后的界面*/
void features(char *name)
{
		//发送消息
		//发消息之前，启动一个线程,用来接受服务器发送过来的消息
		FILE *fp,ff;
		pthread_t pid;
		struct msg mes;
		char nnme[20];
		pthread_create(&pid,0,recv_thread,0);
		sys_logs(3);
		int i,x;
		while(1)
		{
			system("clear");	
			printf("\033[0;36m欢迎用户%s!!!\033[0m\n",name);
                	printf("\033[0;35m-----------------\033[0m\n");
			printf("\033[0;36m1.给所有人发消息!\033[0m\n");
                	printf("\033[0;36m2.给一个人发消息!\033[0m\n");
                	printf("\033[0;36m3.查看在线用户!  \033[0m\n");
			printf("\033[0;36m4.查看聊天记录!  \033[0m\n");
			printf("\033[0;36m5.查看好友请求!  \033[0m\n");
			printf("\033[0;36m6.查看好友列表!  \033[0m\n");
			printf("\033[0;36m7.添加好友!      \033[0m\n");
			printf("\033[0;36m8.删除好友!      \033[0m\n");
                	printf("\033[0;36m9.退出!          \033[0m\n");
			printf("\033[0;35m-----------------\033[0m\n");
			printf("\033[0;36m请选择相应的序号: \033[0m\n");
			scanf("%d",&i);
			strcpy(mes.username,name);
			switch(i)
			{
				
				case 1:
					mes.flag = ALL;
					printf("请输入消息内容:");
					scanf("%s",mes.mess);
					save_msg(mes.mess,name);
					send(conn_fd,&mes,sizeof(mes),0);
					break;
				case 2:
					mes.flag = ONE;
					printf("请输入消息内容:");
					scanf("%s",mes.mess);
					printf("请输入发送的用户名");
					scanf("%s",mes.revname);
					if (print_add(mes.revname,mes.username)==0)
					{
						printf("无此好友\n");
						getchar();
						printf("请按回车继续:\n");
						getchar();
					}
					else
					{
						save_msg(mes.mess,name);
						send(conn_fd,&mes,sizeof(mes),0);
					}	
					break;
				case 3:	mes.flag = ONLINE;
					send(conn_fd,&mes,sizeof(mes),0);
					getchar();
					getchar();
					break;	
				case 4:print_msg(name);break;
				case 5: x=print_Ad(name);
					mes.flag = ONE;
					if (x==1)
					{
						sprintf(mes.mess,"%s同意添加好友",name);
						strcpy(mes.revname,timess);
						send(conn_fd,&mes,sizeof(mes),0);
					}
					print_dl(name);
				 	break;
				case 6:print_hy(name);break;
					
				case 7:
					mes.flag = ONE;
					printf("请输入添加用户名");
					scanf("%s",mes.revname);
					sprintf(nnme,"%saddQ",mes.revname);
					fp = fopen(nnme,"a+");
					fprintf(fp,"%s\n",mes.username);
					fclose(fp);
					sprintf(mes.mess,"%s请求添加好友",name);
					send(conn_fd,&mes,sizeof(mes),0);
					break;
				case 8:
					printf("请输入删除删除名:");
					scanf("%s",mes.revname);
					save_dle(name,mes.revname);
					save_dle(mes.revname,name);
					sprintf(mes.mess,"%s将你移除好友列表",name);
					send(conn_fd,&mes,sizeof(mes),0);
					break;
				case 9:sig_close();sys_logs(7);break;
			}
				
		}
	
}


void menu()
{
         int number;
         do
         {
                  system("clear");
                  printf("\033[0;35m-----------------------------------\033[0m\n");
                  printf("\033[0;36m           欢迎来到聊天室          \033[0m\n");
                  printf("\033[0;36m           1.注册                  \033[0m\n");
                  printf("\033[0;36m           2.登录                  \033[0m\n");
                  printf("\033[0;36m           3.退出                  \033[0m\n");
                  printf("\033[0;35m-----------------------------------\033[0m\n");
                  printf("\033[0;36m请选择:\033[0m\n");
                  setbuf(stdin, NULL);
                 scanf("%d", &number);
                 setbuf(stdin, NULL);
         }while(number!=1&&number!=2&& number!=3);
	 switch(number)
	 {
	
		case 1:regsiner();menu();break;		
		case 2:login();break;
		case 3:sig_close();break;
	 } 
}


int main()
{

	signal(SIGINT,sig_close);//关闭CTRL+C	
	ready();
	menu();
	return 0;	
}
