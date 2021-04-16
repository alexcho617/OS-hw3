//21800691 Sungjin Cho Operating Systems Homework 3
//manager.c
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

//global variables
int sizeTasks = 0;
bool caseFlag = false;
bool absoluteFlag = false;
int pipes[2];
char taskList[1000][1000];

int main(int argc, char *argv[])
{

    int opt; //getopt variables
    extern char *optarg;
    extern int optind;
    int procNumber = 2;  //default proc value
    char *userDirectory; //from user input command line
    DIR *rootdir;
    pid_t worker_pid;

    /*optarg stuff here*/
    while ((opt = getopt(argc, argv, "p:ca")) != -1)
    {
        switch (opt)
        {
        case 'p':
            procNumber = atoi(optarg);
            if (procNumber < 1 || 7 < procNumber)
            { //if smaller then 1 or greater then 7
                printf("Process Number must be between 1~7\n");
                exit(1);
            }
            else if (procNumber != 2)
            {
                printf("Using %d Process\n", procNumber);
            }
            break;
        case 'c':
            //case sensitive disabled
            printf("Case Insensitive\n");
            caseFlag = true;
            break;
        case 'a':
            printf("Absolute Path\n");
            absoluteFlag = true;
            break;
        case '?':
            printf("Non-option argument");
            exit(1);
        default:
            printf("error in getopt from default case");
            break;
        }
    }
    if (procNumber == 2)
        printf("Using default:%d Process\n", procNumber);
    //get directory from argv
    userDirectory = argv[optind];
    if (!(rootdir = opendir(userDirectory)))
    {
        exit(1);
        printf("Unable to Open root directory.\n");
    }
    //printf("Directory: %s\n", userDirectory);

    char *keywords[argc - 1 - optind]; //to store keywords from argv
    int j = 0;
    for (int i = optind + 1; i < argc; i++) //repeat as many times as number of argument
    {
        keywords[j] = argv[i];
        j++;
    }
    int sizeKeywords = sizeof(keywords) / sizeof(keywords[0]);
    if (sizeKeywords == 0)
    {
        printf("Keyword Error\n");
        exit(1);
    }



    /*file using unnamed pipe*/
    struct dirent *entry;
    if (!(rootdir = opendir(userDirectory)))
    {
        exit(1);
        printf("Unable to Open root directory.\n");
    }
    pipe(pipes);
    if (pipe(pipes) != 0)
        perror("Pipe Error");
    int ptrCount = 0;
    pid_t childPid = fork();
    if (childPid == 0) /*child*/
    {
        dup2(pipes[WRITE], 1);
        execl("/usr/bin/tree", "tree", userDirectory, "-d", "-fi", (char *)NULL); //not properly executed
    }
    else /*parent*/
    {
        char buf[10001];//string to store all returned from tree command
        ssize_t s;
        close(pipes[WRITE]);

        while ((s = read(pipes[READ], buf, 10000)) > 0) //saved all result of tree
        {
            buf[s + 1] = 0x0;
        }

        char *ptr = strtok(buf, "\n");
        while (ptr != NULL)
        {
            strcpy(taskList[ptrCount], ptr);
            ptr = strtok(NULL, "\n");
            ptrCount++;
        }
        wait(0);
        //printf("ptrCount: %d\n", ptrCount);
        sizeTasks = ptrCount - 1;
    }

    //keywords print
    printf("%d Keys Received\n", sizeKeywords);
    for (int i = 0; i < sizeKeywords; i++)
    {
        printf("Key %d: %s\n",i+1, keywords[i]);
    }
    printf("\n\n");

    //tasks print
    printf("%d Tasks Available\n", sizeTasks);
    for (int i = 0; i < sizeTasks; i++)
    {
        if (taskList[i] != NULL)
            printf("Task %d: %s\n",i+1,taskList[i]);
    }
    printf("\n");




    /*---fifo created here--*/
    if (mkfifo("task", 0666))
    {
        if (errno != EEXIST)
        {
            perror("fail to open fifo: ");
            exit(1);
        }
    }
    int taskNumber = 1;


    /*-----pipe writing here------*/
    printf("Sending Task %d to worker. \nTask %d: %s \n",
        taskNumber,taskNumber, taskList[taskNumber-1]);// -1 for index correction
    
    int fd = open("task", O_WRONLY | O_SYNC);

    char s[512];
    int i = 0;
    s[i - 1] = 0x0;
    strcpy(s, taskList[taskNumber-1]); // -1 for index correction
    if (s[0] == 0x0)
        exit(1);

    for (int i = 0; i < 512;)
    {
        i += write(fd, s + i, 512);
    }
    close(fd);
    return 0;
}