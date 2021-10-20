#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "sem.h"


int main(int argc, char *argv[]){
	size_t length;
    int fd, sem_id;
    struct stat mystat;

    // get semaphore
    sem_id = semget(key, 1, IPC_CREAT | 0600);

    if (sem_id == -1) {
        perror("Error - semget");
        exit(1);
    }

    // open file
    if((fd = open("res.txt", O_RDWR)) < 0){
		perror("Error - open\n");
		return 0;
	}

    // get file info and store in mystat
    if (fstat(fd, &mystat) < 0){
        perror("Error - fstat\n");
        close(fd);
        return 0;
    }

    // map file to address
	char* addr = static_cast<char*> (mmap(NULL, mystat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));	
    if (addr == MAP_FAILED) {
        perror("Error - mapped area");
        return 0;
    }
    
    // initialize semaphore to value of 0
    sem_attr.val = 0;
    if(semctl(sem_id, 0, SETVAL, sem_attr) == -1){
        perror("semctl");
        exit(1);
    };

    int a = semctl(sem_id,0, GETVAL,0);

	int resource = 0;
    unsigned char request = 0;

    while(true){
        std::cout << "Resource type: \n";
        std::cin >> resource;
        std::cout << "amount: \n";
        std::cin >> request;

        if((resource+1)*4 > mystat.st_size){
            std::cout << "this resource doesn't exist\n";
            continue;
        }

        // wait for semaphore to be zero and increment
        if(semop(sem_id, wait_use, 2) == -1){
            perror("semop failed");
            exit(1);
        }

        // modify file
        if (addr[resource*5 + 2] < request) {
            std::cout << "Not enough resources\n";

            if(semop(sem_id, post, 1) == -1) {
                perror("semop failed");
                exit(1);
            }
            
            break;
        }
        
        addr[resource*5 + 2] -= request - '0';
        msync(addr, mystat.st_size, MS_ASYNC | MS_INVALIDATE);

        // decrement semaphore for use
        if(semop(sem_id, post, 1) == -1){
            perror("semop failed");
            exit(1);
        }
    }

    semctl(sem_id, 0, SETVAL,ZEROS);
    // clear map
    munmap(addr, mystat.st_size);
}