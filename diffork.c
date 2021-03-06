#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int globVar = 5;

int main(void)
{
	pid_t pid;
	int var = 1,i;
	
	printf("fork is different with vfork\n");

	pid = fork();
	/*pid = vfork();*/
	switch(pid)
	{
		case 0:
			i = 3;	
			while(i-->0)
			{
				printf("child process is running\n");
				globVar++;
				var++;
				sleep(1);
			}
			printf("child's globVar = %d,var = %d\n",globVar,var);
			break;
		case -1:
			perror("process creation failed\n");
			exit(0);
		default:
			i=5;
			while(i-- > 0)
			{
				printf("parent process is running\n");
				globVar++;
				var++;
				sleep(1);
			}
			printf("parent's globVar = %d, var = %d\n", globVar, var);
			exit(0);
	}
}
