#include "tasksys.h"


IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {this->num_threads = num_threads;}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemSerial::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //

    this->pool = new std::thread[num_threads];
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::runTasks(IRunnable* runnable, int num_total_tasks, int numThread) {
    for(int i = numThread; i<num_total_tasks; i += num_threads){
        runnable->runTask(i, num_total_tasks);
    }
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    //int GRANULARITY = 32*32;

    for(int i = 0; i<this->num_threads; i++){
        this->pool[i] = std::thread(&TaskSystemParallelSpawn::runTasks, this, runnable, num_total_tasks, i);
    }

    for(int i = 0; i<this->num_threads; i++){
        this->pool[i].join();
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    pool = new std::thread[num_threads];
    targs = new struct args[num_threads];
    mtx = new std::mutex[num_threads];


    running = true;

    // init threads
    for(int i = 0; i<num_threads; i++){
        targs[i].state = IDLE;
        targs[i].thread_number = i;
    }

    for(int i = 0; i<num_threads; i++){
        pool[i] = std::thread(&TaskSystemParallelThreadPoolSpinning::spinning, this, i);
    }
    
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    running = false;
    for(int i = 0; i<num_threads; i++){
        pool[i].join();
    }

    delete[] pool;
    delete[] targs;
    delete[] mtx;
}

void TaskSystemParallelThreadPoolSpinning::spinning(int thread_number){
    while(running){
        for(int i = 0; i<num_threads; i++){
            int index = (i + thread_number) % num_threads;
            int workIndex = -1;

            mtx[index].lock();
            if(!targs[index].workQueue.empty()){
                workIndex = targs[index].workQueue.back();
                targs[index].workQueue.pop_back();
            }
            mtx[index].unlock();
            if(workIndex == -1)
                continue;

            
            runnable->runTask(workIndex, num_total_tasks);
            
            descriptor_mtx.lock();
            descriptor++;
            descriptor_mtx.unlock();
        }
    }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    descriptor = 0;
    this->runnable = runnable;
    this->num_total_tasks = num_total_tasks;


    for(int i = 0; i<num_threads; i++){
        mtx[i].lock();

        for(int workIndex = i; workIndex < num_total_tasks; workIndex += num_threads){
            targs[i].workQueue.push_front(workIndex);
        }

        mtx[i].unlock();
    }

    while(1){
        descriptor_mtx.lock();
        if(descriptor == num_total_tasks)
        {
            descriptor_mtx.unlock();
            break;
        }
        descriptor_mtx.unlock();
    }
    printf("run finished.\n");
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

 void thread::send(TaskSystemParallelThreadPoolSleeping& system, int req_num) {
    system.send(request(this->thread_number, req_num));
}

void TaskSystemParallelThreadPoolSleeping::send(request req) {
    std::lock_guard<std::mutex> lock(requests_mtx);
    requests.push(req);
}

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads)
    : ITaskSystem(num_threads) {

    pool = new struct thread[num_threads];
    shutdown = false;
    computedTasks = 0;
    chunkCount = 1;

    for (int i = 0; i < num_threads; i++) {
        pool[i].thread_number = i;
        pool[i].worker = std::thread(&TaskSystemParallelThreadPoolSleeping::sleep, this, i);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    shutdown = true;

    for (int i = 0; i < num_threads; i++) {
        pool[i].cv.notify_one();
    }
    for (int i = 0; i < num_threads; i++) {
        pool[i].worker.join();
    }

    delete[] pool;
}

void TaskSystemParallelThreadPoolSleeping::sleep(int thread_number) {
    while (true) {
        bool empty;
        {
            std::lock_guard<std::mutex> lock(pool[thread_number].mtx);
            empty = pool[thread_number].workQueue.empty();
        }

        if (empty) {
            // 큐가 비어있을 때 딱 한 번만 request 전송
            pool[thread_number].send(*this, chunkCount);
            cv.notify_one();

            std::unique_lock<std::mutex> lock(pool[thread_number].mtx);
            pool[thread_number].cv.wait(lock, [this, thread_number] {
                return !pool[thread_number].workQueue.empty() || shutdown;
            });
        }

        if (shutdown)
            break;

        int start, end;
        {
            std::lock_guard<std::mutex> lock(pool[thread_number].mtx);
            start = pool[thread_number].workQueue.front().start;
            end = start + pool[thread_number].workQueue.front().count;
            pool[thread_number].workQueue.pop_front();
        }

        int workIndex = start;
        int clampedEnd = std::min(end, num_total_tasks);
        for (; workIndex < clampedEnd; workIndex++) {
            runnable->runTask(workIndex, num_total_tasks);
        }

        computedTasks += workIndex - start;
        computedTasks_cv.notify_one();
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {
    this->runnable = runnable;
    this->num_total_tasks = num_total_tasks;

    computedTasks = 0;
    assignedTasks = 0;
    // 워커당 대략 4번 나눠 받도록 청크 크기를 동적으로 계산
    chunkCount = std::max(1, num_total_tasks / (num_threads * 4));

    while (assignedTasks < num_total_tasks) {
        request req(-1, -1);
        {
            std::unique_lock<std::mutex> lock(requests_mtx);
            cv.wait(lock, [this, &req] {
                if (!requests.empty()) {
                    req = requests.front();
                    requests.pop();
                    return true;
                }
                return false;
            });
        }

        int thread_number = req.thread_number;
        int assigned_count = std::min(req.work_number, num_total_tasks - assignedTasks);

        {
            std::lock_guard<std::mutex> lock(pool[thread_number].mtx);
            pool[thread_number].workQueue.push_back({assignedTasks, assigned_count});
        }
        assignedTasks += assigned_count;
        pool[thread_number].cv.notify_one();
    }

    {
        std::unique_lock<std::mutex> lock(computedTasks_mtx);
        computedTasks_cv.wait(lock, [this, num_total_tasks] {
            return computedTasks == num_total_tasks;
        });
    }
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
