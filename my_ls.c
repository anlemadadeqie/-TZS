#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <errno.h>
#include <time.h>

#define PARAM_NONE	0
#define PARAM_A		1
#define PARAM_L		2
#define PARAM_I		4
#define MAXROWLEN	100
 
int	g_leave_len = MAXROWLEN;
int	g_maxlen;

void my_err(const char *err_string, int line)
{
	fprintf(stderr, "line:%d", line);
	perror(err_string);
	exit(1);
}

int display_file( struct stat buf, char *name)
{
	struct passwd *psd;
	struct group *grp;
	struct tm *tm;
	
/*打印文件的类型*/
	switch (buf.st_mode & S_IFMT)
	{
		case S_IFBLK : printf("b");	break;
		case S_IFCHR : printf("c");	break;
		case S_IFIFO : printf("p");	break;
		case S_IFLNK : printf("l");	break;
		case S_IFREG : printf("-");	break;
		case S_IFSOCK: printf("s");	break;
		case S_IFDIR : printf("d");     break;
	}

	
/*打印文件所有者对文件的存取权限*/
	if (buf.st_mode & S_IRUSR)
	{
		printf("r");
	}
	else
	{
		printf("-");
	}
	if (buf.st_mode & S_IWUSR)
	{
		printf("w");
	}
	else
	{
		printf("-");
	}
	if (buf.st_mode & S_IXUSR)
	{
		printf("x");
	}
	else
	{
		printf("-");
	}

/*打印与文件所有者同组用户对文件的存取权限*/
	if (buf.st_mode & S_IRGRP)
	{
		printf("r");
	}
	else
	{
		printf("-");
	}
	if (buf.st_mode & S_IWGRP)
	{
		printf("w");
	}
	else 
	{
		printf("-");
	}
	if (buf.st_mode & S_IXGRP)
	{
		printf("x");
	}
	else
	{
		printf("-");
	}

/*打印其它用户对文件的存取权限*/
	if (buf.st_mode & S_IROTH)
	{
		printf("r");
	}
	else
	{
		printf("-");
	}
	if (buf.st_mode & S_IWOTH)
	{
		printf("w");
	}
	else
	{
		printf("-");
	}
	if (buf.st_mode & S_IXOTH)
	{
		printf("x");
	}
	else
	{
		printf("-");
	}
	printf("  ");

	psd = getpwuid(buf.st_uid);
	grp = getgrgid(buf.st_gid);
	printf("%-6d ",(int) buf.st_nlink);
	printf("%-8s", psd->pw_name);
	printf("%-8s", grp->gr_name); 
	printf("%6d ", (int)buf.st_size);
	tm = localtime(&buf.st_ctime);
	printf("%8d-%2d-%2d %2d:%2d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_sec);
       	printf(" %s\n", name);
	
	return 0;
}


/*在没有使用-l选项时,打印出文件名,并保证对齐*/
int display_single( char *name)
{
	int i, len;
	
	if(g_leave_len < g_maxlen)
	{
		printf("\n");
		g_leave_len = MAXROWLEN;
	}
	
	len = strlen(name);
	len = g_maxlen -len;
	printf("%-s", name);
	
	for (i=0; i<len; i++)
	{	
		printf(" ");
	}
	printf("  ");
	g_leave_len -= (g_maxlen + 2);
}

 int display_sangle( char *name,struct stat buf)
 {
	int i, len;
        
	if(g_leave_len < g_maxlen)
        {
                 printf("\n");
                 g_leave_len = MAXROWLEN;
        }
 
	len = strlen(name);
        len = g_maxlen -len;
        printf("%6d ",buf.st_ino);
	printf("%-s", name);
 
        for (i=0; i<len; i++)
        {
                printf(" ");
        }
        printf("  ");
        g_leave_len -= (g_maxlen + 2);
 }

/* 
*	根据命令行参数和完整路径名显示目标文件	
*	参数 flag: 命令行参数
*       参数 pathname : 包含了文件名的路径名
*/
void display(int flag, char *pathname)
{
	int i, j;
	struct stat buf;
	char name[1000];
	
	/*从路径中解析出文件名*/
	for(i=0,j=0; i<strlen(pathname); i++)
	{
		if(pathname[i] == '/')
		{
			j = 0;
			continue;
		}
		name[j++] = pathname[i];
	}
	name[j] = '\0';
	
	if (lstat(pathname, &buf) == -1)
	{
		my_err("stat", __LINE__);
	}
	
	switch (flag)
	{
	case PARAM_NONE:                            //没有-a 和-l选项
		if (name[0] != '.')
		{
			display_single(name);
		}
		break;
	
	case PARAM_A:                               //-a 选项
			display_single(name);
			break;

	case PARAM_L:                               //-l 选项
		if (name[0] != '.')
		{
			display_file(buf, name);
		}	
		break;

	case PARAM_A + PARAM_L:                     //-a和-l 选项
			display_file(buf, name);
			break;
	
	case PARAM_I:				   //-i选项
		if (name[0] != '.')
		{
			display_sangle(name,buf);
		}
		break;
	
	default:
			break;
	}
	
}
 
		
int display_dir(int flag_param, char *path)
{
	DIR	*dir;
	struct dirent 	*ptr;
	int	count = 0;
	char	filenames[256][PATH_MAX+1],temp[PATH_MAX+1];
	
	//获取该目录下文件总数和最长的文件名
	dir = opendir(path);
	if (dir == NULL)
	{
		my_err("opendir",__LINE__);
	}
	while ((ptr=readdir(dir)) != NULL)
	{
		if (g_maxlen < strlen(ptr->d_name))
			g_maxlen = strlen(ptr->d_name);
		count++;
	}
	closedir(dir);

	if(count > 256)
	{
		my_err("too many files under this dir", __LINE__);
	}
	
	int i, j, k, len = strlen(path);
	dir = opendir(path);
	for (i=0;i<count; i++)
	{
		ptr = readdir(dir);
		if (dir == NULL)
		{
			my_err("readdir",__LINE__);
		}
		strncpy(filenames[i], path, len);
		filenames[i][len] = '\0';
		strcat(filenames[i], ptr->d_name);
		filenames[i][len+strlen(ptr->d_name)] = '\0';
	}
	
	//使用选择排序法对文件名进行排序,排序后文件名按字母顺序存储于filenames
	for (i = 0; i < count; i++)
        {		
		k=i;
		for (j = i+1; j < count; j++)
		{
			if (strcmp (filenames[k], filenames[j]) > 0)
			{
				k=j;
			}
			if (k != i)
			{
				strcpy (temp, filenames[k]);
				temp[strlen(filenames[k])] = '\0';
				strcpy (filenames[k], filenames[i]);
				filenames[k][strlen(filenames[i])] = '\0';
				strcpy (filenames[i], temp);
				filenames[i][strlen(temp)] = '\0';
			}
		}
	}
	
	for (i = 0; i < count; i++)
		display(flag_param, filenames[i]);

	closedir(dir);
	
	//如果命令行中没有-l选项,打印一个换行符
	if ((flag_param & PARAM_L) == 0)
		printf("\n");

	return 0;
}


int display_dirn(int flag_param, char *path)
 {
         DIR     *dir;
         struct dirent   *ptr;
         int     count = 0;
         char    filenames[256][PATH_MAX+1],temp[PATH_MAX+1];
 
         //获取该目录下文件总数和最长的文件名
         dir = opendir(path);
         if (dir == NULL)
         {
                 my_err("opendir",__LINE__);
         }
         while ((ptr=readdir(dir)) != NULL)
         {
                 if (g_maxlen < strlen(ptr->d_name))
                         g_maxlen = strlen(ptr->d_name);
                 count++;
         }
         closedir(dir);
 
         if(count > 256)
         {
                 my_err("too many files under this dir", __LINE__);
         }
 
         int i, j, k, len = strlen(path);
         dir = opendir(path);
         for (i=0;i<count; i++)
         {
                 ptr = readdir(dir);
                 if (dir == NULL)
                 {
                         my_err("readdir",__LINE__);
                 }                 
		strncpy(filenames[i], path, len);
                 filenames[i][len] = '\0';
                 strcat(filenames[i], ptr->d_name);
                 filenames[i][len+strlen(ptr->d_name)] = '\0';
         }

	         //使用选择排序法对文件名进行排序,排序后文件名按字母顺序存储于filenames
         for (i = 0; i < count; i++)
         {
                 k=i;
                 for (j = i+1; j < count; j++)
                 {
                         if (strcmp (filenames[k], filenames[j]) < 0)
                         {
                                 k=j;
                        }
                         if (k != i)
                         {
                                 strcpy (temp, filenames[k]);
                                 temp[strlen(filenames[k])] = '\0';
                                 strcpy (filenames[k], filenames[i]);
                                 filenames[k][strlen(filenames[i])] = '\0';
                                 strcpy (filenames[i], temp);
                                 filenames[i][strlen(temp)] = '\0';
                         }
                 }
         }
 
         for (i = 0; i < count; i++)
                 display(flag_param, filenames[i]);
 
         closedir(dir);
 
         //如果命令行中没有-l选项,打印一个换行符
         if ((flag_param & PARAM_L) == 0)
                 printf("\n");
 
         return 0;
 }

int main( int argc, char **argv)
{
	int	i, j, k, num;
	char	path[PATH_MAX+1];
	char	param[32];		//保存命令行参数,目标文件名和目录名不在此列
	int	flag_param = PARAM_NONE;	//参数种类即是否有-l和-a选项
	struct stat	buf;

	/*命令行参数的解析,分析-l,-a,-al,-la,-i选项*/
	j = 0;
	num = 0;
	for (i=1; i<argc; i++)
	{
		if(argv[i][0] == '-')
		{
			for (k=i; k<strlen(argv[i]); k++,j++)
			{
				param[j] = argv[i][k];		//获取-后面的参数保存到数组param中
			}
		num++;
		}
	}

	for(i=0; i<j; i++)
	{
		if (param[i] =='a')
		{
			flag_param |= PARAM_A;
			continue;
		}
		else if (param[i] == 'l')
		{
			flag_param |= PARAM_L;
			continue;
		}
		else if (param[i] == 'i')
		{
			flag_param |= PARAM_I;
			continue;
		}
		else 
		{
			printf("my_ls : invalid option -%c\n", param[i]);
			exit(1);
		}
	}
	param[j] = '\0';

	//如果没有输入文件名或目录,就显示当前目录
	if ((num+1) == argc)
	{
		strcpy(path, "./");
		path[2] = '\0';
		display_dir(flag_param, path);
		return 0;
	}

	i=1;
	do{
		//如果不是目标文件名或目录,解析下一个命令行参数
		if (argv[i][0] == '-')
		{
			i++;
			continue;
		}
		else
		{	
			strcpy(path, argv[i]);
		
			//如果目标文件或目录不存在,报错并退出程序
			if (stat(path, &buf) == -1)
				my_err("stat",__LINE__);
		
			if (S_ISDIR(buf.st_mode))
			{
		 	//如果目录的最后一个字符不是'/',就加上'/'
				if ( path[ strlen(argv[i]) -1] != '/')
				{
					path [ strlen(argv[i]) ] !='/';
					path [ strlen(argv[i])+1] = '\0';
				}
				else
					path[ strlen(argv[i])] = '\0';
			
				display_dir(flag_param, path);
				i++;
			}
		}
	}while (i<argc);

	return 0;
}





	
