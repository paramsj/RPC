#include "service/order_service.hpp"
#include <random>

OrderService::OrderService()
{
    os::Order order;
    order.set_id(this->generate_id());
    order.set_status(os::OrderStatus::PENDING);
    order.set_address("123 Main St");
    order.set_total_price(100.0);
    order.set_created_at(this->get_current_timestamp());
    order.set_updated_at(this->get_current_timestamp());

    os::Item item;
    item.set_id(this->generate_id());
    item.set_name("Product 1");
    item.set_quantity(2);
    item.set_price(50.0);
    order.add_items()->CopyFrom(item);

    os::Order order2;
    order2.set_id(this->generate_id());
    order2.set_status(os::OrderStatus::DELIVERED);
    order2.set_address("123 Main St");
    order2.set_total_price(50.0);
    order2.set_created_at(this->get_current_timestamp());
    order2.set_updated_at(this->get_current_timestamp());

    os::Item item2;
    item2.set_id(this->generate_id());
    item2.set_name("Product 2");
    item2.set_quantity(1);
    item2.set_price(50.0);
    order2.add_items()->CopyFrom(item2);

    os::Order order3;
    order3.set_id(this->generate_id());
    order3.set_status(os::OrderStatus::PROCESSING);
    order3.set_address("123 Main St");
    order3.set_total_price(200.0);
    order3.set_created_at(this->get_current_timestamp());
    order3.set_updated_at(this->get_current_timestamp());

    os::Item item3;
    item3.set_id(this->generate_id());
    item3.set_name("Product 3");
    item3.set_quantity(4);
    item3.set_price(50.0);
    order3.add_items()->CopyFrom(item3);

    this->orders_[order.id()] = order;
    this->orders_[order2.id()] = order2;
    this->orders_[order3.id()] = order3;

    this->user_orders_["1"].push_back(order.id());
    this->user_orders_["1"].push_back(order2.id());
    this->user_orders_["1"].push_back(order3.id());
}

Status OrderService::Get(ServerContext *ctx, const os::GetRequest *request, os::GetResponse *response) {

    std::lock_guard<std::mutex> lock(this->mutex_);
    std::string order_id = request->order_id();
    if (this->orders_.find(order_id)==this->orders_.end()) {
        return Status(grpc::StatusCode::NOT_FOUND, "Order is not Found!");
    }
    os::Order order = this->orders_[order_id];
    response->mutable_order()->CopyFrom(order);

    return Status::OK;
}

Status OrderService::List(ServerContext *ctx, const os::ListRequest *request, os::ListResponse *response) {

    std::lock_guard<std::mutex> lock(this->mutex_);
    std::string user_id = request->user_id();
    int limit = request->limit() > 0 ? request->limit() : 10;
    int page = request->page() > 0 ? request->page() : 1;
    auto filters = request->filter();
    auto user_order_it = this->user_orders_.find(user_id);
    if(user_order_it==this->user_orders_.end()) {
        return Status::OK;
    }
    for(auto order_id : user_order_it->second) {
        os::Order order = this->orders_[order_id];
        response->add_orders()->CopyFrom(order);
    }
    return Status::OK;
}

Status OrderService::Create(ServerContext *ctx, const os::CreateRequest *request, os::CreateResponse *response) {

    std::lock_guard<std::mutex> lock(this->mutex_);
    std::string user_id = request->user_id();
    os::Order order = request->order();
    
    order.set_id(this->generate_id());
    order.set_status(os::OrderStatus::PENDING);
    order.set_created_at(this->get_current_timestamp());
    order.set_updated_at(this->get_current_timestamp());
    double total_price = 0;

    for(auto item : order.items()) {
        total_price = item.price() * item.quantity();
    }

    order.set_total_price(total_price);
    this->orders_[order.id()] = order;
    this->user_orders_[user_id].emplace_back(order.id());
    response->mutable_order()->CopyFrom(order);

    return Status::OK;
}


Status OrderService::Update(ServerContext *ctx, const os::UpdateRequest *request, os::UpdateResponse *response) {

    std::lock_guard<std::mutex> lock(this->mutex_);
    os::Order order = request->order();

    if(this->orders_.find(order.id())==this->orders_.end()) {
        return Status(grpc::StatusCode::NOT_FOUND,"NOt found!");
    }

    this->orders_[order.id()] = order;
    response->mutable_order()->CopyFrom(order);
    return Status::OK;
}

Status OrderService::StreamOrderUpdates(ServerContext *ctx, const os::StreamOrderUpdateRequest *request, ServerWriter<os::StreamOrderUpdateResponse> *writer)
{
    const std::string &order_id = request->order_id();

    {
        std::lock_guard<std::mutex> lock(this->mutex_);

        if (!order_id.empty())
        {
            // Check specific order
            auto order_it = this->orders_.find(order_id);
            if (order_it == this->orders_.end())
            {
                return grpc::Status(::grpc::StatusCode::NOT_FOUND, "Order not found");
            }

            // Send initial update
            os::StreamOrderUpdateResponse response;
            *(response.mutable_order()) = order_it->second;
            response.set_type(os::UpdateType::CREATED);
            response.set_updated_at(this->get_current_timestamp());

            if (!writer->Write(response))
            {
                return grpc::Status::CANCELLED;
            }
        }
        else
        {
            return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                                  "order_id must be specified");
        }
    }

    int update_count = 0;
    while (!ctx->IsCancelled() && update_count < 5)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        std::lock_guard<std::mutex> lock(mutex_);

        if (!order_id.empty())
        {
            auto order_it = orders_.find(order_id);
            if (order_it != orders_.end())
            {
                os::Order &order = order_it->second;
                os::OrderStatus current_status = order.status();

                // Progress the status if possible
                if (current_status == os::OrderStatus::PENDING)
                {
                    order.set_status(os::OrderStatus::PROCESSING);
                }
                else if (current_status == os::OrderStatus::PROCESSING)
                {
                    order.set_status(os::OrderStatus::SHIPPED);
                }
                else if (current_status == os::OrderStatus::SHIPPED)
                {
                    order.set_status(os::OrderStatus::DELIVERED);
                }
                // Only send update if status changed
                if (current_status != order.status())
                {
                    order.set_updated_at(this->get_current_timestamp());
                    os::StreamOrderUpdateResponse response;
                    *(response.mutable_order()) = order;
                    response.set_type(os::UpdateType::STATUS_CHANGE);
                    response.set_updated_at(this->get_current_timestamp());

                    if (!writer->Write(response))
                    {
                        break;
                    }

                    update_count++;
                }
            }
        }
    }
    return ::grpc::Status::OK;
}

std::string OrderService::generate_id()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char *hex = "0123456789abcdef";

    std::string uuid;
    for (int i = 0; i < 32; ++i)
    {
        uuid += hex[dis(gen)];
        if (i == 7 || i == 11 || i == 15 || i == 19)
        {
            uuid += '-';
        }
    }

    return uuid;
}

int64_t OrderService::get_current_timestamp()
{
    return std::time(nullptr);
}