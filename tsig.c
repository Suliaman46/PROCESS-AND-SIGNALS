
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#define NUM_CHILD 5

#ifdef WITH_SIGNAL
int keyhit = 0;

    void termHandler(int sig)
    {
        printf("child [%ld]: SIGTERM SIGNAL RECIEVED, TERMINATING\n",(long)getpid());
        fflush(stdout);
    }
    void  kBInterrupt(int sig)
    {
        printf("parent [%ld]: Keyboard Interrupt\n",(long)getpid());
        fflush(stdout);
        keyhit = 1;
    }
#endif

void childProcess()
{
    printf("child [%ld]: parent %ld\n",(long)getpid(), (long)getppid());
    sleep(10);
    printf("child [%ld]: Finished Execution\n",(long)getpid());
    fflush(stdout);
}

int main() 
{
    #ifdef WITH_SIGNAL
        for(int i =1; i<NSIG;i++) // Ignoring all signals
        {
            signal(i,SIG_IGN);
        }
        signal(SIGCHLD,SIG_DFL); // Resetting SIGCHILD to defualt handler
        // signal(SIGINT, &kBInterrupt); //Setting custom handler for SIGINT

        // SIGACTION ALTRENATIVE
        struct sigaction new_action, old_action;
        new_action.sa_handler = &kBInterrupt;
        sigemptyset (&new_action.sa_mask);
        new_action.sa_flags = 0;
        sigaction(SIGINT,&new_action,NULL);
    #endif

    int count = 0;
    while (count < NUM_CHILD) //Creating NUM_CHILD child process
    {
        pid_t processId = fork();
        if (processId == 0) //Checking if we are in child process
        {
            #ifdef WITH_SIGNAL
                signal(SIGINT,SIG_IGN); 
                // signal(SIGTERM,&termHandler);
                // SIGACTION ALTRENATIVE 
                struct sigaction new_action, old_action;
                new_action.sa_handler = &termHandler;
                sigemptyset (&new_action.sa_mask);
                new_action.sa_flags = 0;
                sigaction(SIGTERM,&new_action,NULL);
            
            #endif
            childProcess(); //Printing relavant information
            exit(0); //Immediately exiting to not execute any further within the child process
        }

        if (processId == -1) // If child process is not created correctly
        {
            printf("parent [%ld]: Unexpected Error from Fork\n",(long)getpid());
            printf("parent [%ld]: Sending SIGTERM signal\n",(long)getpid());
            signal(SIGTERM,SIG_IGN); // To ignore SIGTERM signal in the parent
            kill(0,SIGTERM); // Sending SIGTERM(15) signal to every process in the process group of the parent
            return 1;
        }

        sleep(1); // one second delay between consecutive fork calls

        #ifdef WITH_SIGNAL
            if(keyhit == 1)
            {
                printf("parent [%ld]: Sending SIGTERM signal\n",(long)getpid());
                signal(SIGTERM,SIG_IGN); // To ignore SIGTERM signal in the parent
                kill(0,SIGTERM); // Sending SIGTERM(15) signal to every process in the process group of the parent
                break;
            }
        #endif

        count++;
    }

    #ifdef WITH_SIGNAL
        if(keyhit == 1) printf("parent [%ld]: Creation Process interuppted \n",(long)getpid());
        else printf("parent [%ld]: All children are created \n",(long)getpid());
    #else
        printf("parent [%ld]: All children are created \n",(long)getpid());
    #endif
    
    count = 0;
    while(1)
    {
        if(wait(NULL) == -1) //Checking if errno has been set to ECHILD meaning that the parent process has no more children left
        {
            if(errno == ECHILD) printf("parent [%ld]: No more child process left. Number of children terminated = %d\n",(long)getpid(),count);
            else    printf("parent [%ld]: Unexpected Error from wait\n",(long)getpid());
            break;
        }
        else count++;
    }

    #ifdef WITH_SIGNAL
        for(int i =1; i<NSIG;i++) // Resetting all signals to Default behaviour
        {
            signal(i,SIG_DFL);
        }
    #endif
    return 0;
}

