#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"

#define IDLE 0
#define RUNNING 1

struct args{
    int thread_number;  // 스레드 번호
    int state;          // IDLE = 0, RUNNING = 1
    std::deque<int> workQueue; // 작업 queue
};


class request{
    public:
        request(int thread_number, int work_number) : thread_number(thread_number), work_number(work_number) {};    
        int thread_number; // request보낸 thread 번호
        int work_number;   // 작업 개수
};

/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial: public ITaskSystem {
    public:
        TaskSystemSerial(int num_threads);
        ~TaskSystemSerial();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn: public ITaskSystem {
    public:
        TaskSystemParallelSpawn(int num_threads);
        ~TaskSystemParallelSpawn();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();


        std::thread *pool = nullptr;

        void runTasks(IRunnable* runnable, int num_total_tasks, int numThread);
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSpinning(int num_threads);
        ~TaskSystemParallelThreadPoolSpinning();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();

        void spinning(int thread_number);

        std::thread *pool = nullptr;
        struct args *targs = nullptr;

        
        std::mutex *mtx = nullptr;
        std::mutex descriptor_mtx;
        
        int descriptor;
        IRunnable* runnable;
        int num_total_tasks;

        bool running = false;
        
};

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSleeping: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSleeping(int num_threads);
        ~TaskSystemParallelThreadPoolSleeping();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();

        struct thread* pool;
        
        std::mutex requests_mtx;
        std::queue<request> requests;

        std::condition_variable cv; // system thread 컨트롤변수

        std::atomic<bool> shutdown;

        int assignedTasks;
        std::atomic<int> chunkCount;

        std::atomic<int> computedTasks;
        std::mutex computedTasks_mtx;
        std::condition_variable computedTasks_cv;

        IRunnable* runnable;
        int num_total_tasks;

        void send(request req);
        void sleep(int thread_number);
};

struct workRange{
    int start;
    int count;
};

struct thread{
    std::thread worker;
    std::deque<workRange> workQueue;
    std::mutex mtx;             // workQueue의 mutex
    std::condition_variable cv; // for 스레드 sleep

    bool requestAck;
    int thread_number;

    void send(TaskSystemParallelThreadPoolSleeping& system, int req_num);
};

#endif
