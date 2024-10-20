#include "levels_agg.hpp"
#include "chart.hpp"

using namespace std;

int main(){
    const char *file_path = ".csv"; // location of time and sales data
    const char *store_path = ".txt"; // where you want to store the aggregated data

    vector<const char *> cols = {"id", "price", "qty", "quote_qty", "time", "bim"}; // column names as gotten from binance
    /*cols should be left as it is. But if you want to change cols maybe to use with other data source
    make sure to change the following to correspond with the names in cols (by default it is)
    levels::price_column;
    levels::qty_column;
    levels::time_column;
    levels::is_buyer_maker_column;
    */
   
    int price_interval = 3; // difference in footprint price levels
    int time_interval = 15 * 60; // 15 minute interval

    /*Aggregating and storing in a vector of candlestick*/
    vector<CandleStick> candles;
    size_t line = levels::agg(file_path, cols, candles, price_interval, time_interval, false); // aggregating futures data
    size_t line2 = levels::agg(file_path, cols, candles, price_interval, time_interval, true); // aggregating spot data

    /*Aggregating and storing in a text file. This is recommended as you only aggregate once. Aggregating a 14 gb file on
    my system takes about 40 minutes while loading takes about 9 secs.
    */
    size_t line3 = levels::agg_store(file_path, cols, store_path, price_interval, time_interval, false); // aggregating futures data
    size_t line4 = levels::agg_store(file_path, cols, store_path, price_interval, time_interval, true); // aggregating spot data

    // loading the aggergated text file into chart object
    Chart chart;
    chart.load(store_path);

    // loading the aggregated text file into vector of candles
    fstream file;
    file.open(store_path);
    if (!file) throw logic_error("File not opened.\ncause: Incorrect file path or file does not exist");
    while (1){
        CandleStick temp;
        file >> temp;
        if (file.eof()) break;
        candles.push_back(temp);
    }
    file.close();

    return 0;
}
