/*
This file contains necessary code that describes/ mimics a candlestick
CandleStick = a replica object of a candlestick
*/

#ifndef CANDLESTICK_HPP
#define CANDLESTICK_HPP

#include "defs.hpp"
#include "level_info.hpp"
#include <limits>

/*Object storing information about the candlestick
@param open open of the candle
@param high high of the candle
@param low low of the candle
@param close close of the candle
@time time open time of the candle
@footprint footprint of the candle
*/
class CandleStick{
public:
    double imbalance_level = 3; // minimum ratio between bid and ask to indicate imbalance
    double percentage = 0.7; // Percentage of volume used to calculate the value area. @note Should be in ratio e.g 0.7 instead of 70%
    
    CandleStick() = default;
    
    CandleStick(Price open, Price high, Price low, Price close, time_t time, std::map<Price, Level, std::greater<Price>> &footprint){
        _open = open;
        _high = high;
        _low = low;
        _close = close;
        _time_stamp = time;
        _footprint = footprint;
    }
    
    //@return opening time of the candle
    time_t time_stamp() const { return _time_stamp;}
    
    //@return open of the candle
    Price open() const {return _open;}
    
    //@return close of the candle
    Price close() const {return _close;}
    
    //@return high of the candle
    Price high() const {return _high;}
    
    //@return low of the candle
    Price low() const {return _low;}
    
    /*@return price with the highest volume*/
    Price cot() {
        if (!_is_set) _set_info();
        return _cot;
    }
    
    //@return Price with the highest ask volume
    Price ask_cot(){
        if (!_is_set)_set_info();
        return _acot;
    }
    
    //@return Price with the highest bid volume
    Price bid_cot(){
        if (!_is_set) _set_info();
        return _bcot;
    }
    
    /*@return volume weighted price of the candlestick*/
    Price vwap() {
        if (!_is_set) _set_info();
        return _vwap;
    }
    
    /*@return Value area high of the candlestick*/
    Price vah(){
        if (_vah == -1) _value_area(percentage);
        return _vah;
    }
    
    /*@return value area low of the candlestick*/
    Price val(){
        if (_val == -1) _value_area(percentage);
        return _val;
    }
    
    //@return total ask volume
    Quantity ask_vol() {
        if (!_is_set) _set_info();
        return _ask_vol;}
    
    //@return total bids volume
    Quantity bid_vol() {
        if (!_is_set) _set_info();
        return _bid_vol;
    }
    
    //@return delta of the candle
    Quantity delta() { 
        return bid_vol()- ask_vol(); 
    }
    
    //@return maximum delta in the candle
    Quantity max_delta() {
        if (!_is_set) _set_info();
        return _max_delta; 
    }
    
    //@return minimum delta in the candle
    Quantity min_delta() { 
        if (!_is_set) _set_info();
        return _min_delta; 
    }
    
    //@return total volume traded
    Quantity volume() {return bid_vol()+ask_vol();}
    
    /*@return number of buying imbalance in the candlestick*/
    size_t buy_imbalance(){
        if (!_is_set) _set_info();
        return _buy_imb;
    }
    
    /*@return number of selling imbalance in the candlestick*/
    size_t sell_imbalance(){
        if (!_is_set) _set_info();
        return _sell_imb;
    }

    /*@return map containing the footprint*/
    const std::map<Price, Level, std::greater<Price>> &footprint() const {return _footprint;}
    
    /*Recalculates the value area using the percentage given.
    @param percentage percentage of the value area
    @note percentage should be in ratio e.g 0.7 instead of 70%*/
    void set_va(double percentage){
        _value_area(percentage);
    }
    
    /*Prints the footprint of the candle stick. @note colors indicates imabalance. Green = buy imbalance, Red = sell imbalance*/
    void print_fp() {
        for (auto &x : _footprint){
            if (x.first == cot())
                std::cout << "\033[33m";
            std::cout << x.first << " -> ";
            if (x.second.buy_imbalance(imbalance_level)) 
                std::cout << "\033[92m" << x.second.bids << "\033[0m" << "\t\t"<< x.second.asks;
            else if (x.second.sell_imbalance(imbalance_level))
                std::cout << x.second.bids << "\t\t" << "\033[91m"<< x.second.asks << "\033[0m";
            else
                std::cout << x.second.bids << "\t\t"<< x.second.asks ;
            std::cout << "\033[0m" << "\n";
        } 
    }

    /*Prints the delta in the candlestick. @note Green = positive delta, Red = negative delta*/
    void print_delta() {
        for (auto &x : _footprint){
            if (x.first == cot())
                std::cout << "\033[33m";
            std::cout << x.first << " -> " ;
            if (x.second.bids > x.second.asks) 
                std::cout << "\033[92m" << x.second.bids - x.second.asks << "\033[0m";
            else if (x.second.asks > x.second.bids)
                std::cout << "\033[91m"<< x.second.bids - x.second.asks << "\033[0m";
            else
                std::cout << x.second.bids - x.second.asks ;
            std::cout << "\n";
        }
    }

