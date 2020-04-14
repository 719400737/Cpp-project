#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<list>
#include<cstdio>
#include<exception>
#include<pthread.h>
#include"../lock/locker.h"
template<typename T>
class threadpool{
    public:
        /*thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理的请求的数量*/
        threadpool(int thread_number=8,int max_request=10000);
        ~threadpool();
        bool append(T* request);//往请求队列中添加请求
    private:
        /*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
        static void *worker(void *arg);
        void run();
    private:
        int m_thread_number;//线程池中的线程数
        int m_max_requests;//请求队列中允许的最大请求数
        pthread_t *m_threads;//描述线程池的数组，其大小为m_thread_number
        std::list<T *>m_workqueue;//请求队列
        locker m_queuelocker;//保护请求队列的互斥锁
        sem m_queuestat;//是否有任务需要处理
        bool m_stop;//是否结束线程
};
template<typename T>
threadpool<T>::threadpool(int thread_number,int max_requests):m_thread_number(thread_number),m_max_requests(max_requests),m_stop(false),m_threads(NULL){
    if(thread_number<=0||max_requests<=0)
        throw std::exception();
    m_threads=new pthread_t[m_thread_number];
    if(!m_threads)
        throw std::exception();
    for(int i=0;i<thread_number;++i)
    {
        //printf("create the %dth thread\n",i);
        if(pthread_create(m_threads+i,NULL,worker,this)!=0){
            delete [] m_threads;
            throw std::exception();
        }
        if(pthread_detach(m_threads[i])){//子线程与主线程分离，子线程结束后系统自动
            delete[] m_threads;
            throw std::exception();
        }
    }
}
template<typename T>
threadpool<T>::~threadpool(){
    delete [] m_threads;
    m_stop=true;
}
template<typename T>
bool threadpool<T>::append(T* request)
{
    m_queuelocker.lock();//上锁，因为他被所有线程共享
    if(m_workqueue.size()>m_max_requests)//请求数大于最大限数
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);//请求加入队列
    m_queuelocker.unlock();
    m_queuestat.post();//信号的pv操作，post是释放+1，表示有任务需要处理
    return true;
}
template<typename T>
void* threadpool<T>::worker(void* arg){
    threadpool* pool=(threadpool*)arg;
    pool->run();
    return pool;
}
template<typename T>
void threadpool<T>::run()
{
    while(!m_stop)
    {
        m_queuestat.wait();//任务处理-1
        m_queuelocker.lock();//加锁
        if(m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }
        T* request=m_workqueue.front();//请求队列第一个出来
        m_workqueue.pop_front();
        m_queuelocker.unlock();//解锁
        if(!request)
            continue;
        request->process();//对请求进行处理
    }
}
#endif

