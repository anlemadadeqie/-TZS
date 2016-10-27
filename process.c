/*************************************************************************
	> File Name: process.c
	> Author:jiafei 
	> Mail:hjh@xiyoulinux.org 
	> Created Time: 2016年10月20日 星期四 10时52分33秒
 ************************************************************************/

#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<signal.h>
#include<ctype.h>
#include<stdlib.h>


/*建立子进程最大数量*/
#define MAX_CHILD_NUMBER 10

/*子进程睡眠时间*/
#define SLEEP_INTERVAL 2

/*子进程的自编号，从0开始*/
int proc_number  = 0;

void do_something();

int main(int argc, char *argv[]){
    /*子进程个数*/
    int child_proc_number = MAX_CHILD_NUMBER;
    int i, ch;
    pid_t child_pid;
    pid_t pid[10] = {0};   //存放每个子进程的id

    if(argc > 1){
        child_proc_number = atoi(argv[1]);
        child_proc_number = (child_proc_number > 10)?10:child_proc_number;
    }
    

    for(i = 0; i < child_proc_number; i++){
        child_pid = fork();

        if(child_pid == 0){

            proc_number = i;
            do_something();

        }else if(child_pid == 1){

            printf("fork error!");
            return 1;
            
        }else{

            pid[i] = child_pid;
        }

    }

    /*让用户选择杀死进程，数字表示杀死该进程，q退出*/
    while((ch = getchar()) != 'q'){
        if(isdigit(ch)){
            kill(pid[ch='0'], SIGTERM );
        }
    }

    kill(0, SIGTERM);
    return 0;
}


void do_something(){
    for(;;){
        printf("this is process No.%d and its pid is %d\n", proc_number, getpid());
        sleep(SLEEP_INTERVAL);    //主动阻塞两秒钟
    }
}




