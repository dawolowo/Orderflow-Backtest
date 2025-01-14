#include "backtest.hpp"
#include <iostream>
#include "levels_agg.hpp"
#include <cmath>
#include <stack>
#include <format>
using namespace std;

/*
This speed test uses a popular trading strategy called orderblocks. The detection and trading of orderblocks is contained in ob
*/

stack<Swing> swing_highs, swing_lows;
void detect_swing(BackTest &self){
    int i = self.index();
    if (i >= 2){
        const CandleStick &x = self.candles()[i-1], &l = self.candles()[i-2], &r = self.candles()[i];
        if (x.low() < min(l.low(), r.low()) && x.high() < min(l.high(), r.high())){
            swing_lows.push(Swing(i-1, x.low(), Source::low));
        }
        if (x.high() > max(l.high(), r.high()) && x.low() > max(l.low(), r.low())){
            swing_highs.push(Swing(i-1, x.high(), Source::high));
        }
    }
}
/*
ob is function that contains the algorithm to detect an orderblock. The strategy is to set a limit order at the entry of the orderblock after an orderblock is detected, 
it cancels the order after 10 candles have passed.
*/
void ob(BackTest &);

int main(){
    //file_path should point to the location of the aggregated time and sales. SEE loading_file.cpp if you haven't
    const char *file_path = ".txt";
    Chart chart;
    time_t now = time(0);
    chart.load(file_path);
    BackTest btest(chart, ob);
    btest.run();
    btest.print_stat();

    return 0;
}

int bull_id, bear_id;
void ob(BackTest &self){
    detect_swing(self);
    CandleStick &x = self.candles()[self.index()];
    if (x.close() > x.open())bear_id = self.index(); //Index of potential bearish ob
    if (x.close() < x.open()) bull_id = self.index(); // Index of potential bullish ob
    if (!swing_highs.empty()){
        Swing s_high = swing_highs.top();
        if (x.high() > s_high.price) {
            CandleStick &entry = self.candles()[bull_id];
            double risk = entry.high()-entry.low();
            tm time;
            time_t unix = entry.time_stamp()/1000;
            localtime_s(&time, &unix);
            string comment = format("Trigger = {}/{}/{} {}:{}",time.tm_mday, time.tm_mon+1, time.tm_year+1900, time.tm_hour, time.tm_min);
            self.add_order(Order(entry.high(), entry.low(), entry.high()+risk, Direction::buy, OrderType::limit, 10, comment));
        }
        while (x.high() > swing_highs.top().price){
            swing_highs.pop();
            if (swing_highs.empty()) break;
        }
    }
    if (!swing_lows.empty()){
        Swing s_low = swing_lows.top();
        if (x.low() < s_low.price){
            CandleStick &entry = self.candles()[bear_id];
            tm time;
            time_t unix = entry.time_stamp()/1000;
            localtime_s(&time, &unix);
            string comment = format("Trigger = {}/{}/{} {}:{}",time.tm_mday, time.tm_mon+1, time.tm_year+1900, time.tm_hour, time.tm_min);
            double risk = entry.high()-entry.low();
            self.add_order(Order(entry.low(), entry.high(), entry.low()-risk, Direction::sell, OrderType::limit, 10, comment));
        }
        while (x.low() < swing_lows.top().price){
            swing_lows.pop();
            if (swing_lows.empty()) break;
        }
    }
}
