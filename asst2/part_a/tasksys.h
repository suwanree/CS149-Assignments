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


        // idle thread가 잡는 뮤텍스/cv
        std::mutex idleMtx;
        std::condition_variable idleCv;

        // thread가 작업하는 최대 단위
        int GRANULARITY;

        // 매 run마다 초기화
        IRunnable* runnable;
        int num_total_tasks;
        
        // 처리된 task 추적
        std::atomic<int> computedTasks;
        std::mutex finishTasks;
        std::condition_variable finishTasks_cv;

        // 현재 모든 thread 통틀어 큐에 작업이 있는지 확인
        std::atomic<int> availableWork{0};


        // 프로그램 종료 추적
        std::atomic<bool> shutdown;

        void sleep(int thread_number);

};

typedef struct workRange{
    int start;
    int count;
} workRange;

struct thread{
    std::thread worker;
    std::deque<workRange> workQueue;
    std::mutex mtx;             // workQueue의 mutex

    int thread_number;
    
    // 자신의 local로 steal하는 함수
    // queue에 넣지 않음.
    bool steal(struct thread& thrd, workRange& out){
        std::unique_lock<std::mutex> lock(thrd.mtx);

        if (thrd.workQueue.empty())
            return false;

        out = thrd.workQueue.back();
        thrd.workQueue.pop_back();


        return true;
    };

    void push(workRange work) {
        mtx.lock();
        workQueue.push_back(work);
        mtx.unlock();
    };

    bool pop(workRange& out){
        std::unique_lock<std::mutex> lock(mtx);

        if (workQueue.empty())
            return false;

        out = workQueue.front();
        workQueue.pop_front();


        return true;
    };
};

#endif
