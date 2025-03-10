#include "header/aggregator.hpp"
#include "header/chart.hpp"

/*
Tutorial: How to aggregate a binance time and sales data
*/

using namespace std;

int main(){
    const char *file_path = "{path}.csv"; // location of time and sales data
    const char *store_path = "{path}.txt"; // where you want to store the aggregated data


    Price price_interval = 3; // difference in footprint price levels
    int time_interval = 15 * 60; // 15 minute interval

    /*Aggregating and storing in a vector of candlestick*/
    vector<CandleStick> candles;

    /* aggregating futures data
   skip = 1 because we have skip the first line of a futures data contains row names
   */
    size_t line = aggregator::aggregate(file_path, handler::binance_handler, candles, price_interval, time_interval, 1);
    // aggregating spot data
    size_t line2 = aggregator::aggregate(file_path, handler::binance_handler, candles, price_interval, time_interval, 0); 


    /*Aggregating and storing in a text file. This is recommended as you only aggregate once.
    */

   /* aggregating futures data
   skip = 1 because we have skip the first line of a futures data contains row names
   */
    size_t line3 = aggregator::aggregate_store(file_path, handler::binance_handler, store_path, price_interval, time_interval, 1); 
    // aggregating spot data
    size_t line4 =aggregator::aggregate_store(file_path, handler::binance_handler, store_path, price_interval, time_interval, 0); 

    
    // loading the aggergated text file into chart object
    Chart chart;
    chart.load(store_path);

    // loading the aggregated text file into vector of candles
    data::FileStream file;
    file.open_except(store_path, ios::in);
    while (1){
        CandleStick temp;
        file >> temp;
        if (file.eof()) break;
        candles.push_back(temp);
    }

    return 0;
}
