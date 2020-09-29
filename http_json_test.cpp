#define CURL_STATICLIB
#include <iostream>
#include <thread>

#include "include/json/json.hpp"
#include "curl.h"

using json = nlohmann::json;

typedef std::unique_ptr<CURL, std::function<void(CURL*)>> CURL_ptr;

extern "C" std::size_t dataHandler(const char* buffer, std::size_t size, std::size_t nmeb, std::string * userData) {
    if (userData == nullptr) {
        return 0;
    }

    userData->append(buffer, (size * nmeb));
    return size * nmeb;
};

class CurlHandle {
private :
    CURL_ptr curlptr;
    std::string data;
    constexpr static auto deleter = [](CURL* c) {
        curl_easy_cleanup(c);
        curl_global_cleanup();
    };
public:
    CurlHandle() : curlptr(curl_easy_init(), deleter) {
        curl_global_init(CURL_GLOBAL_ALL);
        curl_easy_setopt(curlptr.get(), CURLOPT_WRITEFUNCTION, dataHandler);
        curl_easy_setopt(curlptr.get(), CURLOPT_WRITEDATA, &data);
    }

    void set_url(const std::string& url) {
        curl_easy_setopt(curlptr.get(), CURLOPT_URL, url.c_str());
    }

    CURLcode fetch() {
        return curl_easy_perform(curlptr.get());
    }

    std::string get_fetched_data() {
        return data;
    }
};

class Bitcoin {
private:
    CurlHandle curlHandle;
    static constexpr const char* API_URL = "https://blockchain.info/ticker";
public:
    Bitcoin() : curlHandle({}) {
        curlHandle.set_url(API_URL);
    }

    json fetch_bitcoin_data() {
        curlHandle.fetch();
        return json::parse(curlHandle.get_fetched_data());
    }
};

int main()
{
    using namespace std;
    uint8_t count = 0;
    while (count < 10) {
        ++count;
        try {
            Bitcoin bitcoin;
            json bitcoinData = bitcoin.fetch_bitcoin_data();

            cout << "1 BTC = ";
            const auto& bitcoin_to_usd_value_it = bitcoinData.find("USD");

            if (bitcoin_to_usd_value_it != bitcoinData.end()) {
                printf("\t(%3s)%10d %s\n", bitcoin_to_usd_value_it.key().c_str(),
                    bitcoin_to_usd_value_it.value()["last"].get<int>(),
                    bitcoin_to_usd_value_it.value()["symbol"].get<string>().c_str());
            }
        }
        catch (...) {
            cerr << "Failed to fetch bitcoin exchange rates \n";
        }
        std::this_thread::sleep_for(5s);
    }
    
    system("pause");

    return 0;
}