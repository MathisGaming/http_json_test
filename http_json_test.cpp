#define CURL_STATICLIB
#include <iostream>
#include <thread>

#include "include/json/json.hpp"
#include "curl.h"

using json = nlohmann::json;

typedef std::unique_ptr<CURL, std::function<void(CURL*)>> CURL_ptr;

extern "C" std::size_t dataHandler(const char* buffer, std::size_t size, std::size_t nmeb, std::string * userData)
{
    if (userData == nullptr) return 0;

    userData->append(buffer, (size * nmeb));
    return size * nmeb;
};

class CurlHandle 
{
    private :
        CURL_ptr m_curl_ptr;
        std::string m_data;
        constexpr static auto deleter = [](CURL* c) 
        {
            curl_easy_cleanup(c);
            curl_global_cleanup();
        };
    public:
        CurlHandle() : m_curl_ptr(curl_easy_init(), deleter)
        {
            curl_global_init(CURL_GLOBAL_ALL);
            curl_easy_setopt(m_curl_ptr.get(), CURLOPT_WRITEFUNCTION, dataHandler);
            curl_easy_setopt(m_curl_ptr.get(), CURLOPT_WRITEDATA, &m_data);
        }

        void set_url(const std::string& url)
        {
            curl_easy_setopt(m_curl_ptr.get(), CURLOPT_URL, url.c_str());
        }

        CURLcode fetch() 
        {
            return curl_easy_perform(m_curl_ptr.get());
        }

        std::string get_fetched_data() 
        {
            return m_data;
        }
};

class Bitcoin
{
    private:
        CurlHandle m_curl_handle;
        static constexpr const char* API_URL = "https://blockchain.info/ticker";
    public:
        Bitcoin() : m_curl_handle({})
        {
            m_curl_handle.set_url(API_URL);
        }

        json fetch_bitcoin_data() 
        {
            m_curl_handle.fetch();
            return json::parse(m_curl_handle.get_fetched_data());
        }
};

int main()
{
    using namespace std;
    uint64_t max_refresh = 60;
    uint64_t count = 0;
    const std::chrono::seconds refresh_rate_s = 1s;
    while (count < max_refresh)
    {
        ++count;
        try 
        {
            Bitcoin bitcoin;
            json bitcoin_json = bitcoin.fetch_bitcoin_data();

            const auto& bitcoin_to_usd_value_it = bitcoin_json.find("USD");

            if (bitcoin_to_usd_value_it != bitcoin_json.end())
            {
                cout << "1 BTC = ";
                printf("\t(%3s)%10d %s\n", bitcoin_to_usd_value_it.key().c_str(),
                    bitcoin_to_usd_value_it.value()["last"].get<int>(),
                    bitcoin_to_usd_value_it.value()["symbol"].get<string>().c_str());
            }
        }
        catch (...) 
        {
            cerr << "Failed to fetch bitcoin exchange rates \n";
        }
        std::this_thread::sleep_for(refresh_rate_s);
    }

    system("pause");
    return 0;
}