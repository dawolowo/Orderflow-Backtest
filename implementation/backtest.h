/*
This file is contains necessary code to successfully backtest a strategy

BackTest = a class that contains 'properties' to simulate live market and test a strategy
*/

#ifndef BACKTEST_H
#define BACKTEST_H
#include "defs.h"
#include "candlestick.h"
#include "chart.h"
#include "order.h"
/*Indicates the direction of a trade*/

/*An object that backtest a strategy on a given data
@tparam candles vector containing the candlesticks to be backtested
@tparam chart chart containing the candlesticks to be backtested
@tparam strategy function containing strategy to be backtested
@note Limit order trades might not be backtested correctly due to not using tick data. Nevertheless, result won't vary much from
live testing unless very few trades were taken.
*/
class BackTest{
public:
    Quantity equity = 10'000;
    double risk = 1; //risk per trade in percentage. @note Should not be negative

    BackTest(std::vector<CandleStick> &candles, void (*strategy) (BackTest &)){
        _candles = candles;
        _strategy = strategy;
    }
    BackTest(Chart &chart, void (*strategy) (BackTest &)){
        _chart = chart;
        _strategy = strategy;
        _candles = _chart.candles();
    }
    /*Runs the backtest on the strategy*/
    void run(){
        _reset();
        for (; _index < _candles.size(); ++_index){
            _manage_trades();
            _strategy(*this);
            _manage_orders();
            _update_dd();
        }
        _run_analysis();
    }
    // @return index
    size_t index(){return _index;}
    //@return candles in backtest engine
    std::vector<CandleStick> &candles() {return _candles;}
    //@return chart
    Chart &chart() {return _chart;}

    /*Adds an order to the backtest engine*/
    void add_order(Order &order){
        _orders.push_back(order);
    }
    /*Adds an order to the backtest engine*/
    void add_order(Order &&order){
        _orders.push_back(order);
    }

    /*@brief Prints statistical information about the strategy backtested to the console.*/
    void print_stat(){
        std::cout << std::setprecision(4) << std::fixed;
        std::cout << "winrate : " << ((double) (_short_wins+_long_wins))/_n_trades << "\tnumber of trades : " << _n_trades
        << "\nmax loss in a row : " << _max_loss_in_a_row  << "\tmax win in a row : " << _max_win_in_a_row 
        <<"\nmax drawdown : " << _max_dd  << "\tmax drawdown (duration) : " << _max_dd_duration
        << "\nlongs : " << _longs << "\tshorts : " << _shorts 
        << "\nlongs winrate : " << (_longs > 0? ((double) _long_wins)/ _longs : 0) << "\tshorts winrate : " 
        << (_shorts > 0 ? ((double) _short_wins)/ _shorts : 0) 
        << "\nsignal rate : " << (_candles.size() > 0 ? ((double)_n_trades)/ _candles.size() : 0) << "\treturns : " 
        << (equity-_initial_equity)/_initial_equity << "\n";
        std::cout.unsetf(std::ios::scientific | std::ios::fixed);
    }
    /*Prints the time, direction and the trades success to the console*/
    void print_trades(){
        std::tm ti;
        time_t temp;
        for (auto &tr : _trades){
            temp = tr.time_stamp/1000;
            localtime_s(&ti, &temp);
            std::cout << ti.tm_year+1900 << "/" << ti.tm_mon+1 << "/" << ti.tm_mday << " " << ti.tm_hour << ":" << ti.tm_min << "\t" 
            << (tr.direction == Direction::buy? "buy" : "sell") << "\t" << (tr.success? "successful" : "not successful") << std::endl;
        }        
    }
    /*Print the time, direction, trades success, entry, stop loss, take profit to the console*/
    void debug(){
        std::tm ti;
        time_t temp;
        for (auto &tr : _trades){
            temp = tr.time_stamp/1000;
            localtime_s(&ti, &temp);
            std::cout << ti.tm_year+1900 << "/" << ti.tm_mon+1 << "/" << ti.tm_mday << " " << ti.tm_hour << ":" << ti.tm_min << "\t" 
            << (tr.direction == Direction::buy? "buy" : "sell") << "\t" << (tr.success? "successful" : "not successful") 
            << "\tent : " << tr.entry << "\tsl : " << tr.sl << "\ttp : " << tr.tp << std::endl;
        }
    }


private:
    std::vector<CandleStick> _candles;
    Chart _chart;
    void (*_strategy) (BackTest &);
    size_t _index = 0;
    std::vector<Trade> _trades;
    std::vector<Order> _orders;
    size_t _long_wins = 0, _short_wins = 0, _longs = 0, _shorts = 0, _n_trades = 0;
    size_t _max_loss_in_a_row = 0, _max_win_in_a_row = 0;
    /* total reward to risk ratio, negative rr means not profitable. you can multiply it by your risk per trade in dollars to get
     the profit/loss over the backtest.
    */
    double _rr = 0, _max_dd = 0; 
    Quantity _max_equity = equity, _initial_equity = equity;
    /*Longest duration of a drawdown.@note It is unrelated to max drawdown*/
    long long _max_dd_duration = 0, _dd_duration = 0;
    
