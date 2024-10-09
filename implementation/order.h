/*
This file contains code that describes an Order and an executed order i.e Trade
Direction = an enum indicating direction of trade
Trade = a structure that contains elements of a real life executed order
*/

#ifndef ORDER_H
#define ORDER_H

#include "defs.h"

/*Indicates the direction of a trade/ order*/
enum class Direction{
    buy,
    sell
};

enum OrderType {
    mo, // mo = market order. Fills at the close which is the market price.
    limit
};

/*Contain elements of a typical trade i.e an executed order*/
struct Trade{
    Price entry; // entry price
    Price sl; // stop loss
    Price tp; // take profit
    time_t time_stamp;
    Direction direction;
    bool success;
    bool trade_completed = false;
    double rr = 0; // reward-to-risk
};
/*Contains elements of an order.

Order is unexecuted. Not to be confused with Trade, it becomes a Trade when executed.*/
struct Order{
    Price entry;
    Price sl; // stop loss
    Price tp; // take profit
    Direction direction;
    OrderType order_type;
    // the number of candlestick it should wait for before cancelling. Only necessary of order_type = limit
    int cancel_after; 
    int counter = 0; // to keep track of number of candlestick since it was added. @note Should be left as it is.
    bool filled = false;
    bool cancelled = false;
};

#endif
