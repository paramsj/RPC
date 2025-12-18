#pragma once

#include <grpcpp/grpcpp.h>
#include <grpcpp/support/interceptor.h>
#include <grpcpp/support/server_interceptor.h>
#include <grpcpp/security/server_credentials.h>
#include <iostream>
#include <string>

class LoggerInterceptor final : public grpc::experimental::Interceptor
{
private:
    std::string method_name_;

public:
    explicit LoggerInterceptor(grpc::experimental::ServerRpcInfo *info)
    {
        method_name_ = info->method();
        // Log immediately on creation for simplicity, as per your original style
        if (method_name_ == "unknown") {
            std::cout << "Unimplemented RPC called" << std::endl;
        } else {
            std::cout << "RPC called: " << method_name_ << std::endl;
        }
    }

    // Added 'override' and fixed the logic to ensure the RPC continues
    void Intercept(grpc::experimental::InterceptorBatchMethods *methods) override
    {
        // In gRPC, Proceed() must be called to allow the RPC to continue 
        // through its lifecycle (metadata, messages, status).
        methods->Proceed();
    }
};

class LoggerInterceptorFactory : public grpc::experimental::ServerInterceptorFactoryInterface
{
public:
    // Added 'override' to match the base class signature exactly
    grpc::experimental::Interceptor* CreateServerInterceptor(grpc::experimental::ServerRpcInfo *info) override
    {
        return new LoggerInterceptor(info);
    }
};