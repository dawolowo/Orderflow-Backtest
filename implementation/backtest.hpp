/*
This file is contains necessary code to successfully backtest a strategy

BackTest = a class that contains 'properties' to simulate live market and test a strategy
*/

#pragma once
#include "defs.hpp"
#include "candlestick.hpp"
#include "chart.hpp"
#include "order.hpp"
#include <queue>
#include <utility>
#include <chrono>

/*An object that backtest a strategy on a given data
@param candles vector containing the candlesticks to be backtested
@param chart chart containing the candlesticks to be backtested
@param strategy function containing strategy to be backtested
@note Limit order trades might not be backtested correctly due to not using tick data. Nevertheless, result won't vary much from
live testing.
*/
class BackTest{
public:
    float risk = 0.01; //Risk per trade. It is not in percentage i.e 1% should be 0.01. @note Should not be negative

    BackTest(Chart &chart, void (*strategy) (BackTest &), const char *strat_name = "") : _candles(_chart.candles()){
        _chart = chart;
        _strategy = strategy;
        _strategy_name = strat_name;
    }

    /*Runs the backtest on the strategy*/
    void run(){
        auto start = std::chrono::high_resolution_clock::now();
        _reset();
        for (; _index < _candles.size(); ++_index){
            _manage_trades();
            _manage_orders();
            _strategy(*this);
            _update_dd();
        }
        _run_analysis();
        _time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-start);
    }
    
    // @return Index of the current candle during backtest
    size_t index() const {return _index;}
    
    //@return candles in backtest engine
    std::vector<CandleStick> &candles() {return _candles;}
    
    //@return chart
    Chart &chart() {return _chart;}
    
    //@return Returns of the strategy @note Not in percentage
    float returns() const {return _returns;}
   
    //@return The accuracy of the strategy @note Not in percentage
    float winrate() const {return ((float) (_short_wins+_long_wins))/_n_trades;}
    
    //@return The maximum drawdown @note Not in percentage
    float max_dd() const {return _max_dd;}
    
    //@return A const reference to the trades taken
    const std::vector<Trade> &trades() const {
        return _trades;
    }

    /*Adds an order to the backtest engine*/
    void add_order(Order &order){
        if (_check(order)){
            order.entry_id = _index;
            if (order.order_type == OrderType::mo){
                order.entry = _candles[_index].close();
                _fill(order);
            }
            else if (order.direction == Direction::buy) _buy_limit.push(order);
            else _sell_limit.push(order);
        }
    }
    
    /*Adds an order to the backtest engine*/
    void add_order(Order &&order){
        add_order(order);
    }
    
    /*@brief Prints statistical information about the strategy backtested to the console.@note Max drawdown (duration) is not the duration of 
    the maximum drawdown, it is the maximum time spent in a drawdown (It may or may not be the maximum drawdown).
    
    */
    void print_stat(){
        std::ios cout_state(nullptr);
        cout_state.copyfmt(std::cout); // To reset the console later
        std::cout << std::setprecision(4);
        std::cout << "strategy name: " << _strategy_name << '\n'
        << "winrate : " << ((_n_trades > 0 ? (float) (_short_wins+_long_wins)/_n_trades : 0) *100)<< "%\tnumber of trades : " 
        << _n_trades << "\nmax loss in a row : " << _max_loss_in_a_row  << "\tmax win in a row : " << _max_win_in_a_row 
        <<"\nmax drawdown : " << _max_dd*100  << "%\tmax drawdown (duration) : " << _max_dd_duration << " candles"
        << "\nlongs : " << _longs << "\t\tshorts : " << _shorts 
        << "\nlongs winrate : " << ((_longs > 0? ((float) _long_wins)/ _longs : 0)*100) << "%\tshorts winrate : " 
        << (_shorts > 0 ? ((float) _short_wins)/ _shorts : 0) *100
        << "%\nsignal rate : " << (_candles.size() > 0 ? ((float)_n_trades)/ _candles.size() : 0)*100 << "%\treturns : " 
        << _returns*100 << "%\n" << "time taken : " << _time_taken.count() << " ms\tnumber of candles : " << _candles.size() << "\n";
        std::cout.copyfmt(cout_state);
    }
    
    /*Prints the time, direction and the trades success to the console*/
    void print_trades(){
        std::tm ti;
        time_t temp;
        for (auto &tr : _trades){
            temp = tr.timestamp/1000;
            localtime_s(&ti, &temp);
            std::cout << ti.tm_year+1900 << "/" << ti.tm_mon+1 << "/" << ti.tm_mday << " " << ti.tm_hour << ":" << ti.tm_min << "\t" 
            << (tr.direction == Direction::buy? "buy" : "sell") << "\t" << (tr.success? "successful" : "not successful") << "\n";
        }        
    }
    
    /*Print the time, direction, trades success, entry, stop loss, take profit and comment to the console*/
    void debug(){
        std::tm ti;
        time_t temp;
        for (auto &tr : _trades){
            temp = tr.timestamp/1000;
            localtime_s(&ti, &temp);
            std::cout << ti.tm_year+1900 << "/" << ti.tm_mon+1 << "/" << ti.tm_mday << " " << ti.tm_hour << ":" << ti.tm_min << "\t" 
            << (tr.direction == Direction::buy? "buy" : "sell") << "\t" << (tr.success? "successful" : "not successful") 
            << "\tent : " << tr.entry << "\tsl : " << tr.sl << "\ttp : " << tr.tp << "\t" << tr.comment << "\n";
        }
    }

