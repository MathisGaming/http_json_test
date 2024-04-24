#define CURL_STATICLIB
#include <iostream>
#include <thread>
#include <future>
#include <queue>
#include <vector>
#include <algorithm>
#include <shared_mutex>
#include <optional>

#include "include/json/json.hpp"
#include "http_data_loader.h"
#include "curl.h"

using namespace std;

constexpr const char* BTC_API_URL = "https://blockchain.info/ticker";

nlohmann::json fetchJSONFromUrl(std::string && url) {
    HttpDataLoader curlHandle(url);
    curlHandle.fetch();
    return nlohmann::json::parse(curlHandle.getData());
}

struct CurrencyData {
    int price;
    std::string symbol;
};

template <typename T, typename Iter>
auto getCryptoData(Iter iter, std::string&& text) {
    using ReturnType = std::remove_reference_t<decltype(std::declval<T>())>;
    return std::forward<ReturnType>(iter.value()[std::move(text).c_str()].get<ReturnType>());
}

std::optional<CurrencyData> downloadBTCData() {
    std::this_thread::sleep_for(1s);
    CurrencyData btcData;
    try {
        const auto bitcoinJSON = fetchJSONFromUrl(BTC_API_URL);
        const auto bitcoinToUSDIt = bitcoinJSON.find("USD");
        if (bitcoinToUSDIt != bitcoinJSON.end()) {
            btcData.price = getCryptoData<int>(bitcoinToUSDIt, "last");
            btcData.symbol = getCryptoData<std::string>(bitcoinToUSDIt, "symbol");
        }
    }
    catch (std::exception e) {
        std::cerr << "Failed to fetch bitcoin exchange rates. error=" << e.what() << std::endl;
        return std::nullopt;
    }
    return btcData;
}

void displayData(const CurrencyData& data) {
    printf("%s = %i\n", data.symbol.c_str(), data.price);
}

int main() {
    constexpr uint64_t MAX_TASKS = 3;
    std::vector<std::future<std::optional<CurrencyData>>> futures;
    std::cout << "Fetchig BTC data..." << std::endl;
    for (uint64_t i = 0; i < MAX_TASKS; ++i) {
        std::this_thread::sleep_for(1s);
        futures.push_back(std::move(std::async(std::launch::async, downloadBTCData)));
    }

    for (auto&& f : futures){
        if (auto result = f.get(); result != std::nullopt) {
            displayData(result.value());
        }
    }
    std::cout << "Finished fetching BTC data." << std::endl;

    system("pause");
    return 0;
}