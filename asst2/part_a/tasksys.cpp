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



const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads)
    : ITaskSystem(num_threads) {

    pool = new struct thread[num_threads];
    shutdown = false;

    for(int i = 0; i<num_threads; i++){
        pool[i].thread_number = i;
        pool[i].worker = std::thread(&TaskSystemParallelThreadPoolSleeping::sleep, this, i);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    {
        std::lock_guard<std::mutex> lock(idleMtx);
        shutdown = true;
    }
    idleCv.notify_all();

    for(int i = 0; i<num_threads; i++){
        pool[i].worker.join();
    }

    delete[] pool;
}

void TaskSystemParallelThreadPoolSleeping::sleep(int thread_number) {

    workRange work;
    bool hasWork = false;

    while (!shutdown) {
        if (!hasWork) {
            // work를 채우려고 시도
            for (int i = 0; i < num_threads; i++) {
                int searchThreadIndex = (thread_number + i) % num_threads;

                // 자기 queue면 pop
                if (i == 0) {
                    hasWork = pool[thread_number].pop(work);
                }
                // 다른 queue면 steal
                else {
                    hasWork = pool[thread_number].steal(pool[searchThreadIndex], work);
                }

                if (hasWork) {
                    std::lock_guard<std::mutex> lock(idleMtx);
                    availableWork--;
                    break;
                }
            }

            if (!hasWork) {
                std::unique_lock<std::mutex> lock(idleMtx);
                idleCv.wait(lock, [this] {
                    return shutdown || availableWork > 0;
                });
            }
        }
        else {
            // work를 시행
            int start = work.start;
            int count = work.count;

            if (count > GRANULARITY) {

                int leftcount = count / 2;
                int rightcount = count - leftcount;

                work = workRange{start, leftcount};

                // 절반을 push하고 아무나 wakeup
                {
                    std::lock_guard<std::mutex> lock(idleMtx);
                    pool[thread_number].push(workRange{start + leftcount, rightcount});
                    availableWork++;
                }       
                idleCv.notify_one();
            }
            else {
                for (int i = 0; i < count; i++) {
                    runnable->runTask(start + i, num_total_tasks);
                }

                {
                    std::lock_guard<std::mutex> lock(finishTasks);
                    computedTasks += count;
                }
                finishTasks_cv.notify_one();
                // local work 비워줌
                hasWork = false;
            }
        }
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {
    this->runnable = runnable;
    this->num_total_tasks = num_total_tasks;
    GRANULARITY = std::max(1, num_total_tasks / (num_threads*4));
    computedTasks = 0;

    {
        std::lock_guard<std::mutex> lock(idleMtx);
        pool[0].push(workRange{0, num_total_tasks});
        availableWork++;
    }
    idleCv.notify_one();

    {
        std::unique_lock<std::mutex> lock(finishTasks);
        finishTasks_cv.wait(lock, [this, num_total_tasks] {return computedTasks == num_total_tasks;});
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
