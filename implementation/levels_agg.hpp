/*
This file contains code to aggregrate time and sales data specifically from binance.
Aggregrate in this case means to restructure the data like a footprint chart
Footprint = a candlestick that contains traded bid and ask volume at various price intervals
*/
#pragma once

#include "defs.hpp"
#include "data.hpp"
#include "level_info.hpp"
#include "candlestick.hpp"
#include <thread>

namespace levels{

    //The column index of price, quantity, time, buyer is maker
    size_t price_id = 1, qty_id = 2, time_id = 4, is_buyer_maker_id = 5;

    namespace __internal_function {
        using namespace std;

        /*@brief Checks if two time are within the same time interval.
        @param x first time
        @tparam y second time
        @param interval interval
        @note interval should be in seconds
        */
        inline bool __within_interval__(time_t x, time_t y, const int &interval){
            x /= 1000;   // I am dividing because this time format is in milliseconds
            y /= 1000;        
            return x/interval == y/interval;
        }

        /*@brief Fills footprint parameter with the necessary information about the price level such as bid, ask.
        @param footprint map containing the footprint information
        @param row unordered map containing the row that was read
        @param price_interval price interval between each price level. It determines each price level of the footprint
        */
        inline void __set_price_level__(std::map<Price, Level, std::greater<Price>> &footprint, std::vector<std::string> &row,
                const Price &price_interval){
            
            Price price = stof(row[price_id]);
            Quantity quantity = stof(row[qty_id]);
            //is_bid represents if a trade is an aggressive buy/ passive sell. True if it's aggressive buy
            bool is_bid = row[is_buyer_maker_id][0] == 'F' || row[is_buyer_maker_id][0] == 'f';
            Price level = ((int)(price/price_interval + 1)) * price_interval; //Upper bounded level
            Level &x = footprint[level];
            x.price = level;
            if (is_bid) x.bids += quantity;
            else x.asks += quantity;
            
        }

        inline void __write__(data::FileStream &out, Price &open, Price &high, Price &low, Price &close, time_t &time, std::map<Price, Level, std::greater<Price>> &footprint){
            out << open << ' ' << high << ' ' << low << ' ' << close << ' ' << time << ' ' << footprint.size();
            for (auto &p : footprint){
                out << ' ' << p.second;
            }
            out << '\n';
        }

        //Separates the line by ','
        inline void __split__(std::vector<std::string> &row, const std::string &line){
            size_t n = 0;
            row[n] = "";
            for (const char &x : line){
                if (x == ','&& n+1 < row.size()){
                    row[++n] = "";
                }
                else if (x == ',') n++;
                else if (n < row.size()) row[n] += x;
                else break;
            }
        }


        inline size_t __agg__(const char *path, size_t no_cols, const char *store_path, std::vector<CandleStick> &candles,
                const Price price_level_interval, const int time_interval, const bool store, const bool spot){
            
            data::FileStream file_in;
            file_in.open_except(path, std::ios::in);
            size_t no_of_lines = 1;
            // The next two lines are not needed if the data is binance spot. The first row is of binance futures data is column names
            if (!spot){
                std::string _;
                file_in >> _;
                char __;
                file_in.get(__); // This line is important to get rid of the \n character. FIXED BUG
            }        

            Price high, low, close, open, t_price;
            time_t timestamp, prev_time,curr_time ;
            std::map<Price, Level, std::greater<Price>> footprint;
            data::FileStream file_out;
            if (store) file_out.open_except(store_path, std::ios::out);
            std::vector<std::string> row(no_cols);
            
            while (true){
                data::stream_file(no_cols, row, file_in);
                if (file_in.eof()){
                    if (store){
                        __write__(file_out, open, high, low, close, timestamp, footprint);
                    }
                    else candles.emplace_back(open, high, low, close, timestamp, footprint);
                    break;
                }

                curr_time = stoll(row[time_id]);
                t_price = stof(row[price_id]);
                if (no_of_lines == 1){
                    high = low = open = t_price;
                    prev_time = timestamp = curr_time;
                }
                if (!__within_interval__(prev_time, curr_time, time_interval)){
                    if (store){
                        __write__(file_out, open, high, low, close, timestamp, footprint);
                    }
                    else candles.emplace_back(open, high, low, close, timestamp, footprint);
                    footprint = {};
                    low = open = high = t_price;
                    timestamp = curr_time;
                }
                __set_price_level__(footprint, row, price_level_interval);
                high = (high > t_price) ? high : t_price;
                low = (low < t_price) ? low  : t_price;
                close = t_price;
                prev_time = curr_time;
                no_of_lines++;
            }
            return no_of_lines;
        }
        