    /*Calculates useful information about the backtest*/
    void _run_analysis(){
        size_t consecutive_loss = 0, consecutive_win = 0;
        for (auto &tr : _trades){
            if (tr.trade_completed){
                if (tr.success){
                    if (tr.direction == Direction::sell) _short_wins++;
                    else if (tr.direction == Direction::buy) _long_wins++;
                    consecutive_loss = 0;
                    consecutive_win++;
                }
                else {
                    consecutive_win = 0;
                    consecutive_loss++;
                }
                if (tr.direction == Direction::sell) ++_shorts;
                else _longs++;
                
                if (consecutive_loss > _max_loss_in_a_row) _max_loss_in_a_row = consecutive_loss;
                if (consecutive_win > _max_win_in_a_row) _max_win_in_a_row = consecutive_win;
                _rr += tr.rr;
                ++_n_trades;
            }
        }
    }
    /*Manage trades. Responsible for checking if trades is successful or not*/
    void _manage_trades(){
        for (Trade &tr : _trades){
            if (!tr.trade_completed){
                if (_candles[_index].low() < tr.sl && tr.direction == Direction::buy){
                    tr.trade_completed = true;
                    tr.success = false;
                    tr.rr = -1;
                }
                else if (_candles[_index].high() > tr.sl && tr.direction == Direction::sell){
                    tr.trade_completed = true;
                    tr.success = false;
                    tr.rr = -1;
                }
                else if (_candles[_index].low() < tr.tp && tr.direction == Direction::sell){
                    tr.trade_completed = true;
                    tr.success = true;
                    tr.rr = (tr.entry-tr.tp)/ (tr.sl-tr.entry);
                }
                else if (_candles[_index].high() > tr.tp && tr.direction == Direction::buy){
                    tr.trade_completed = true;
                    tr.success = true;
                    tr.rr = (tr.tp-tr.entry)/ (tr.entry-tr.sl);
                }
                if (tr.trade_completed) _update_balance(tr);
            }
        }
    }
    /*Adds a trade to the backtest engine*/
    void _add_trade(Trade &&trade){ _trades.push_back(trade);}

    void _fill(Order &od){
        _add_trade(Trade(od.entry, od.sl, od.tp, _candles[_index].time_stamp(), od.direction));
        od.filled = true;
    }
    /*Manage orders. Responsible for cancelling and filling orders*/
    void _manage_orders(){
        for (Order &od : _orders){
            if (!od.filled && !od.cancelled){
                if (od.order_type == OrderType::mo){
                    od.entry = _candles[_index].close();
                    _fill(od);
                }
                else if (od.counter > 0 && _candles[_index].high() > od.entry && od.entry > _candles[_index].low()){
                    _fill(od);
                }
                else if (od.counter > od.cancel_after) od.cancelled = true;
            }
            od.counter++;
        }
    }
    //Update equity 
    void _update_balance(Trade &tr){
        double reward = tr.rr * risk;
        equity += equity*reward/100;
    }
    //Update drawdowns
    void _update_dd(){
        if (equity >= _max_equity){
            _max_equity = equity;
            _dd_duration = 0;
        }
        else {
            double dd = (equity-_max_equity)/equity;
            if (++_dd_duration > _max_dd_duration)_max_dd_duration = _dd_duration;
            if (dd < _max_dd) _max_dd = dd;
        }
    }
    /* Resets all private variables.
    @note Does not reset public variables
    */
    void _reset(){
        _index = 0;
        _trades = {};
        _orders = {};
        _long_wins = 0, _short_wins = 0, _longs = 0, _shorts = 0, _n_trades = 0;
        _max_loss_in_a_row = 0, _max_win_in_a_row = 0;
        _rr = 0, _max_dd = 0; 
        _max_equity = _initial_equity = equity;
    }
};

#endif