#define CURL_STATICLIB
#include <iostream>
#include <thread>
#include <future>
#include <queue>
#include <vector>
#include <algorithm>
#include <shared_mutex>

#include "include/json/json.hpp"
#include "http_data_loader.h"
#include "threadsafe_queue.h"
#include "threadpool.h"
#include "curl.h"

using namespace std;

// Class to store the data
class BitcoinData
{
private:
    HttpDataLoader m_curl_handle;
    static constexpr const char* API_URL = "https://blockchain.info/ticker";
public:
    BitcoinData() : m_curl_handle(API_URL)
    {}

    nlohmann::json fetch_bitcoin_data()
    {
        cout << "Fetching bitcoin data" << std::endl;
        m_curl_handle.fetch();
        return nlohmann::json::parse(m_curl_handle.get_fetched_data());
    }
};

int download_btc_data()
{
    int res = 0;
    try
    {
        BitcoinData bitcoin;
        const auto& bitcoin_json = bitcoin.fetch_bitcoin_data();

        const auto& bitcoin_to_usd_value_it = bitcoin_json.find("USD");

        if (bitcoin_to_usd_value_it != bitcoin_json.end())
        {
            res = bitcoin_to_usd_value_it.value()["last"].get<int>();

            printf("BTC = \t(%3s)%10d %s\n", bitcoin_to_usd_value_it.key().c_str(),
                bitcoin_to_usd_value_it.value()["last"].get<int>(),
                bitcoin_to_usd_value_it.value()["symbol"].get<string>().c_str());
        }
    }
    catch (std::exception e)
    {
        std::cerr << "Failed to fetch bitcoin exchange rates. error=" << e.what() << std::endl;
    }
    return res;
}

void test_async_libcurl1()
{
    using namespace std;
    const static uint64_t max_task = 10;
    const static std::chrono::seconds refresh_rate_s = 1s;
    std::vector<std::future<int>> futures;
    uint64_t count = 0;
    while (count < max_task)
    {
        ++count;
        futures.push_back(std::move(std::async(std::launch::async, download_btc_data)));
        std::this_thread::sleep_for(refresh_rate_s);
    }

    std::cout << "Before wait" << std::endl;

    for (const auto& f : futures)
    {
        f.wait();
    }

    std::cout << "After wait" << std::endl;
}

int main()
{
    test_async_libcurl1();

    system("pause");
    return 0;
}