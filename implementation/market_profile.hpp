#pragma once
#include "defs.hpp"
#include "level_info.hpp"
#include <limits>
#include <iostream>

/*
An object representing a market profile. Contains api's for getting common information relating to market profile e.g vwap, cot/poc
@param x footprint of a candle or aggregated footprint of a range of candles
@param va_percent percentage used to calculate value area. Defaults to 0.7 (70%)
*/
class Profile {
    Price _cot, _acot, _bcot, _vah, _val, _vwap;
    Quantity _max_vol = 0, _max_bid = 0, _max_ask = 0, _ask_vol = 0, _bid_vol = 0;
    Quantity _max_delta = std::numeric_limits<Quantity>::lowest(), _min_delta = std::numeric_limits<Quantity>::max();
    double _percent = 0.7;

    /*Calculates and sets information such as max_delta, bid volume, vwap etc*/
    void _set_info(const std::map<Price, Level, std::greater<Price>> &footprint){
        if (footprint.empty())return;
        double pv = 0; // pv = price * volume

        for (auto it : footprint){
            _setter(it.second);
            pv += it.first * (it.second.asks+it.second.bids);
        }
        _vwap = (volume() > 0) ? pv/volume() : 0;
        _value_area(footprint, _percent);
    }
    
    /*helper function for _set_info()*/
    void _setter(const Level &temp){
        Quantity del, vol;
        del = temp.bids - temp.asks; //calculating delta
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
    }

    /*Performs the computation to get the value area high and value area low
    @param footprint footprint
    @param percentage Percentage of volume within the value area
    */
    void _value_area(const std::map<Price, Level, std::greater<Price>> &footprint, double percentage){
        if (percentage > 1.0) percentage = 1.0;
        auto it = footprint.find(_cot);
        Quantity vol = it->second.asks + it->second.bids;
        const Quantity total_vol = volume();
        std::map<Price, Level, std::greater<Price>>::const_iterator up;
        std::map<Price, Level, std::greater<Price>>::const_iterator down;
        if (it == footprint.begin()){ 
            up = it;
            down = std::next(it);
        }
        else if (std::next(it) == footprint.end()){
            down = it;
            up = std::prev(it);
        }
        else {
            up = std::prev(it);
            down = std::next(it);
        }
        bool reached_top = (up == it);
        bool reached_bottom = (down == it);
        while (vol < percentage*total_vol){
            Quantity down_vol = 0, up_vol = 0;
            
            if (!reached_bottom) down_vol = down->second.asks + down->second.bids;
            if (!reached_top) up_vol = up->second.bids + up->second.asks;

            if (down_vol > up_vol){
                vol += down_vol;
                if (std::next(down) != footprint.end()) down = std::next(down);
                else reached_bottom = true;
            }
            else if (up_vol > down_vol){
                vol += up_vol;
                if (up != footprint.begin()) up = std::prev(up);
                else reached_top = true;
            }
            else {
                vol += up_vol + down_vol;
                if (std::next(down) != footprint.end()) down = std::next(down);
                else reached_bottom = true;
                if (up != footprint.begin()) up = std::prev(up);
                else reached_top = true;
            } 
        }
        if (!reached_top) up = std::next(up);
        if (!reached_bottom) down = std::prev(down);
        _vah = up->first;
        _val = down->first;
    }

public:
    Profile() = default;

    Profile(std::map<Price, Level, std::greater<Price>> &x, double va_percent = 0.7){
        _percent = va_percent;
        _set_info(x);
    }

    /*@return price with the highest volume i.e Commitment Of Traders*/
    Price cot() const {
        return _cot;
    }

    //@return Price with the highest ask volume
    Price ask_cot() const {
        return _acot;
    }

    //@return Price with the highest bid volume
    Price bid_cot() const {
        return _bcot;
    }

    Price vah() const {
        return _vah;
    }

    Price val() const {
        return _val;
    }

    Price vwap() const {
        return _vwap;
    }

    Quantity ask_vol() const {
        return _ask_vol;
    }

    Quantity bid_vol() const {
        return _bid_vol;
    }

    Quantity volume() const {
        return _ask_vol+_bid_vol;
    }

    Quantity min_delta() const {
        return _min_delta;
    }

    Quantity max_delta() const {
        return _max_delta;
    }
    
    /*Computes the volume analysis on a give Profile/ footprint
    @param x footprint to be analyzed
    @param va_percent Percentage to calculated the value area. Defaults to 0.7 (70%)
    */
    void set_fp(const std::map<Price, Level, std::greater<Price>> &x, double va_percent = 0.7){
        _percent = va_percent;
        _set_info(x);
    }
};
