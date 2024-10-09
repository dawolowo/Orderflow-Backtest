/*
This file contains code that describes the market level.

*/ 
#ifndef LEVEL_INFO_H
#define LEVEL_INFO_H
#include "defs.h"

/*A structure that contains information about the bid and ask volumes traded at a price*/
struct Level{
    Price price_;
    Quantity bids_ = 0;
    Quantity asks_ = 0;
    // Checks for buy imbalance
    bool buy_imbalance(double ratio) const {
        return bids_ > ratio * asks_;
    }
    // Checks for sell imbalance
    bool sell_imbalance(double ratio) const{
        return asks_ > ratio * bids_;
    }
    
    Level operator+(const Level &operand){
        if (price_ != operand.price_) throw std::logic_error("operator+ error : Cannot add levels with different price\n");
        return Level(price_, bids_+operand.bids_, asks_+operand.asks_);
    }
    
    friend std::ostream &operator<<(std::ostream &out, Level &obj){
        out << obj.price_ << " " << obj.bids_ << " " << obj.asks_;
        return out;
    }
    
    friend std::istream &operator>>(std::istream &in, Level &obj){
        in >> obj.price_ >> obj.bids_ >> obj.asks_;
        return in;
    }

};

#endif