    /*Prints the volume bar and the associated volume @note Green = positive delta, Red = negative delta*/
    void print_bar(){
        int bars = _footprint.size() * 8;
        for (auto &x : _footprint){
            if (x.first == cot())
                std::cout << "\033[33m";
            std::cout << x.first << " -> ";

            for (int i = 0; i <= (x.second.asks + x.second.bids)*bars/volume(); i++)
                std::cout << "⬜";
            if (x.second.bids > x.second.asks)
                std::cout << "\033[92m" ; //set color to green
            else
                std::cout << "\033[91m"; // set color to red
            std::cout << " " << x.second.asks + x.second.bids  << "\033[0m" << "\n";
            
        }
    }

    friend std::ostream &operator<<(std::ostream &out, CandleStick &obj){
        out << obj._open << " " << obj._high << " " << obj._low << " " << obj._close << " " << obj._time_stamp << " " << obj._footprint.size();
        for (auto &p : obj._footprint){
            out << " " << p.second;
        }
        return out;
    }

    friend std::ostream &operator<<(std::ostream &out, CandleStick &&obj){
        out << obj;
        return out;
    }
    
    friend std::istream &operator >>(std::istream &in, CandleStick &obj){
        int level_size = 0;
        in >> obj._open >> obj._high >> obj._low >> obj._close >> obj._time_stamp >> level_size;

        if (in.eof()) return in;
        for (int i = 0; i < level_size; i++){
            Level temp;
            in >> temp;
            obj._footprint[temp.price] = temp;            
        }
        return in;
    }
    
private:
    Price _open, _high, _low, _close;
    time_t _time_stamp;

    Quantity _ask_vol = 0, _bid_vol = 0, _max_vol = 0;
    std::map<Price, Level, std::greater<Price>> _footprint;
    size_t _buy_trans = 0, _sell_trans = 0;
    Quantity _max_delta = std::numeric_limits<Quantity>::lowest(), _min_delta = std::numeric_limits<Quantity>::max();

    Price _cot; // COT = commitment of traders AKA POC price wtih the highest volume in the candlestick
    Price _vah = -1, _val = -1, _vwap = -1; // VAH = value area high. VAL = value area low
    size_t _buy_imb = 0, _sell_imb = 0; // number of buy and sell imbalance

    Price _bcot; // Bids commitment of traders
    Price _acot; // Asks commitment of traders
    Quantity _max_ask = 0, _max_bid = 0;
    bool _is_set = false;

    /*Calculates and sets information such as max_delta, bid volume, vwap etc*/
    void _set_info(){
        double pv = 0; // pv = price * volume
        for (const auto &it : _footprint){
            _setter(it.second);
            pv += it.first * (it.second.asks+it.second.bids);
        }
        _is_set = true;
        _vwap = pv/volume();
    }
    
    /*helper function for _set_info()*/
    void _setter(const Level &temp){
        Quantity del, vol;
        del = temp.bids - temp.asks;
        vol = temp.bids + temp.asks;
        if (vol > _max_vol){
            _max_vol = vol;
            _cot = temp.price;
        }
        if (temp.bids > _max_bid){
            _max_bid = temp.bids;
            _bcot = temp.price;
        }
        if (temp.asks > _max_ask){
            _max_ask = temp.asks;
            _acot = temp.price;
        }
        if (del > _max_delta) _max_delta = del;
        if (del < _min_delta) _min_delta = del;
        _ask_vol += temp.asks;
        _bid_vol += temp.bids;
        if (temp.buy_imbalance(imbalance_level)) _buy_imb++;
        if (temp.sell_imbalance(imbalance_level)) _sell_imb++;
    }
    
    /*Finds the value area high and value area low of the footprint given percentage
    @param percentage The percentage of total volume the the VAH and VAL should enclose
    */
    void _value_area(double perecentage){
        if (percentage > 1.0) percentage = 1.0;
        if (!_is_set) _set_info();
        const auto &it = _footprint.find(_cot);
        Quantity vol = it->second.asks + it->second.bids;
        const Quantity total_vol = volume();
        std::map<Price, Level, std::greater<Price>>::iterator up;
        std::map<Price, Level, std::greater<Price>>::iterator down;
        if (it == _footprint.begin()){ 
            up = it;
            down = std::next(it);
        }
        else if (std::next(it) == _footprint.end()){
            down = it;
            up = std::prev(it);
        }
        else {
            up = std::prev(it);
            down = std::next(it);
        }
        bool reached_top = (up == it);
        bool reached_bottom = (down == it);
        while (vol < perecentage*total_vol){
            Quantity down_vol = 0, up_vol = 0;
            
            if (!reached_bottom) down_vol = down->second.asks + down->second.bids;
            if (!reached_top) up_vol = up->second.bids + up->second.asks;

            if (down_vol > up_vol){
                vol += down_vol;
                if (std::next(down) != _footprint.end()) down = std::next(down);
                else reached_bottom = true;
            }
            else if (up_vol > down_vol){
                vol += up_vol;
                if (up != _footprint.begin()) up = std::prev(up);
                else reached_top = true;
            }
            else {
                vol += up_vol + down_vol;
                if (std::next(down) != _footprint.end()) down = std::next(down);
                else reached_bottom = true;
                if (up != _footprint.begin()) up = std::prev(up);
                else reached_top = true;
            } 
        }
        if (!reached_top) up = std::next(up);
        if (!reached_bottom) down = std::prev(down);
        _vah = up->first;
        _val = down->first;
    }
    
};

#endif
