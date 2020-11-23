#define CURL_STATICLIB
#include <iostream>
#include <thread>
#include <future>
#include <queue>
#include <vector>

#include "include/json/json.hpp"
#include "http_data_loader.h"
#include "threadsafe_queue.h"
#include "curl.h"

using json = nlohmann::json;

std::mutex g_mutex;

// Class to store the data
class Bitcoin
{
private:
    HttpDataLoader m_curl_handle;
    static constexpr const char* API_URL = "https://blockchain.info/ticker";
public:
    Bitcoin() : m_curl_handle(API_URL)
    {}

    json fetch_bitcoin_data()
    {
        m_curl_handle.fetch();
        return json::parse(m_curl_handle.get_fetched_data());
    }
};

void download_btc_data()
{
    try
    {
        std::lock_guard<std::mutex> lk(g_mutex);
        using namespace std;
        Bitcoin bitcoin;
        const auto& bitcoin_json = bitcoin.fetch_bitcoin_data();

        const auto& bitcoin_to_usd_value_it = bitcoin_json.find("USD");

        if (bitcoin_to_usd_value_it != bitcoin_json.end())
        {
            std::cout << "1 BTC = ";
            printf("\t(%3s)%10d %s\n", bitcoin_to_usd_value_it.key().c_str(),
                bitcoin_to_usd_value_it.value()["last"].get<int>(),
                bitcoin_to_usd_value_it.value()["symbol"].get<string>().c_str());
        }
    }
    catch (...)
    {
        std::cerr << "Failed to fetch bitcoin exchange rates \n";
    }
}

void test_asynhc_libcurl1()
{
    using namespace std;
    const uint64_t max_task = 10;
    const std::chrono::milliseconds refresh_rate_ms = 5000ms;
    std::vector<std::future<void>> futures;
    uint64_t count = 0;
    while (count < max_task)
    {
        ++count;
        futures.push_back(std::move(std::async(std::launch::async, download_btc_data)));
    }

    std::cout << "Before wait" << std::endl;

    std::this_thread::sleep_for(refresh_rate_ms);

    for (const auto& f : futures)
    {
        f.wait();
    }

    std::cout << "After wait" << std::endl;
}

int main()
{
    test_asynhc_libcurl1();

    system("pause");
    return 0;
}