        inline size_t __tagg__(const char *path, size_t no_cols, const char *store_path, std::vector<CandleStick> &candles,
                const Price price_level_interval, const int time_interval, const bool store, const bool spot){
            data::FileStream file_in;
            file_in.open_except(path, std::ios::in);
            size_t no_of_lines = 1;
            //  The first row is of binance futures data is column names so we it get rid of it
            if (!spot){
                std::string _;
                file_in >> _;
                char __;
                file_in.get(__); // This line is important to get rid of the \n character. FIXED BUG
            }        

            Price high, low, close, open, t_price;
            time_t timestamp, prev_time, curr_time ;
            std::map<Price, Level, std::greater<Price>> footprint;
            data::FileStream file_out;
            if (store) file_out.open_except(store_path, std::ios::out);
            std::vector<std::string> row(no_cols);
            AtomicQueue<std::string> buffer;        
            std::thread worker(data::thread_stream, std::ref(buffer), std::ref(file_in));
            
            while (true){
                if (file_in.eof() && buffer.empty()){
                    if (store){
                        __write__(file_out, open, high, low, close, timestamp, footprint);
                    }
                    else candles.emplace_back(open, high, low, close, timestamp, footprint);
                    break;
                }
                if (buffer.empty()) continue;
                __split__(row, buffer.front());
                buffer.pop();

                curr_time = stoll(row[time_id]);
                t_price = stof(row[price_id]);
                if (no_of_lines == 1){
                    high = low = open = t_price;
                    prev_time = timestamp = curr_time;
                }
                if (!__within_interval__(prev_time, curr_time, time_interval)){
                    if (store){
                        __write__(file_out, open, high, low, close, timestamp, footprint);
                    }
                    else candles.emplace_back(open, high, low, close, timestamp, footprint);
                    footprint = {};
                    low = open = high = t_price;
                    timestamp = curr_time;
                }
                __set_price_level__(footprint, row, price_level_interval);
                high = (high > t_price) ? high : t_price;
                low = (low < t_price) ? low  : t_price;
                close = t_price;
                prev_time = curr_time;
                no_of_lines++;
            }
            worker.join();
            return no_of_lines;
        }

    }
    //__internal_function end
    


    /*@brief Aggregates the data and fills the candles parameter with the candlestick.
    @param path location of the file to be read from
    @param no_cols number of columns in the csv file    @param candles vector that will contain the candlesticks
    @param price_level_interval the price difference between each price level. It determines each price level of the footprint
    @param time_interval time interval (in seconds)
    @param is_spot set to true if the data is binance spot data. Default is false.
    @return number of lines read
    */
    size_t agg(const char *path, size_t no_cols, std::vector<CandleStick> &candles,
            const Price price_level_interval, const int time_interval, bool is_spot = false){
        
        return __internal_function::__agg__(path, no_cols, "", candles, price_level_interval, time_interval, false, is_spot);        
    }

    /*@brief Aggregates the data and stores it in the location of store_path.
    @param path location of the file
    @param no_cols number of columns in the csv file
    @param store_path location of the text file that will contain the aggregated data
    @param price_level_interval the price difference between each price level. It determines each price level of the footprint
    @param time_interval time interval (in seconds)
    @param is_spot set to true if the data is binance spot data. Default is false.
    @return number of lines read
    */
    size_t agg_store(const char *path, size_t no_cols, const char *store_path,
            const Price price_level_interval, const int time_interval, bool is_spot = false){
        
        std::vector<CandleStick> candles;
        return __internal_function::__agg__(path, no_cols, store_path, candles, price_level_interval, time_interval, true, is_spot);
    }

    /*@brief Aggregates the data and fills the candles parameter with the candlestick.
    
    Uses multithreading to make it faster but at the cost of RAM usage. To set the RAM usage check data::MAX_RAM_USE
    @param path location of the file to be read from
    @param no_cols number of columns in the csv file    @param candles vector that will contain the candlesticks
    @param price_level_interval the price difference between each price level. It determines each price level of the footprint
    @param time_interval time interval (in seconds)
    @param is_spot set to true if the data is binance spot data. Default is false.
    @return number of lines read
    */
    size_t agg_thread(const char *path, size_t no_cols, std::vector<CandleStick> &candles,
            const Price price_level_interval, const int time_interval, bool is_spot = false){
        
        return __internal_function::__tagg__(path, no_cols, "", candles, price_level_interval, time_interval, false, is_spot);        
    }

    /*@brief Aggregates the data and stores it in the location of store_path.
    
    Uses multithreading to make it faster but at the cost of RAM usage. To set the RAM usage check data::MAX_RAM_USE
    @param path location of the file
    @param no_cols number of columns in the csv file
    @param store_path location of the text file that will contain the aggregated data
    @param price_level_interval the price difference between each price level. It determines each price level of the footprint
    @param time_interval time interval (in seconds)
    @param is_spot set to true if the data is binance spot data. Default is false.
    @return number of lines read
    */
    size_t agg_store_thread(const char *path, size_t no_cols, const char *store_path,
            const Price price_level_interval, const int time_interval, bool is_spot = false){
        
        std::vector<CandleStick> candles;
        return __internal_function::__tagg__(path, no_cols, store_path, candles, price_level_interval, time_interval, true, is_spot);
    }
}
