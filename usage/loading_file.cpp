#include "levels_agg.hpp"
#include "chart.hpp"

using namespace std;

int main(){
    const char *file_path = "{path}.csv"; // location of time and sales data
    const char *store_path = "{path}.txt"; // where you want to store the aggregated data

    size_t cols = 6; //colums = {"id", "price", "qty", "quote_qty", "time", "bim"} column names as gotten from binance
    /*If you want to use with other data source make sure to change the indexes to correspond with the respective columns. i.e
    levels::price_id;
    levels::qty_id;
    levels::time_id;
    levels::is_buyer_maker_id;
    Index starts from zero
    */
   
    int price_interval = 3; // difference in footprint price levels
    int time_interval = 15 * 60; // 15 minute interval

    /*Aggregating and storing in a vector of candlestick*/
    vector<CandleStick> candles;
    size_t line = levels::agg(file_path, cols, candles, price_interval, time_interval, false); // aggregating futures data
    size_t line2 = levels::agg(file_path, cols, candles, price_interval, time_interval, true); // aggregating spot data

    
    /*Aggregating and storing in a vector of candlestick with multithreading.Faster but uses large memory
    When using this, you can change data::MAX_RAM_USE to set the RAM usage according to your requirement. Default is 1000 MB
    */
    size_t line7 = levels::agg_thread(file_path, cols, candles, price_interval, time_interval, false); // aggregating futures data
    size_t line8 = levels::agg_thread(file_path, cols, candles, price_interval, time_interval, true); // aggregating spot data


    /*Aggregating and storing in a text file. This is recommended as you only aggregate once.
    */
    size_t line3 = levels::agg_store(file_path, cols, store_path, price_interval, time_interval, false); // aggregating futures data
    size_t line4 = levels::agg_store(file_path, cols, store_path, price_interval, time_interval, true); // aggregating spot data

    
    /*Aggregating and storing in a text file with multithreading. Faster but uses large memory
    When using this, you can change data::MAX_RAM_USE to set the RAM usage according to your requirement. Default is 1000 MB
    */
    size_t line5 = levels::agg_store_thread(file_path, cols, store_path, price_interval, time_interval, false); // aggregating futures data
    size_t line6 = levels::agg_store_thread(file_path, cols, store_path, price_interval, time_interval, true); // aggregating spot data

    // loading the aggergated text file into chart object
    Chart chart;
    chart.load(store_path);

    // loading the aggregated text file into vector of candles
    data::File file;
    file.open_except(store_path, ios::in);
    while (1){
        CandleStick temp;
        file >> temp;
        if (file.eof()) break;
        candles.push_back(temp);
    }

    return 0;
}
