#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#define NUM_CHILD 3

void childProcess(){
    printf("child [%ld]: parent %ld\n",(long)getpid(), (long)getppid());
    sleep(10);
    printf("child [%ld]: Finished Execution\n",(long)getpid());
}

int main(int argc, char* argv[]) {

    int count = 0;

    while (count < NUM_CHILD) //Creating NUM_CHILD child process
    {
        int processId = fork();

        if (processId == -1) // If child process is not created correctly
        {
            printf("parent [%ld]: Unexpected Error from Fork\n",(long)getpid());
            signal(SIGTERM,SIG_IGN); // To ignore SIGTERM signal in the parent
            kill(0,15); // Sending SIGTERM(15) signal to every process in the process group of the parent
            return 1;
        }
        
        if (processId == 0) //Checking if we are in child process
        {
            childProcess(); //Printing relavant information
            exit(0); //Immediately exiting to not execute any further within the child process
        }

        sleep(1); // one second delay between consecutive fork calls

        count++;

    }

    count = 0;
    while(1)
    {
        errno = 0;
        if(wait(NULL) == -1) //Checking if errno has been set to ECHILD meaning that the parent process has no more children left
        {
            if(errno == ECHILD)
            {
                printf("parent [%ld]: No more child process left. Number of children terminated = %d\n",(long)getpid(),count);
                return 0;
            }
            else
            {
                printf("parent [%ld]: Unexpected Error from wait\n",(long)getpid());
                break;
            }
            
        }
        else 
        {
            count++;
            continue;
        }
    }

    return 0;

}