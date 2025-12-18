#include<bits/stdc++.h>
#include "order_service/order.pb.h"
#include "service/order_service.hpp"
#include "config/config.hpp"
#include "server/server.hpp"
#include <csignal>
#include <condition_variable>
#include<iostream>


using namespace std;


std::unique_ptr<Server> g_server = nullptr;
bool shutdown_requested = false;
std::mutex shutdown_mutex;
std::condition_variable shutdown_cv;

void handle_signal(int signal) {
    cout<<"signal recv"<< strsignal(signal) << endl;
    shutdown_requested = true;
    shutdown_cv.notify_one();   
}


int main() {
    try {
        std::signal(SIGINT, handle_signal);
        std::signal(SIGTERM, handle_signal);
        Config config = Config::New();
        std::shared_ptr<OrderService> oService = std::make_shared<OrderService>();
        g_server = std::make_unique<Server>(config.host+":"+config.port,std::static_pointer_cast<grpc::Service>(oService),os::OrderService::service_full_name());

        std::thread shutdown_thread(
            [&]()
            {
                std::unique_lock<std::mutex> lock(shutdown_mutex);

                shutdown_cv.wait(lock,
                                 [&]()
                                 {
                                     return shutdown_requested;
                                 });

                g_server->Stop();
            });
        
        g_server->Start();
        g_server->Wait();

        shutdown_thread.join();
    } catch (exception &e) {
        cerr<<e.what()<<endl;
        return 1;
    } 
    return 0;
}