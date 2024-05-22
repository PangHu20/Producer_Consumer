//  g++ producer_consumer.cpp -o producer_consumer -l pthread
#include <iostream>
#include <mutex>               // 锁
#include <queue>                // 任务队列
#include <thread>              // 创建线程
#include <functional>          // bind
#include <condition_variable>  // 条件变量


uint64_t num = 0;
class producer_consumer{
public:
    producer_consumer(){
        producer = new std::thread(std::bind(&producer_consumer::produce, this));
        consumer = new std::thread(std::bind(&producer_consumer::consume, this));
    }
    ~producer_consumer(){
        if(producer -> joinable()){
            producer -> join();
        }
        if(consumer -> joinable()){
            consumer -> join();
        }
        produce_is_not_finished_flag = false;            
        std::cout << "producer_consumer end" << std::endl;
    }

    void produce(){
        int produced_production_count = 0;
        while(produce_is_not_finished_flag){
            std::unique_lock<std::mutex> lockGuard(mutexLock);
            while(buffer_q.size() >= max_buffer_q_size){
                condition.wait(lockGuard);      
            }
            uint64_t data = num++;
            buffer_q.push(data);
            std::cout << "task data = " << data << " produced" << std::endl;
            produced_production_count++;
            if(produced_production_count > total_production_count){
                produce_is_not_finished_flag = false;     
            }
            lockGuard.unlock();                   
            condition.notify_all();               
        }   
    }
    void consume(){
        while(consume_is_not_finished_flag){
            std::unique_lock<std::mutex> lockGuard(mutexLock);
            while(buffer_q.empty()){
                condition.wait(lockGuard);        
            }
            uint64_t data = buffer_q.front();
            buffer_q.pop();
            std::cout << "task data = " << data << " has been consumed" << std::endl;
            if(!produce_is_not_finished_flag && buffer_q.empty()){
                consume_is_not_finished_flag = false;
            }
            lockGuard.unlock();                   
            condition.notify_all();               
        }   
    }

private:
    // uint64_t num = 0;
    std::queue<uint64_t> buffer_q{};
    std::mutex mutexLock{};             
    std::condition_variable condition{};
    std::thread * producer;
    std::thread * consumer;
    bool produce_is_not_finished_flag = true;
    bool consume_is_not_finished_flag = true;
    uint64_t total_production_count = 100;
    uint64_t max_buffer_q_size = 10;
};

static producer_consumer * global_PC;

int main() {
    global_PC = new producer_consumer();
    delete global_PC;
    return 0;
}
