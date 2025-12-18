#include "server/server.hpp"
#include <iostream>

Server::Server(std::string server_address, std::shared_ptr<grpc::Service> service, std::string service_name)
    : server_address_(server_address), service_(service), service_name_(service_name)
{
    // FIX: You must provide the Factory, not the Interceptor itself.
    // The Factory handles the creation of Interceptors for each RPC.
    this->intercepttor_creators_.emplace_back(std::make_unique<LoggerInterceptorFactory>());
}

void Server::Start() {
    grpc::ServerBuilder builder;
    
    // Use InsecureServerCredentials for the server-side listener
    builder.AddListeningPort(this->server_address_, grpc::InsecureServerCredentials());

    // Move ownership of the factory creators to the gRPC builder
    if (!this->intercepttor_creators_.empty()) {
        builder.experimental().SetInterceptorCreators(std::move(this->intercepttor_creators_));
    }

    builder.RegisterService(this->service_.get());

    this->server_ = builder.BuildAndStart();
    
    if (this->server_) {
        std::cout << this->service_name_ << " started and listening on " << this->server_address_ << std::endl;
    } else {
        std::cerr << "Failed to start " << this->service_name_ << "!" << std::endl;
    }
}

// Added this to allow main.cpp to block while keeping Start() non-blocking
void Server::Wait() {
    if (this->server_) {
        this->server_->Wait();
    }
}

void Server::Stop() {
    if (this->server_) {
        std::cout << this->service_name_ << " shutting down..." << std::endl;
        this->server_->Shutdown();
        std::cout << this->service_name_ << " shutdown complete!" << std::endl;
    }
}