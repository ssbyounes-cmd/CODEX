#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main (int argc, char **argv)
{
    int pid = fork();
    if (pid == -1){
        return 1;
    }
    printf("Hello from process %d\n", getpid());
    if (pid != 0){
        wait(NULL);
    }
}