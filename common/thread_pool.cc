#include <iostream>
#include "thread_pool.h"

#ifdef _WIN32
#include "sched.h"
#endif

namespace dbase {
Thread_Pool::Thread_Pool(int thread_max):_thread_max(thread_max),
    _stop(false)
{
    LOGI("Thread_Pool thread_max= %d", _thread_max);
}

Thread_Pool::~Thread_Pool() {

}

void Thread_Pool::on_work(int index) {
    while(!_stop) {
        THREAD_FUNC thread_func_obj = nullptr;
        {
            std::unique_lock<std::mutex> lock(_queue_mutex_array[index]);
            _condition_array[index].wait(lock, [=](){
                return _stop || !_task_vec_array[index].empty();
            });
            if (_stop && _task_vec_array[index].empty()) {
                return;
            }
//            thread_func_obj = std::move(_task_vec_array[index].front());
//            _task_vec_array[index].pop();
            auto item = _task_vec_array[index].begin();
            thread_func_obj = item->second;
            _task_vec_array[index].erase(item);
        }
        if (thread_func_obj == nullptr) {
            continue;
        }
        try {
            thread_func_obj(nullptr);
        }
        catch (std::exception e) {
            LOGE("call function exception: %s", e.what());
        }
    }
    return;
}

int Thread_Pool::Start() {
    for (int index = 0; index < _thread_max; index++) {
        _thread_list.emplace_back(std::thread(std::bind(&Thread_Pool::on_work, this, index)));
    //     change the priority and calling policy of the getPCMLoop thread
        int max_priority = sched_get_priority_max(SCHED_FIFO);
//        int min_priority = sched_get_priority_min(SCHED_FIFO);
        sched_param sch;
        sch.sched_priority = max_priority;
    #ifndef _WIN32
        if (pthread_setschedparam(_thread_list[index].native_handle(), SCHED_FIFO, &sch)) {
            LOGE("Thread_Pool set thread SCHED_FIFO err!");
        }
//        CP_DBG_LOG("max_priority = %d, min_priority = %d",max_priority,min_priority);
        
    #endif
    }
    return 0;
}


void Thread_Pool::Stop() {
    if (_stop) {
        return;
    }
       
    _stop = true;

    int index = 0;
    for (std::thread &thread_item : _thread_list) {
        _condition_array[index++].notify_all();
        thread_item.join();
    }
    _thread_list.clear();
    return;
}

void Thread_Pool::addTask(uint32_t hash_key, THREAD_FUNC task_obj) {
    uint32_t index = hash_key%_thread_max;
//    CP_DBG_LOG("Thread_Pool:: addTask hash_key = %d, index = %d",hash_key,index);
    {
        std::unique_lock<std::mutex> lock(_queue_mutex_array[index]);
//        if (_stop) {
//            throw std::runtime_error("task on stopped ThreadPool");
//        }
        _task_vec_array[index].emplace_back(std::pair<uint32_t,THREAD_FUNC>(hash_key, task_obj));
    }
    _condition_array[index].notify_one();
    return;
}

int Thread_Pool::taskSize(uint32_t hash_key) {
    uint32_t index = hash_key % _thread_max;
    std::unique_lock<std::mutex> lock(_queue_mutex_array[index]);
    return _task_vec_array[index].size();
}
    
void Thread_Pool::stopTask(uint32_t hash_key) {
    uint32_t index = hash_key%_thread_max;
    {
        LOGI("Thread_Pool:: stopTask hash_key = %d",hash_key);
        std::unique_lock<std::mutex> lock(_queue_mutex_array[index]);
//        if (_stop) {
//            throw std::runtime_error("task on stopped ThreadPool");
//        }
        for (auto iter = _task_vec_array[index].begin(); iter != _task_vec_array[index].end();) {
            auto del_pair = *iter;
//            CP_INFO_LOG("Thread_Pool:: stopTask first = %d, hash_key = %d",del_pair.first,hash_key);
            if (del_pair.first == hash_key) {
                iter = _task_vec_array[index].erase(iter);
                LOGI("Thread_Pool:: stopTask find hash and erase %d",hash_key);
            } else {
                iter++;
            }
        }
    }
}

}
