#define CURL_STATICLIB

#pragma once
#include <iostream>
#include <thread>
#include <functional>

#include "curl.h"

class HttpDataLoader
{
public:
    HttpDataLoader(const std::string& url);
    CURLcode fetch();
    std::string getData() const {
        return m_data;
    }

private:
    constexpr static auto deleter = [](CURL* c) {
        curl_easy_cleanup(c);
        curl_global_cleanup();
    };

    typedef std::unique_ptr<CURL, std::function<void(CURL*)>> CURL_ptr;
    CURL_ptr m_curlPtr;
    std::string m_data;
    std::string m_url;
};

