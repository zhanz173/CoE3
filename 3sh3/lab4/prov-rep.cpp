#include <sys/types.h>
 #include <sys/prctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <signal.h>
#include "sem.h"

int main (int argc, char *argv[]) {
    size_t length;
    int fd, sem_id;
    struct stat mystat;

    // get semaphore
    sem_id = semget(key, 1, IPC_CREAT | 0600);

    if (sem_id == -1) {
        perror("Error - semget");
        exit(1);
    }

    // initialize semaphore to value of 0
    sem_attr.val = 0;
    if(semctl(sem_id, 0, SETVAL, sem_attr) == -1){
        perror("semctl");
        exit(1);
    };

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

    int a = semctl(sem_id,0, GETVAL,0);
    std::cout << "val: "<<a << std::endl;

    // map file to address
	char* addr = static_cast<char*> (mmap(NULL, mystat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));	
    if (addr == MAP_FAILED) {
        perror("Error - mapped area");
        return 0;
    }

    // create child process (display info about file)
    pid_t childpid;
    if ((childpid = fork()) == -1) {
        perror("fork");
    }

    // child process
    if (childpid == 0) {
        auto print = [](auto& i){
            if(i&0x01) std::cout << "address " << static_cast<int> (i) << " is in core\n";
            else std::cout << "address " << static_cast<int> (i) << " is not in core\n";};

        int size = getpagesize();
        int vec_size = mystat.st_size/size + 1;
        unsigned char* vec = new unsigned char[size];
        for(int i = 0; i < vec_size; i++) vec[i] = 0;
        while(true){
            if(semop(sem_id, wait_use, 2) == -1){
                perror("semop failed");
                exit(1);
            }
            getpagesize();
            msync(addr, mystat.st_size, MS_SYNC | MS_INVALIDATE);
            if(mincore(addr, mystat.st_size,vec)==-1) exit(1);
            for(int i = 0; i < mystat.st_size; i++) std::cout << addr[i];
            std::cout <<"\n page size: " << size << std::endl;
            for(int i = 0; i < vec_size; i++) print(vec[i]);

            pid_t id = getpid();
            std::cout << "child process sleep,id: " << id << std::endl; 
            if(semop(sem_id, post, 1) == -1){
                perror("semop failed");
                exit(1);
            }
            sleep(10);
        }
    
    }
    
    // parent process (provide resources)
    else {
        std::cout << childpid << std::endl;
        int resource;
        unsigned char request;
        while (true) {
            std::cout << "Resource type: \n";
            std::cin >> resource;
            std::cout << "Amount to add: \n";
            std::cin >> request;
            
            if  ((resource+1)*4 > mystat.st_size){
                std::cout << "this resource doesn't exist\n";
                continue;
            }

            // wait for semaphore to be zero and increment
            if(semop(sem_id, wait_use, 2) == -1) {
                perror("semop failed");
                exit(1);
            }
      
            addr[resource*5 + 2] += request - '0';
            msync(addr, mystat.st_size, MS_ASYNC | MS_INVALIDATE);

            // decrement semaphore for use
            if(semop(sem_id, post, 1) == -1){
                perror("semop failed");
                exit(1);
            }
        }
    }
}