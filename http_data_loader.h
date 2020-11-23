#define CURL_STATICLIB

#pragma once
#include <iostream>
#include <thread>
#include <functional>

#include "curl.h"

class HttpDataLoader
{
private:
    typedef std::unique_ptr<CURL, std::function<void(CURL*)>> CURL_ptr;

    CURL_ptr m_curl_ptr;
    std::string m_data;
    constexpr static auto deleter = [](CURL* c)
    {
        curl_easy_cleanup(c);
        curl_global_cleanup();
    };
    std::string m_url;

public:
    HttpDataLoader(const std::string& url);
    CURLcode fetch();
    std::string get_fetched_data();
    std::string get_url() const;
};

