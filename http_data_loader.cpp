#define CURL_STATICLIB

#include "http_data_loader.h"

extern "C" std::size_t data_handler(const char* buffer, std::size_t size, std::size_t nmeb, std::string * user_data) {
    if (user_data == nullptr) return 0;

    user_data->append(buffer, (size * nmeb));
    return size * nmeb;
};

HttpDataLoader::HttpDataLoader(const std::string& url)
    : m_curl_ptr(curl_easy_init(), deleter)
    , m_url(url) {
    curl_global_init(CURL_GLOBAL_ALL);
    curl_easy_setopt(m_curl_ptr.get(), CURLOPT_WRITEFUNCTION, data_handler);
    curl_easy_setopt(m_curl_ptr.get(), CURLOPT_WRITEDATA, &m_data);
    curl_easy_setopt(m_curl_ptr.get(), CURLOPT_URL, url.c_str());
}

CURLcode HttpDataLoader::fetch() {
    return curl_easy_perform(m_curl_ptr.get());
}