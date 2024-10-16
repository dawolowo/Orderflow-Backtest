/*
This file contains code to aggregrate time and sales data specifically from binance.
Aggregrate in this case means to restructure the data like a footprint chart
Footprint = a candlestick that contains traded bid and ask volume at various price intervals
*/

#ifndef LEVELS_AGG_H
#define LEVELS_AGG_H
#include "defs.h"
#include "data.h"
#include "level_info.h"
#include "candlestick.h"

namespace levels{
    const char *price_column = "price", *qty_column = "qty", *time_column = "time", *is_buyer_maker_column = "bim";

    /*@brief Checks if two time are within the same time interval.
    @tparam x first time
    @tparam y second time
    @tparam interval interval
    @note interval should be in seconds
    */
    bool within_interval(time_t x, time_t y, const int &interval){
        x /= 1000;   // I am dividing because this time format is in milliseconds
        y /= 1000;        
        return x/interval == y/interval;
    }
    /*@brief Fills footprint parameter with the necessary information about the price level such as bid, ask.
    @tparam footprint map containing the footprint information
    @tparam row unordered map containing the row that was read
    @tparam price_interval price interval between each price level. It determines each price level of the footprint
    */
    void set_price_level(std::map<Price, Level, std::greater<Price>> &footprint, std::unordered_map<const char *, std::string> &row,
             const Price price_interval){
        
        Price price = stod(row[price_column]);
        Quantity quantity = stod(row[qty_column]);
        bool bid = (row[is_buyer_maker_column][0] == 'F' || row[is_buyer_maker_column][0] == 'f');
        Price rem = price/price_interval - (int)(price/price_interval);
        Price level = (price/price_interval - rem) * price_interval;
        level += price_interval; // to make the price level upper bound
        footprint[level].price_ = level;
        if (bid) footprint[level].bids_ += quantity;
        else footprint[level].asks_ += quantity;
    }
    
    size_t __agg__(const char *path, const std::vector<const char *> &column_names, const char *store_path, std::vector<CandleStick> &candles,
            const Price price_level_interval, const int time_interval, bool store, bool spot){
        
        data::open_file(path);
        size_t no_of_lines = 1;
        // The next two lines are not needed if the data is binance spot. The first row is of binance futures data is column names
        if (!spot){
            std::string _;
            data::file_in >> _;
        }        

        Price high, low, close, open;
        time_t timestamp, prev_time ;
        std::map<Price, Level, std::greater<Price>> footprint;
        std::fstream file;
        if (store) file.open(store_path, std::ios::out);
        while (true){
            std::unordered_map<const char *, std::string> row;
            data::stream_file(column_names, row);
            if (no_of_lines == 1){
                high = low = open = stod(row[price_column]);
                prev_time = timestamp = stoll(row[time_column]);
            }
            if (row.empty() || data::file_in.eof()){
                if (store){
                    CandleStick temp(open, high, low, close, timestamp, footprint);
                    file << temp << '\n';
                }
                else candles.push_back(CandleStick(open, high, low, close, timestamp, footprint));
                break;
            }
            time_t curr_time = stoll(row[time_column]);
            Price t_price = stod(row[price_column]);
            if (!within_interval(prev_time, curr_time, time_interval)){
                if (store){
                    CandleStick temp(open, high, low, close, timestamp, footprint);
                    file << temp << '\n';
                }
                else candles.push_back(CandleStick(open, high, low, close, timestamp, footprint));
                footprint.clear();
                low = open = high = t_price;
                timestamp = curr_time;
            }
            set_price_level(footprint, row, price_level_interval);
            high = (high > t_price) ? high : t_price;
            low = (low < t_price) ? low  : t_price;
            close = t_price;
            prev_time = curr_time;
            no_of_lines++;
        }
        file.close();
        data::close_file();
        return no_of_lines;
    }
    /*@brief Aggregates the data and fills the candles parameter with the candlestick.
    @tparam path location of the file to be read from
    @tparam time_interval time interval (in seconds)
    @tparam candles vector that will contain the candlesticks
    @tparam price_level_interval the price difference between each price level. It determines each price level of the footprint
    @tparam is_spot set to true if the data is binance spot data. Default is false.
    @return number of lines read
    */
    size_t agg(const char *path, const std::vector<const char *> &column_names, std::vector<CandleStick> &candles,
            const Price price_level_interval, const int time_interval, bool is_spot = false){
        
        return __agg__(path, column_names, "", candles, price_level_interval, time_interval, false, is_spot);        
    }

    /*@brief Aggregates the data and stores it in the location of store_path.
    @tparam path location of the file
    @tparam time_interval time interval (in seconds)
    @tparam store_path location of the text file that will contain the aggregated data
    @tparam price_level_interval the price difference between each price level. It determines each price level of the footprint
    @tparam is_spot set to true if the data is binance spot data. Default is false.
    @return number of lines read
    */
    size_t agg_store(const char *path, const std::vector<const char *> &column_names, const char *store_path,
            const Price price_level_interval, const int time_interval, bool is_spot = false){
        
        std::vector<CandleStick> candles;
        return __agg__(path, column_names, store_path, candles, price_level_interval, time_interval, true, is_spot);
    }

}

#endif
