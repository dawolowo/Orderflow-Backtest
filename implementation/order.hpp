/*
This file contains code that describes an Order and an executed order i.e Trade
Direction = an enum indicating direction of trade
Trade = a structure that contains elements of a real life executed order
*/

#ifndef ORDER_HPP
#define ORDER_HPP

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
@param time_stamp time of entry
@param direction direction of order, buy/sell
@param comment additional information on order. For easy debugging
*/
struct Trade {
    Price entry; // entry price
    Price sl; // stop loss
    Price tp; // take profit
    time_t time_stamp;
    Direction direction;
    /*Additional information about the trade for easy debugging e.g values of variables when the trade was taken. @note It would be the
     same as Order::comment
    */
    std::string_view comment; 
    bool success = false;
    bool trade_completed = false;
    double rr = 0; // reward-to-risk
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
    Price entry;
    Price sl; // stop loss
    Price tp; // take profit
    Direction direction;
    OrderType order_type;
    // the number of candlestick it should wait for before cancelling. Only necessary of order_type = limit
    size_t cancel_after = SIZE_MAX; 
    std::string comment; //Additional information about the order for easy debugging e.g values of variables when the order was placed
    size_t counter = 0; // to keep track of number of candlestick since it was added. @note Should be left as it is.
    bool filled = false;
    bool cancelled = false;
    
};

#endif
