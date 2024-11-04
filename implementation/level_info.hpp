/*
This file contains code that describes the market level.

*/ 
#ifndef LEVEL_INFO_HPP
#define LEVEL_INFO_HPP
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
        return Level(price, bids+operand.bids, asks+operand.asks);
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

#endif
