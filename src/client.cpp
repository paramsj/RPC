#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "order_service/order.grpc.pb.h"
#include "order_service/order.pb.h"

class OrderClient {
public:
    OrderClient(std::string server_address) {
        this->channel_ = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
        this->stub_ = order_service::v1::OrderService::NewStub(this->channel_);
    }

    order_service::v1::ListResponse List(const order_service::v1::ListRequest &request) {
        grpc::ClientContext context;
        order_service::v1::ListResponse response;

        grpc::Status status = this->stub_->List(&context, request, &response);

        if (!status.ok()) {
            std::cout << "Error " << status.error_code() << ": " << status.error_message() << std::endl;
            throw std::runtime_error(status.error_message());
        }

        return response;
    }

private:
    std::unique_ptr<order_service::v1::OrderService::StubInterface> stub_;
    std::shared_ptr<grpc::ChannelInterface> channel_;
};

int main() {
    try {
        OrderClient client("0.0.0.0:50055");

        order_service::v1::ListRequest request;
        request.set_page(1);
        request.set_limit(10);
        request.set_user_id("1");

        std::cout << "Requesting order list..." << std::endl;

        auto response = client.List(request);

        std::cout << "List Response Received!" << std::endl;
        std::cout << "Total Orders: " << response.total() << std::endl;

        for (const auto& order : response.orders()) {
            std::cout << "---------------------------" << std::endl;
            std::cout << "Order ID:         " << order.id() << std::endl;
            std::cout << "Order Status:     " << order.status() << std::endl;
            std::cout << "Created At:       " << order.created_at() << std::endl;
            
            for (const auto& item : order.items()) {
                std::cout << "  - Item: " << item.name() 
                          << " (ID: " << item.id() 
                          << ", Qty: " << item.quantity() 
                          << ", Price: $" << item.price() << ")" << std::endl;
            }
        }
    }
    catch (const std::exception &e) {
        std::cerr << "Failed to get list of orders: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}