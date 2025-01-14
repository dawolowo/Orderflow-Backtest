/*
This file contains code that describes an Order and an executed order i.e Trade
Direction = an enum indicating direction of trade
Trade = a structure that contains elements of a real life executed order
*/

#pragma once

#include "defs.hpp"

/*Indicates the direction of a trade/ order*/
enum class Direction {
    buy,
    sell
};

enum OrderType {
    mo, // mo = market order. Fills at the close which is the market price.
    limit
};

/*Contain elements of a typical trade i.e an executed order
@param entry entry price
@param sl stop loss
@param tp take profit
@param timestamp time of entry
@param direction direction of order, buy/sell
@param comment additional information on order. For easy debugging
*/
struct Trade {
    Price entry; // entry price
    Price sl; // stop loss
    Price tp; // take profit
    time_t timestamp;
    Direction direction;
    /*Additional information about the trade for easy debugging e.g values of variables when the trade was taken. @note It would be the
     same as Order::comment
    */
    std::string comment; 
    bool success = false;
    bool trade_completed = false;
    float rr = 0; // reward-to-risk
};

/*Contains elements of an order.

Order is unexecuted. Not to be confused with Trade, it becomes a Trade when executed.
@param entry entry price
@param sl stop loss
@param tp take profit
@param direction direction of order, buy/sell
@param order_type type of order, market order/ limit order
@param cancel_after maximum number of candles before cancelling if order is not triggered (for limit ordertype)
@param comment additional information on order. For easy debugging
*/
struct Order{

    Order(Price _entry, Price _sl, Price _tp, Direction _direction, OrderType _order_type, size_t _cancel_after = SIZE_MAX,
     std::string _comment = ""){
        entry = _entry, sl = _sl, _tp = tp;
        direction = _direction;
        order_type = _order_type;
        cancel_after = _cancel_after;
        comment = std::move(_comment);
    }
    Price entry;
    Price sl; // stop loss
    Price tp; // take profit
    Direction direction;
    OrderType order_type;
    // the number of candlestick it should wait for before cancelling. Only necessary for order_type = limit
    size_t cancel_after = SIZE_MAX; 
    std::string comment; //Additional information about the order for easy debugging e.g values of variables when the order was placed
    // current index when the order was placed. @note Should be left has it is
    size_t entry_id = 0;
    
    friend int operator<=>(const Order &l, const Order &r){
        if (l.entry < r.entry) return -1;
        else if (l.entry > r.entry) return 1;
        else return 0;
    }
    
};
