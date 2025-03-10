/*
This file contains code to simulate a real life chart
Chart = class that mimics a real life chart. i.e a collection of candlesticks
Source = enum containing different points of application of an indicator
Swing = structure that contains information about swing points i.e swing high and swing lows

*/

#pragma once
#include "candlestick.hpp"
#include "defs.hpp"
#include <filesystem>
#include <cmath>

//Enum indicating the point of application of an indicator
enum class Source{
    open,
    high,
    low,
    close
};

struct Swing{
    size_t index; // where in the chart it occured. if index = -1 it doesn't exist
    Price price; // Price where it formed
    Source source; // typically high and low. high indicating swing high and low indicating swing low. source can also be close as some traders use
};

/*Collection of CandleSticks
@param candles vector of candles
@note move is called on ```candle``` i.e the contents in ```candles``` are moved not copied to the chart object
*/
class Chart{
public:
    Chart() = default;

    Chart(std::vector<CandleStick> &candles){
        _candles = std::move(candles); 
    }

    size_t size()const {return _candles.size();}
    
    //Returns true if chart is empty
    bool empty(){return _candles.empty();}

    /*Loads the data stored in a file to Chart object

    Data should contain the aggragrated data which is stored in .txt . To get this, data see agg_store()
    @param file_path path to the .txt file containing the aggregated data
    */
    void load(const char *file_path){ 
        std::filesystem::path filepath = file_path;
        if (filepath.extension() != ".txt") throw std::logic_error("cause = load() : file name should end with .txt\n");
        std::fstream file;
        file.open(file_path);
        if (!file) throw std::logic_error("cause = load() : File not opened. Incorrect file path or file does not exist\n");
        
        while (1){
            CandleStick c;
            file >> c;
            if (file.eof()) break;

            _candles.push_back(std::move(c));
        }
        file.close();
    }

    /*Applies simple moving average indicator to the chart.
    @param length period of the indicator e.g 14-period moving average
    @param source where it should be applied to i.e (close, open, high, low) of the candle. Default is close.
    @return Name of the indicator
    */
    std::string apply_sma(size_t length, Source source = Source::close){   
        std::string pre;
        if (Source::close == source) pre = "close";
        else if (Source::open == source) pre = "open";
        else if (Source::high == source) pre = "high";
        else if (Source::low == source) pre = "low";
        std::string name = "sma_" + pre + "_" + std::to_string(length);
        double sum = 0;
        size_t n = 1, rebalance = 0;
        for (size_t i = 0; i < _candles.size(); i++){
            Price x = _select(_candles[i], source);
            sum += x;
            if (i >= length){
                sum -= _select(_candles[rebalance], source);
                rebalance++;
            }
            _indicators[name].push_back(sum/n);
            if (n < length) n++;
        }
        return name;
    }

    /*Applies standard deviation indicator to the chart. 
    @param length period of the indicator e.g 14-period
    @param source where it should be applied to i.e (close, open, high, low) of the candle. Default is close.
    @return Name of the indicator*/
    std::string apply_std(size_t length, Source source = Source::close){
        std::string sma = apply_sma(length, source);
        std::string pre;
        if (Source::close == source) pre = "close";
        else if (Source::open == source) pre = "open";
        else if (Source::high == source) pre = "high";
        else if (Source::low == source) pre = "low";
        std::string name = "sma_" + pre + "_" + std::to_string(length);
        size_t n = 1, rebalance = 0;
        for (size_t i = 0; i < size(); i++){
            double temp = 0; // temp = ∑(x- x̄)²
            for (size_t j = rebalance; j < n+rebalance; j++)
                temp += pow(_select(_candles[j], source) - select_indicator(sma)[i], 2); // (x- x̄)²
            _indicators[name].push_back(sqrt(temp/n));
            if (n < length) n++;
            else rebalance++;
        }
        return name;
    }

    /* Applies your custom indicator to the chart
    @param name name of the indicator. It will be used to access your indicator
    @param data data of the indicator
    @note Ensure that look ahead bias is not being included in the data
    */
    void custom_indicator(const char *name, std::vector<Price> &data){        
        if (data.size() != _candles.size()) throw std::logic_error("cause = custom_indicator() : Data of indicator not equal to length of data in chart\n");
        _indicators[name] = data;
    }

    /*Adds a candle to the end of the chart
    @param c candle to be added
    */
    void push_back(CandleStick &c){
        _candles.push_back(c);
    }

    /*Adds a candle to the end of the chart
    @param c candle to be added
    */
    void push_back(CandleStick &&c){
        _candles.push_back(std::move(c));
    }

    /*@brief selects an indicator
    @return Data of the indicator selected
    @param name name of the indicator
    */
    const std::vector<Price> &select_indicator(std::string name){
        if (_indicators.find(name) == _indicators.end()) throw std::logic_error("cause = select_indicator() : Indicator does not exist\n");
        return _indicators[name];
    }

    /*@return vector containing candles*/
    std::vector<CandleStick> &candles() {return _candles;}

    CandleStick &operator[](size_t id){return _candles[id];}

private:
    std::vector<CandleStick> _candles;
    std::map<std::string, std::vector<Price>> _indicators;

    /*@return Data corresponding to source*/
    Price _select(const CandleStick &x, const Source &source){        
        if (source == Source::open) return x.open();
        else if (source == Source::high) return x.high();
        else if (source == Source::low) return x.low();
        else if (source == Source::close) return x.close();
        return 0;
    }
};
