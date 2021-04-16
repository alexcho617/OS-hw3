//21800691 Sungjin Cho Operating Systems Homework 3
//worker.c
//Please look at the github readme file for execution.
//must execute manager first
//Execute worker in separate terminal
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/file.h>
#include <fcntl.h>
#include <errno.h>

#define READ 0
#define WRITE 1
int pipes[2];

int main()
{

	//GETTING DIRECTORY PART HERE
	int fd = open("task", O_RDONLY | O_SYNC);
	char dir[512];
	int len;
	if ((len = read(fd, dir, 512)) == -1)
		exit(1);
	close(fd);

	//SEARCHING HERE
	DIR *folder;
	struct dirent *entry;
	folder = opendir(dir); //absolute directory given to opendir

	if (folder == NULL) //file fail check
	{
		perror("Unable to read directory\n");
		exit(1);
	}
	char currentdir[2] = ".";
	char parentdir[3] = "..";
	int arrindex = 0;

	while ((entry = readdir(folder)))
	{ //filter current and parent directories
		char filetoread[200] = "";
		if (strcmp(entry->d_name, currentdir) != 0 && strcmp(entry->d_name, parentdir) != 0)
		{
			strcat(filetoread, dir);
			strcat(filetoread, "/");
			strcat(filetoread, entry->d_name);
			//printf("%s\n", filetoread);
			
		/*Gotta Differentiante text FILES*/
			int ptrCount = 0;
			pid_t childPid = fork();
			if (childPid == 0) /*child*/
			{
				dup2(pipes[WRITE], 1);
				execl("/usr/bin/file", "file", filetoread, (char *)NULL); //not properly executed
			}
			else /*parent*/
			{
				char buf[10001];
				ssize_t s;
				close(pipes[WRITE]);

				while ((s = read(pipes[READ], buf, 10000)) > 0) //saved all result to buf
				{
					buf[s + 1] = 0x0;
					printf("test\n");
				}
				wait(0);
			}
		}//filter for directory
	}//readdir
	closedir(folder); //folder contents exhausted

	return 0;
}