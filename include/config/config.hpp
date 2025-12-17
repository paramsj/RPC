#pragma once
#include <string>
using namespace std;

inline string get_env(const string &key, const string &default_value) {
    string value = getenv(key.c_str());
    return !value.empty() ? value : default_value;
}
struct Config {
    string host;
    string port;

    static Config New() {
        Config config;
        config.host = get_env("GRPC_HOST","0.0.0.0");
        config.port = get_env("GRPC_PORT","50055");

        return config;
    }
};