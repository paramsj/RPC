#pragma once

#include<grpcpp/grpcpp.h>
#include<grpcpp/support/interceptor.h>
#include<grpcpp/support/server_interceptor.h>
#include<grpcpp/security/server_credentials.h>
#include <memory>
#include<string>
#include<vector>
#include<grpcpp/impl/service_type.h>

#include "interceptors/logger.hpp"

class Server
{
public:
    explicit Server(std::string server_address, std::shared_ptr<grpc::Service> service = nullptr, std::string service_name = "Server");
        void Start();
        void Stop();
        void Wait();

private:
    std::unique_ptr<grpc::Server> server_;
    std::shared_ptr<grpc::Service> service_;
    std::string server_address_;
    std::string service_name_;

    std::vector<std::unique_ptr<grpc::experimental::ServerInterceptorFactoryInterface>> intercepttor_creators_;

};