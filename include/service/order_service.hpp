#pragma once
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/status.h>
#include <grpcpp/server_context.h>

#include "order_service/order.grpc.pb.h"
#include "order_service/order.pb.h"
#include "google/protobuf/map.h"
#include<unordered_map>
#include<vector>
#include<string>
#include<mutex>

namespace os = order_service::v1;

using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

class OrderService final : public os::OrderService::Service 
{
public :
    OrderService();

    grpc::Status Get(grpc::ServerContext* context, const os::GetRequest* request, os::GetResponse* response) override;
    grpc::Status List(grpc::ServerContext* context, const os::ListRequest* request, os::ListResponse* response) override;
    grpc::Status Create(grpc::ServerContext* context, const os::CreateRequest* request, os::CreateResponse* response) override;
    grpc::Status Update(grpc::ServerContext* context, const os::UpdateRequest* request, os::UpdateResponse* response) override;
    grpc::Status StreamOrderUpdates(grpc::ServerContext* context, const os::StreamOrderUpdateRequest* request, os::StreamOrderUpdateResponse* response) override;

private :

    std::unordered_map<std::string,os::Order> orders_;
    std::unordered_map<std::string,std::vector<std::string>> user_orders_;
    std::mutex mutex_;

    static std::string generate_id();
    static int64_t get_current_timestamp();
};