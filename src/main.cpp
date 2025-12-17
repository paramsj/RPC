#include<bits/stdc++.h>
#include "order_service/order.pb.h"
#include "config/config.hpp"

using namespace std;

int main() {
    try {
        Config config = Config::New();
    } catch (exception &e) {
        cerr<<e.what()<<endl;
        return 1;
    } 
    return 0;
}