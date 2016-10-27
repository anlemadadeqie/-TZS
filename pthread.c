/*************************************************************************
	> File Name: pthread.c
	> Author:jiafei 
	> Mail:hjh@xiyoulinux.org 
	> Created Time: 2016年10月25日 星期二 19时05分01秒
 *********************************************************************/


#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<ctype.h>
#include<pthread.h>
#include<semaphore.h>

#define MAX_THREAD 3													//线程的个数
unsigned long  main_counter,counter[MAX_THREAD];
pthread_mutex_t mutexa = PTHREAD_MUTEX_INITIALIZER;

void* thread_aworker(void*);
void *thread_worker(void *p)
{
	unsigned long thread_num;
    int ret;
	thread_num = (unsigned long)p;
	for(;;)														//无限循环
	{
		
        while(1){
            if((ret = pthread_mutex_lock(&mutexa)) == 0){
                counter[thread_num]++;
                main_counter++;
                usleep(10);
                pthread_mutex_unlock(&mutexa);
                break;
            }else{
                    
            }
        }
	}
}

int main(int argc,char *argv[])
{
	unsigned long  i, rtn, ch;
	pthread_t pthread_id[MAX_THREAD] = {0};						//存放线程ID
	for(i = 0; i < MAX_THREAD; i++)
	{
		rtn = pthread_create(&pthread_id[i],NULL,thread_worker,(void*)i);
	}
	do{															//用户按一次回车执行下面的循环体一次，按ｑ退出
		unsigned long  sum = 0;
		for(i = 0; i < MAX_THREAD; i++)								//求所有线程的counter的和
		{
			sum += counter[i];
			printf("第%ld个counter的值:%ld\n",i+1,counter[i]);
		}
		printf("main_counter的值/sum的值:%ld/%ld\n",main_counter,sum);
	}while((ch=getchar())!='q');
	return 0;
}
