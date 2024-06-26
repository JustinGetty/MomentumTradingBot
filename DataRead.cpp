#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>  
#include <fstream>
#include <ctime>
#include "StockDataStruct.h"
#include <string>

using namespace std;
using json = nlohmann::json;


//std::string api_key = "https://yahoo-finance127.p.rapidapi.com/historic/tsla/1h/15d";
//https://twelve-data1.p.rapidapi.com/time_series?outputsize=30&symbol=AMZN&interval=1day&format=json

std::string dblToStr(double x){
    std::string y = to_string(x);
    return y;
}

std::string fixLngthDbl(std::string x){
    std::string temp;
    double y;
    for(size_t i = 0; i < 12; i++){
        temp.push_back(x[i]);
    }
    return temp;

}

//"Timestamp, Open, High, Low, Close, Volume"
void printData(StockData data){
    //std::string output = 
    

    std::cout << "|" << data.timestamp << " | " << 
    dblToStr(data.open) << " | " << 
    dblToStr(data.high) << " | " <<     
    dblToStr(data.low) << " | " << 
    dblToStr(data.close) << " | " << 
    fixLngthDbl(dblToStr(data.volume)) << " | " << std::endl;

    //printf("%s | %f | %f | %f | %f | %f", data.timestamp);
}

std::string convertUnixTimestamp(double timestamp) {
    std::time_t t = static_cast<time_t>(timestamp);
    char buffer[80];
    struct tm* timeinfo;
    timeinfo = gmtime(&t);  
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);  
    return std::string(buffer);
}

std::string defineStockParams(string ticker, string range, string granularity){

    std::string requestStr = "https://twelve-data1.p.rapidapi.com/time_series?outputsize=";

    std::string combined = requestStr + range + "&symbol=" + ticker + "&interval=" + granularity + "&format=json";
    return combined;

}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::vector<StockData> processHistoricData(string ticker, string range, string granularity){
    CURL* hnd;
    CURLcode ret;
    std::string readBuffer;

    string api_key = defineStockParams(ticker, range, granularity);

    std::vector<StockData> stock_data;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    hnd = curl_easy_init();
    if(hnd) {
        curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(hnd, CURLOPT_URL, api_key.c_str());

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "x-rapidapi-key: b5a656e741mshf96723c09bf1ce5p1506c7jsn4183363ab252");
        headers = curl_slist_append(headers, "x-rapidapi-host: twelve-data1.p.rapidapi.com");
        curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &readBuffer);

        ret = curl_easy_perform(hnd);
        if(ret != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));
        } else {
            try {
                auto json_response = json::parse(readBuffer);
                //prints raw json - ignore
                std::cout << "Parsed JSON: " << json_response.dump(4) << std::endl;


                if (json_response.contains("values")) {
                    auto quotes = json_response["values"];

                    for (const auto& quote : quotes) {
                        StockData data;
                        data.timestamp = quote["datetime"].get<std::string>();
                        data.open = std::stod(quote["open"].get<std::string>());
                        data.high = std::stod(quote["high"].get<std::string>());
                        data.low = std::stod(quote["low"].get<std::string>());
                        data.close = std::stod(quote["close"].get<std::string>());
                        data.volume = std::stod(quote["volume"].get<std::string>());
                        stock_data.push_back(data);
                    }
                    
                } else {
                    std::cout << "Expected keys not found in JSON response." << std::endl;
                }
            } catch (json::parse_error& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            }
        }

        curl_easy_cleanup(hnd);
        curl_slist_free_all(headers);
    }

    curl_global_cleanup();
    return stock_data;
/*
   //makes fake data to avoid api usage
    StockData data;
    data.timestamp = "2024-05-17 13:30:00";
    data.open =  173.750000;
    data.close = 175.794693;
    data.high = 175.899994;
    data.low = 172.750000;
    data.volume = 15893921.000000;
    stock_data.push_back(data);
    return stock_data;
    */
}

void printDataFrame(std::vector<StockData> data_frame){
    std::cout << " Timestamp            Open         High         Low          Close        Volume" << std::endl;
	std::cout << "-----------------------------------------------------------------------------------------" << std::endl;	
        for(size_t i = 0; i < data_frame.size(); i++){
            printData(data_frame[i]);
        }
		
        std::cout << std::endl;


}



