#include<bits/stdc++.h>
#include "order_service/order.pb.h"

using namespace std;

int main() {
    order_service::v1::Order order;
    order.set_id("1");

    cout<<order.id()<<endl;
    return 0;
}