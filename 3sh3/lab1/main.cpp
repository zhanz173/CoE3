#include <iostream>
#include <signal.h>
#include <unistd.h>

using namespace std;

class handler{
public:   
    handler(const handler&) = delete;

    static void  alarm_handler(int);
    static void  interupt_handler(int);
    static void  stop_handler(int);

    private:
        handler();
        static handler h;
};
handler handler::h;

handler::handler(){
    if (signal(SIGALRM, alarm_handler) == SIG_ERR)
    {
        cout << "failed to register alarm handler.\n";
        exit(1);
    }

    if (signal(SIGINT, interupt_handler) == SIG_ERR){
        cout << "failed to register interupt handler.\n";
        exit(1);
    }

    if (signal(SIGTSTP, stop_handler) == SIG_ERR){
        cout << "failed to register stop handler.\n";
        exit(1);
    }
}

void handler:: alarm_handler(int signo){
    if (signo == SIGALRM){
           cout << "alarm\n";
    }
    
}

void handler::interupt_handler(int signo){
    if (signo == SIGINT){
        cout << "CRTL+C pressed!\n";
    }
}

void handler::stop_handler(int signo){
    if (signo == SIGTSTP){
        cout << "CRTL+Z pressed!\n";
    }
}


int main(){
    cout.setf(ios::unitbuf);
    pid_t pid = getpid();
    cout << "current pid: " << pid << "\n";

    while(1){
        raise(SIGALRM);
        sleep(2);}

    return 0;
}