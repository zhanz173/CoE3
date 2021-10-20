#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
    char END = 0xff;
    int fd[2], fd2[2],nbytes,sum = 0;
    char iobuffer;
    pid_t childpid;

    pipe(fd); pipe(fd2);

    if((childpid = fork()) == -1){
        perror("fork failed");
        exit(-1);
    }

    //child process
    if(childpid == 0){
        close(fd[0]);
        close(fd2[1]);
        while(1)
        {
            scanf("%c", &iobuffer);
            if(iobuffer == '-'){
                scanf("%c", &iobuffer);
                if(iobuffer == '1'){
                    write(fd[1],&END,1); 
                    break;
                }              
            }
            if(iobuffer != '\n')write(fd[1],&iobuffer,1); 
        }
        nbytes = read(fd2[0],&sum,sizeof(int));
        printf("inside child, sum =  %d\n", sum);
        exit(0);
    }    //parent process
    else
    {
        close(fd[1]);
        close(fd2[0]);
        while(nbytes = read(fd[0],&iobuffer,1)){
            if (iobuffer == END) break;
            sum += (iobuffer & 0x0F);
        }
        write(fd2[1],&sum,sizeof(int)); 
    }

    wait(NULL);
    return 0;
}