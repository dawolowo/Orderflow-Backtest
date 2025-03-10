/*
This file contains code that describes the market level.

*/ 
#pragma once
#include "defs.hpp"

/*A structure that contains information about the bid and ask volumes traded at a price*/
struct Level{
    Price price;
    Quantity bids = 0;
    Quantity asks = 0;

    // Checks for buy imbalance
    bool buy_imbalance(double ratio) const {
        return bids > ratio * asks;
    }
    
    // Checks for sell imbalance
    bool sell_imbalance(double ratio) const{
        return asks > ratio * bids;
    }
    
    Level operator+(const Level &operand){
        if (price != operand.price) throw std::logic_error("operator+ error : Cannot add levels with different price\n");
        Level ret;
        ret.price = price;
        ret.bids = bids+operand.bids;
        ret.asks = asks+operand.asks;
        return ret;
    }
    
    friend std::ostream &operator<<(std::ostream &out, Level &obj){
        out << obj.price << " " << obj.bids << " " << obj.asks;
        return out;
    }
    
    friend std::istream &operator>>(std::istream &in, Level &obj){
        in >> obj.price >> obj.bids >> obj.asks;
        return in;
    }

};

