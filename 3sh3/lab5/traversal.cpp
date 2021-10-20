#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysmacros.h>
#include <pwd.h>
#include <grp.h>

using namespace std;

auto print = [](const char *name, int indent){
    cout << ">>";
    for(int i=0; i<indent; i++) cout << "\t";
    cout << name << endl;
};

bool listdir(const char *name, int indent){
    struct dirent *sd;
    DIR *dir;

    if (!(dir = opendir(name))){
        cerr << strerror(errno) << endl;
        return false;
    }

    while( (sd=readdir(dir)) != NULL ){
        if(sd->d_type == DT_DIR){
            if (strcmp(sd->d_name, ".") == 0 || strcmp(sd->d_name, "..") == 0)
                continue;
            string s = name;
            s.append("/");s.append(sd->d_name);
            listdir(s.c_str(),indent+1);
        }
        print(sd->d_name,indent);
    }
    closedir(dir);
    return true;
}

int main(int argc, char *argv[]){

    // file description structure
    struct stat sb;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (lstat(argv[1], &sb) == -1) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }

    printf("File type: ");

    switch (sb.st_mode & S_IFMT) {
        case S_IFBLK:  printf("block device\n");      break;
        case S_IFCHR:  printf("character device\n");  break;
        case S_IFDIR:  printf("directory\n");         break;
        case S_IFIFO:  printf("FIFO/pipe\n");         break;
        case S_IFLNK:  printf("symlink\n");           break;
        case S_IFREG:  printf("regular file\n");      break;
        case S_IFSOCK: printf("socket\n");            break;
        default:       printf("unknown?\n");          break;
    }

    printf("Mode: %lo (octal)\n", (unsigned long) sb.st_mode);
    printf("Link count: %ld\n", (long) sb.st_nlink);
    printf("Owner's name: %s\n", getpwuid(sb.st_uid)->pw_name);
    printf("Group name: %s\n", getgrgid(sb.st_gid)->gr_name);
    printf("File size: %lld bytes\n", (long long) sb.st_size);
    printf("Blocks allocated: %lld\n", (long long) sb.st_blocks);
    printf("Last file modification: %s", ctime(&sb.st_mtime));
    printf("Name: %s\n", argv[1]);
    
    string dname;
    cout << "Enter the name of the base dir\n";
    cin >> dname;

    if(listdir(dname.c_str(),0)){
        cout << "all dir listed, exiting\n";
    }else{
        do{
            cout << "Please enter a new dir(q to exit)\n";
            cin >> dname;
            listdir(dname.c_str(),0);
        }while(!dname.compare("q") && !dname.compare("Q"));
    }
    return 0;
}