private:
    std::vector<CandleStick> &_candles;
    Chart _chart;
    void (*_strategy) (BackTest &);
    size_t _index = 0;
    std::vector<Trade> _trades;
    std::priority_queue<Order, std::vector<Order>> _buy_limit; //Descending
    std::priority_queue<Order, std::vector<Order>, std::greater<Order>> _sell_limit; //Ascending
    size_t _long_wins = 0, _short_wins = 0, _longs = 0, _shorts = 0, _n_trades = 0;
    size_t _max_loss_in_a_row = 0, _max_win_in_a_row = 0;
    std::chrono::milliseconds _time_taken;
    std::string _strategy_name;
    
    /* total reward to risk ratio, negative rr means not profitable. you can multiply it by your risk per trade in dollars to get
     the profit/loss over the backtest.
    */
    float _rr = 0; 
    float _max_dd = 0; 
    Quantity _equity = 10'000;
    Quantity _max_equity = _equity, _initial_equity = _equity;
    
    /*Longest duration of a drawdown.@note It is unrelated to max drawdown*/
    long long _max_dd_duration = 0, _dd_duration = 0;
    float _returns = 0;
    
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
        _returns = (_equity-_initial_equity)/_initial_equity;
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
    
    // Execute an order
    void _fill(const Order &od){
        _trades.emplace_back(od.entry, od.sl, od.tp, _candles[_index].timestamp(), od.direction, std::move(od.comment));
    }
    
    /*Manage orders. Responsible for cancelling and filling orders*/
    void _manage_orders(){
        while (!_buy_limit.empty()){
            if (_candles[_index].low() <= _buy_limit.top().entry){
                if (_index - _buy_limit.top().entry_id <= _buy_limit.top().cancel_after) _fill(_buy_limit.top());
                _buy_limit.pop();
            }
            else break;
        }
        while (!_sell_limit.empty()){
            if (_candles[_index].high() >= _sell_limit.top().entry){
                if (_index - _sell_limit.top().entry_id <= _sell_limit.top().cancel_after) _fill(_sell_limit.top());
                _sell_limit.pop();
            }
            else break;
        }
    }
    
    //Update _equity 
    void _update_balance(Trade &tr){
        float reward = tr.rr * risk;
        _equity += _equity*reward;
    }
    
    //Update drawdowns
    void _update_dd(){
        if (_equity >= _max_equity){
            _max_equity = _equity;
            _dd_duration = 0;
        }
        else {
            float dd = (_equity-_max_equity)/_equity;
            if (++_dd_duration > _max_dd_duration)_max_dd_duration = _dd_duration;
            if (dd < _max_dd) _max_dd = dd;
        }
    }
    
    //Checks if an order is proper
    bool _check(Order &order){
        if ((order.entry <= order.sl || order.tp <= order.entry) && order.direction == Direction::buy) return false;
        else if ((order.entry >= order.sl || order.tp >= order.entry) && order.direction == Direction::sell) return false;
        else if (order.order_type == OrderType::limit && order.direction == Direction::buy && order.entry > _candles[_index].close()) return false;
        else if (order.order_type == OrderType::limit && order.direction == Direction::sell && order.entry < _candles[_index].close()) return false;
        return true;
    }
    
    /* Resets all private variables.
    @note Does not reset public variables
    */
    void _reset(){
        _index = 0;
        _trades = {};
        _buy_limit = {};
        _sell_limit = {};
        _long_wins = 0, _short_wins = 0, _longs = 0, _shorts = 0, _n_trades = 0;
        _max_loss_in_a_row = 0, _max_win_in_a_row = 0;
        _rr = 0, _max_dd = 0; 
        _equity = 10'000;
        _max_equity = _initial_equity = _equity;
        _returns = 0;
    }
};

