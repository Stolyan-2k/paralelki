#include <iostream>
#include <fstream>
#include <queue>
#include <future>
#include <thread>
#include <chrono>
#include <cmath>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <optional>
#include <experimental/random>

#define myType double

std::mutex cout_mutex; 

template<typename T>
T fun_sin(T arg) {
    return std::sin(arg);
}

template<typename T>
T fun_sqrt(T arg) {
    return std::sqrt(arg);
}

template<typename T>
T fun_pow(T x, T y) {
    return std::pow(x, y);
}


template <typename T>
class Server {
public:
    Server() {}

    ~Server() {
        stop();
        {
            std::lock_guard<std::mutex> lock(result_mutex);
            results.clear();
        }
    }

    void start() {
        server_thread = std::jthread(&Server::server_loop, this, stop_src.get_token());
    }

    void stop() {
        stop_src.request_stop();
        cond_var.notify_one();
        if (server_thread.joinable()) {
            server_thread.join();
        }
    }

    size_t add_task(std::function<double()> task) {
        size_t task_id = task_counter++;
        std::packaged_task<T()> packaged_task(std::move(task));
        std::future<T> result = packaged_task.get_future();
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            tasks.emplace(task_id, std::move(packaged_task));
        }
        {
            std::lock_guard<std::mutex> lock(result_mutex);
            results[task_id] = std::move(result);
        }
        cond_var.notify_one();
        return task_id;
    }


    double request_result(size_t task_id) {
        std::unique_lock<std::mutex> lock(result_mutex);
        auto it = results.find(task_id);

        if (it == results.end()) {
            throw std::runtime_error("Task ID not found!");
        }
        it->second.wait();
        return it->second.get();
    }

private:
    std::jthread server_thread;
    std::queue<std::pair<size_t, std::packaged_task<T()>>> tasks;
    std::unordered_map<size_t, std::future<T>> results;

    std::mutex queue_mutex, result_mutex;
    std::stop_source stop_src;
    std::condition_variable cond_var;
    std::atomic<size_t> task_counter{0};

    void server_loop(std::stop_token stoken) {
        while (!stoken.stop_requested()) {
            std::pair<size_t, std::packaged_task<T()>> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                cond_var.wait(lock, [this, &stoken] { return !tasks.empty() ||  stoken.stop_requested(); });

                if (stoken.stop_requested()) break;

                if (!tasks.empty()) {
                    task = std::move(tasks.front());
                    tasks.pop();
                }
            }
            try {
                task.second();
            } catch (const std::exception& e) {
                std::cerr << "Task execution failed: " << e.what() << '\n';
                continue;
            }


        }
        std::cout << "Server stopped.\n";
    }
};


void add_task1_thread(Server<myType>& server) {
    std::ofstream out;
    out.open("Task1.txt");
    for(int i = 0; i < 10000; ++i)
    {    
        double arg = std::experimental::randint(0, 100);
        size_t task_id = server.add_task(std::bind(fun_sin<myType>, arg));

        out << "sin " << arg << " = " << server.request_result(task_id) << '\n';
    }
    out.close();
}

void add_task2_thread(Server<myType>& server) {
    std::ofstream out;
    out.open("Task2.txt");
    for(int i = 0; i < 10000; ++i)
    {    
        double arg = std::experimental::randint(0, 100);
        size_t task_id = server.add_task(std::bind(fun_sqrt<myType>, arg));

        out << "sqrt " << arg << " = " << server.request_result(task_id) << '\n';
    }
    out.close();
}

void add_task3_thread(Server<myType>& server) {
    std::ofstream out;
    out.open("Task3.txt");
    for(int i = 0; i < 10000; ++i)
    {    
        double arg1 = std::experimental::randint(0, 100);
        double arg2 = std::experimental::randint(0, 20);
        size_t task_id = server.add_task(std::bind(fun_pow<myType>, arg1, arg2));

        out << "pow " << arg1 << " " <<  arg2 << " = " << server.request_result(task_id) << '\n';
    }
    out.close();
}

int main() {
    std::cout << "Start\n";

    Server<myType> server;
    server.start();

    std::thread add_task1(add_task1_thread, std::ref(server));
    std::thread add_task2(add_task2_thread, std::ref(server));
    std::thread add_task3(add_task3_thread, std::ref(server));

    add_task1.join();
    add_task2.join();
    add_task3.join();
    server.stop();

    std::cout << "End\n";
}