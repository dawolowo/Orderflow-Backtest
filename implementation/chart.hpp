/*
This file contains code to simulate a real life chart
Chart = class that mimics a real life chart. i.e a collection of candlesticks
Source = enum containing different points of application of an indicator
Swing = structure that contains information about swing points i.e swing high and swing lows

*/

#ifndef CHART_HPP
#define CHART_HPP
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
class Chart{
public:
    Chart() = default;

    Chart(std::vector<CandleStick> &candles){_candles = candles; }

    size_t size()const {return _candles.size();}
    /*loads the data stored in a file to Chart object

    Data should contain the aggragrated data which is stored in .txt . To get this, data see agg_store()
    @tparam file_path path to the .txt file containing the aggregated data
    */
    void load(const char *file_path){
        
        std::filesystem::path filepath = file_path;
        if (filepath.extension() != ".txt") throw std::logic_error("cause = load() : file name should end with .txt\n");
        std::fstream file;
        file.open(file_path);
        if (!file) throw std::logic_error("cause = load() : File not opened. Incorrect file path or file does not exist\n");
        while (1){
            CandleStick temp;
            file >> temp;
            if (file.eof()) break;
            _candles.push_back(temp);
        }
        file.close();
    }
    /*Applies simple moving average indicator to the chart. 
    @return Name of the indicator
    */
    const char *apply_sma(int length, Source source = Source::close){
        
        std::string name = "sma_" + std::to_string(length);
        const char *f = name.c_str();
        double sum = 0;
        int n = 1, rebalance = 0;
        for (int i = 0; i < _candles.size(); i++){
            Price x = _select(_candles[i], source);
            sum += x;
            if (i >= length){
                sum -= _select(_candles[rebalance], source);
                rebalance++;
            }
            _indicators[f].push_back(sum/n);
            if (n < length) n++;
        }
        return f;
    }
    /*Applies standard deviation indicator to the chart. 
    @return Name of the indicator*/
    const char *apply_std(int length, Source source = Source::close){
        
        const char *sma = apply_sma(length, source);
        std::string name = "std" + std::to_string(length);
        const char *f = name.c_str();
        int n = 1, rebalance = 0;
        for (int i = 0; i < size(); i++){
            double temp = 0; // temp = ∑(x- x̄)²
            for (int j = rebalance; j < n+rebalance; j++)
                temp += pow(_select(_candles[j], source) - select_indicator(sma)[i], 2); // (x- x̄)²
            _indicators[f].push_back(sqrt(temp/n));
            if (n < length) n++;
            else rebalance++;
        }
        return f;
    }
    /* Applies your custom indicator to the chart
    @tparam name name of the indicator. It will be used to access your indicator
    @tparam data data of the indicator
    @note Ensure that look ahead bias is not being included in the data
    */
    void custom_indicator(const char *name, std::vector<Price> &data){
        
        if (data.size() != _candles.size()) throw std::logic_error("cause = custom_indicator() : Data of indicator not equal to length of data in chart\n");
        _indicators[name] = data;
    }
    /*@brief selects an indicator
    @return Data of the indicator selected
    @tparam name name of the indicator
    */
    const std::vector<Price> &select_indicator(const char *name){
        if (_indicators.find(name) == _indicators.end()) throw std::logic_error("cause = select_indicator() : Indicator does not exist\n");
        return _indicators[name];
    }
    /*@return vector containing candles*/
    const std::vector<CandleStick> &candles() const{return _candles;}
    CandleStick &operator[](size_t id){return _candles[id];}


private:
    std::vector<CandleStick> _candles;
    std::map<const char *, std::vector<Price>> _indicators;
    /*@return Data corresponding to source*/
    Price _select(const CandleStick &x, const Source &source){
        
        if (source == Source::open) return x.open();
        else if (source == Source::high) return x.high();
        else if (source == Source::low) return x.low();
        else return x.close();
    }
};

#endif
