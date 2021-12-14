#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <vector>
#include <queue>
#include <utility>
#include <thread>
#include <map>
#include <mutex>
#include <future>
#include <functional> //without .h
#include <memory>

#define THREAD_MAX_NUM 4

#ifdef _WIN32
#ifdef DBASE_EXPORTS
#define DBASEAPI __declspec(dllexport)
#else
#define DBASEAPI __declspec(dllimport)
#endif
#else
#define DBASEAPI
#endif

#define LOGI(fmt,...) printf("I %s", fmt, ##__VA_ARGS__);
#define LOGE(fmt,...) printf("E %s", fmt, ##__VA_ARGS__);

namespace dbase {
using THREAD_FUNC = std::function<void(void*)>;
//using TASK_QUEUE  = std::queue<THREAD_FUNC>;
using TASK_VEC = std::vector<std::pair<uint32_t/*assrc*/,THREAD_FUNC>>;

class DBASEAPI Thread_Pool {
public:
    Thread_Pool(int thread_max);
    ~Thread_Pool();

    int Start();
    void Stop();

    void addTask(uint32_t hash_key, THREAD_FUNC task_obj);
    
    void stopTask(uint32_t hash_key);
    int taskSize(uint32_t hash_key);
private:
    Thread_Pool();
    void on_work(int index);

private:
    int _thread_max = THREAD_MAX_NUM;
//    TASK_QUEUE _task_queue_array[256];
    TASK_VEC _task_vec_array[256];
    std::vector< std::thread > _thread_list;
    
    std::mutex _queue_mutex_array[256];
    std::condition_variable _condition_array[256];
    bool _stop;
};

}
#endif//THREAD_POOL_H
