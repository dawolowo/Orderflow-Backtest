/*
This file contains necessary code that describes/ mimics a candlestick
CandleStick = a replica object of a candlestick
*/

#pragma once

#include "defs.hpp"
#include "level_info.hpp"
#include "market_profile.hpp"
#include <utility>
#include <memory>
#include <limits>

/*Object storing information about the candlestick
@param open open of the candle
@param high high of the candle
@param low low of the candle
@param close close of the candle
@param time open time of the candle
@param footprint footprint of the candle
@note The data in ```footprint``` is moved into the object. After the constructor call, ```footprint``` would be empty.
*/
class CandleStick{
public:
    // minimum ratio between bid and ask to indicate imbalance
    double imbalance_level = 3; 
    // Percentage of volume used to calculate the value area. @note Should be in ratio e.g 0.7 instead of 70%
    double percentage = 0.7;
    
    CandleStick() = default;
    
    CandleStick(Price open, Price high, Price low, Price close, time_t time){
        _open = open;
        _high = high;
        _low = low;
        _close = close;
        _time_stamp = time;
    }

    CandleStick(Price open, Price high, Price low, Price close, time_t time, std::map<Price, Level, std::greater<Price>> &footprint){
        _open = open;
        _high = high;
        _low = low;
        _close = close;
        _time_stamp = time;
        _footprint = std::move(footprint);
        _profile = std::shared_ptr<Profile>(new Profile);
        _contains_fp = true;
    }
    
    //@return opening time of the candle
    time_t timestamp() const { return _time_stamp;}
    
    //@return open of the candle
    Price open() const {return _open;}
    
    //@return close of the candle
    Price close() const {return _close;}
    
    //@return high of the candle
    Price high() const {return _high;}
    
    //@return low of the candle
    Price low() const {return _low;}
    
    /*@return price with the highest volume i.e Commitment Of Traders*/
    Price cot(){
        if (!_contains_fp) return -1;
        if (!_set_profile) {
            _profile->set_fp(_footprint);
            _set_profile = true;
        }
        return _profile->cot();
    }
    
    //@return Price with the highest ask volume
    Price ask_cot(){
        if (!_contains_fp) return -1;
        if (!_set_profile) {
            _profile->set_fp(_footprint);
            _set_profile = true;
        }
        return _profile->ask_cot();
    }
    
    //@return Price with the highest bid volume
    Price bid_cot(){
        if (!_contains_fp) return -1;
        if (!_set_profile) {
            _profile->set_fp(_footprint);
            _set_profile = true;
        }
        return _profile->bid_cot();
    }
    
    /*@return volume weighted price of the candlestick*/
    Price vwap(){
        if (!_contains_fp) return -1;
        if (!_set_profile) {
            _profile->set_fp(_footprint);
            _set_profile = true;
        }
        return _profile->vwap();
    }
    
    /*@return Value area high of the candlestick*/
    Price vah(){
        if (!_contains_fp) return -1;
        if (!_set_profile) {
            _profile->set_fp(_footprint);
            _set_profile = true;
        }
        return _profile->vah();
    }
    
    /*@return value area low of the candlestick*/
    Price val(){
        if (!_contains_fp) return -1;
        if (!_set_profile) {
            _profile->set_fp(_footprint);
            _set_profile = true;
        }
        return _profile->val();
    }
    
    //@return total ask volume
    Quantity ask_vol(){
        if (!_contains_fp) return -1;
        if (!_set_profile) {
            _profile->set_fp(_footprint);
            _set_profile = true;
        }
        return _profile->ask_vol();
    }
    
    //@return total bids volume
    Quantity bid_vol(){
        if (!_contains_fp) return -1;
        if (!_set_profile) {
            _profile->set_fp(_footprint);
            _set_profile = true;
        }
        return _profile->bid_vol();
    }
    
    //@return delta of the candle
    Quantity delta(){ 
        if (!_contains_fp) return -1;
        return bid_vol()- ask_vol(); 
    }
    
    //@return maximum delta in the candle
    Quantity max_delta(){
        if (!_contains_fp) return -1;
        if (!_set_profile) {
            _profile->set_fp(_footprint);
            _set_profile = true;
        }
        return _profile->max_delta(); 
    }
    
    //@return minimum delta in the candle
    Quantity min_delta(){ 
        if (!_contains_fp) return -1;
        if (!_set_profile) {
            _profile->set_fp(_footprint);
            _set_profile = true;
        }
        return _profile->min_delta();
    }
    
    //@return total volume traded
    Quantity volume(){
        if (!_contains_fp) return -1;
        if (!_set_profile) {
            _profile->set_fp(_footprint);
            _set_profile = true;
        }
        return bid_vol()+ask_vol();
    }
    
    /*@return Read/write map containing the footprint*/
    std::map<Price, Level, std::greater<Price>> &footprint(){
        return _footprint;
    }

    bool contains_footprint(){return _contains_fp;}
    
    /*Recalculates the value area using the percentage given.
    @param percentage percentage of the value area
    @note percentage should be in ratio e.g 0.7 instead of 70%*/
    void set_va(double percentage){
        if (!_contains_fp) return ;
        _profile->set_fp(_footprint, percentage);
        _set_profile = true;
    }
    
    /*Prints the footprint of the candle stick. @note colors indicates imabalance. Green = buy imbalance, Red = sell imbalance*/
    void print_fp() {
        for (auto &x : _footprint){
            if (x.first == cot())
                std::cout << "\033[33m"; //color code
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
    
    friend std::istream &operator>>(std::istream &in, CandleStick &obj){
        int level_size = 0;
        in >> obj._open >> obj._high >> obj._low >> obj._close >> obj._time_stamp >> level_size;

        if (in.eof()) return in;
        Level temp;
        for (int i = 0; i < level_size; i++){
            in >> temp;
            obj._footprint[temp.price] = temp;            
        }
        if (level_size > 0){
            obj._contains_fp = true;
            obj._profile = std::shared_ptr<Profile>(new Profile);
        }
        return in;
    }

    
private:
    Price _open, _high, _low, _close;
    time_t _time_stamp;
    std::map<Price, Level, std::greater<Price>> _footprint;
    std::shared_ptr<Profile> _profile;
    bool _set_profile = false, _contains_fp = false;
};
