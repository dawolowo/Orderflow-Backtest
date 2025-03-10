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
#include "datahandler.hpp"

namespace aggregator{

    namespace {

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
        inline void __set_price_level__(std::map<Price, Level, std::greater<Price>> &footprint, const RowData &row,
                const Price &price_interval){
           
            Price level = ((int)(row.price/price_interval + 1)) * price_interval; //Upper bounded level
            Level &x = footprint[level];
            x.price = level;
            if (row.buyer_is_taker) x.bids += row.volume;
            else x.asks += row.volume;
            
        }

        inline void __write__(data::FileStream &out, Price &open, Price &high, Price &low, Price &close, time_t &time, std::map<Price, Level, std::greater<Price>> &footprint){
            out << open << ' ' << high << ' ' << low << ' ' << close << ' ' << time << ' ' << footprint.size();
            for (auto &p : footprint){
                out << ' ' << p.second;
            }
            out << '\n';
        }


        inline size_t __tagg__(const std::string &path, RowData (*func) (data::FileStream &), const std::string &store_path, std::vector<CandleStick> &candles,
                const Price price_level_interval, const int time_interval, const bool store, size_t skip = 0){
            data::FileStream file_in;
            file_in.open_except(path, std::ios::in);
            size_t no_of_lines = 1;

            while (skip-- > 0){
                std::string _;
                getline(file_in, _);
            }        

            Price high, low, close, open;
            time_t timestamp, prev_time;
            std::map<Price, Level, std::greater<Price>> footprint;
            data::FileStream file_out;

            if (store) file_out.open_except(store_path, std::ios::out);

            RowData &&first = func(file_in);
            high = low = open = first.price;
            prev_time = timestamp = first.timestamp;
            __set_price_level__(footprint, first, price_level_interval);

            SafeQueue<RowData> buffer;        
            std::thread worker(data::thread_stream, std::ref(buffer), std::ref(file_in), func);
            
            while (true){
                if (file_in.eof() && buffer.empty()){
                    if (store){
                        __write__(file_out, open, high, low, close, timestamp, footprint);
                    }
                    else candles.emplace_back(open, high, low, close, timestamp, footprint);
                    break;
                }
                if (buffer.empty()) continue;
                const RowData &row = buffer.front();

                if (!__within_interval__(prev_time, row.timestamp, time_interval)){
                    if (store){
                        __write__(file_out, open, high, low, close, timestamp, footprint);
                    }
                    else candles.emplace_back(open, high, low, close, timestamp, footprint);
                    footprint = {};
                    low = open = high = row.price;
                    timestamp = row.timestamp;
                }
                __set_price_level__(footprint, row, price_level_interval);
                high = (high > row.price) ? high : row.price;
                low = (low < row.price) ? low  : row.price;
                close = row.price;
                prev_time = row.timestamp;
                no_of_lines++;
                buffer.pop();
            }
            worker.join();
            return no_of_lines;
        }

    }
    //namespace end
    

    /*@brief Aggregates the data and fills the candles parameter with the candlestick.
    
    @param path location of the file to be read from
    @param handler data handler. Basically a function that parses a line of csv and returns RowData.
    @param candles vector that will contain the candlesticks
    @param price_level_interval the price difference between each price level. It determines each price level of the footprint
    @param time_interval time interval (in seconds)
    @param skip number of lines to skip. Sometimes the first few lines are not data but other information e.g column names
    @return number of lines read
    */
    size_t aggregate(const std::string &path,  RowData (*handler) (data::FileStream &), std::vector<CandleStick> &candles,
            const Price price_level_interval, const int time_interval, size_t skip = 0){
        
        return __tagg__(path,  handler, "", candles, price_level_interval, time_interval, false, skip);        
    }

    /*@brief Aggregates the data and stores it in the location of store_path.
    
    @param path location of the file
    @param handler data handler. Basically a function that parses a line of csv and returns RowData.
    @param store_path location of the text file that will contain the aggregated data
    @param price_level_interval the price difference between each price level. It determines each price level of the footprint
    @param time_interval time interval (in seconds)
    @param skip number of lines to skip. Sometimes the first few lines are not data but other information e.g column names
    @return number of lines read
    */
    size_t aggregate_store(const std::string &path,  RowData (*handler) (data::FileStream &), const std::string &store_path,
            const Price price_level_interval, const int time_interval, size_t skip = 0){
        
        std::vector<CandleStick> candles;
        return __tagg__(path, handler, store_path, candles, price_level_interval, time_interval, true, skip);
    }
}
