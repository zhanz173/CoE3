#include <pthread.h>
#include <vector>
#include <atomic>
#include <fstream>
#include <condition_variable>
#include <cmath>
#include <mutex>
#include <iostream>
#include <unistd.h>

using namespace std;

atomic<int> finish;
bool ready = true;
mutex m;
condition_variable cv;
int** arr;
int phase = 1;

void my_swap(int &i, int &j)
{
    int temp = i;
    i = j;
    j = temp;
}

//bubble sort
void* sort(void* r)
{
    int row = *((int*)r);
    while(ready){
        
        // sort the rows
        if (phase%2 != 0) {
            // even row -> sort in increasing order
            if (row%2 == 0) {
                bool swap;
                for (int i = 0; i < 3; i++) {
                    swap = false;
                    for (int j = 0; j < 3 - i; j++) {
                        if (arr[row][j] > arr[row][j+1]) {
                            my_swap(arr[row][j], arr[row][j+1]);
                            swap = true;
                        }
                    }
                      if (!swap)
                         break;
                }
                
            } 
            //odd row -> sort in decreasing order
            else { 
                bool swap;
                for (int i = 0; i < 3; i++) {
                    swap = false;
                    for (int j = 0; j < 3 - i; j++) {
                        if (arr[row][j] < arr[row][j+1]) {
                            my_swap(arr[row][j], arr[row][j+1]);
                            swap = true;
                        }
                    }
                if (!swap)
                    break;
                }
            }
        }

        // sort the columns
        else {
            bool swap;
            for (int i = 0; i < 3; i++) {
                swap = false;
                for (int j = 0; j < 3-i; j++) {
                    if (arr[j][row] > arr[j+1][row]) {
                        my_swap(arr[j][row], arr[j+1][row]);
                        swap = true;
                    }
                }
                if (!swap)
                    break;
            }
        }
                
        unique_lock<mutex> lk(m);
        //finish.fetch_add(1,memory_order_relaxed);
        finish++;
        cv.wait(lk);       
    }
    return NULL;
}

inline void printArray(const int* const arr, const int size)
{
    for(auto i = 0; i < size; i++)
    {
        cout << arr[i] << "\t" ;
    }
}


int main()
{
    
    std::cout.setf(std::ios::unitbuf);
    vector<pthread_t> t;
    const string FILENAME = "input.txt";
    int row_size = 0;
    int n;
    int MAX_PHASE;
    int current_phase = 1;

    ifstream f(FILENAME);
    if (!f) return -1;

    while (f >> n)
    {
        row_size++;
    }
    MAX_PHASE = int(log2(row_size)) + 1;
    row_size = sqrt(row_size);
    f.clear();
    f.seekg(0);

    finish = 0;
    ready = true;

    arr = (int**)new int*[row_size];
    for (int i = 0; i < row_size; ++i) 
        arr[i] = new int[row_size];
 
    {
        auto i = 0, j = 0;
        while (f >> n)
        {
            arr[j][i++] = n;
            if (i >= row_size) {
                j++;
                i = 0;
            }
            
        }
    }

    int thread_count = row_size;

    int* args = new int[row_size];
    for(auto i=0; i<row_size; ++i)  args[i] = i;

    for(auto i=0; i<row_size; ++i) 
    {
        pthread_t pid;
        pthread_create(&pid, NULL, sort, &args[i]);
        t.push_back(std::move(pid));
    }

    while (phase < MAX_PHASE+1) {
        if (finish == thread_count) {
            {
                finish = 0;
                phase++;
                if(phase == MAX_PHASE+1){
                    ready = false;
                } 
            }
            asm volatile("mfence" ::: "memory");    
            cv.notify_all();
        }
    }

    for(auto &thread: t) 
    {
        pthread_join(thread, NULL);
    }

    cout << "final result:" << endl;
    for (int i = 0; i < row_size; ++i) {
        printArray(arr[i], row_size);
        cout << "\n";
    }
    delete[] arr;
    return 0;
}