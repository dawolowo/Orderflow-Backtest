# About
This is a collection of tools I developed for myself and I decided to share to fellow algorithmic traders. This project has two functions:

1. Aggregate time and sales data from a data source i.e Allows the use of orderflow algorithmically. IT WAS NOT DEVELOPED TO VISUALIZE FOOTPRINT CHARTS BUT TO BE USED IN A TRADING BOT. 
2. Backtest trading strategies. It can backtest Orderflow based strategies, indicator based strategies, price action based strategies. Unlike some backtesting engines, this engine can only backtest strategies with a predefined stop loss and take profit.

## Requirement
c++20

## Structure
Details of files in ```header```:
* `safequeue.hpp`: contains implementation of a queue that is thread safe.
* `backtest.hpp`: contains the backtest engine.
* `candlestick.hpp`: contains the `CandleStick` class. `CandleStick` is a structural representation of a realife candlestick.
* `chart.hpp`: contains the `Chart` class. `Chart` is a collection of `CandleStick` with additional functions like adding indicators.
* `data.hpp`: defines function to read a file.
* `level_info.hpp`: contains a struct that stores information on a price level.
* `aggregator.hpp`: defines function to aggregrate time and sales data.
* `market_profile.hpp`: contains `Profile` class which is used volume analysis. e.g value area, vwap, point of control etc.
* `order.hpp`: contains `Order` and `Trade` struct used in `backtest.hpp`.

## Tutorial
### How to aggregate data
```
#include "header/aggregator.hpp"

using namespace std;

int main(){
    const char *file_path = ".csv", *store_path = ".txt";
    const int time_interval = 15*60 // 15 minutes;
    const Price price_interval = 3;
    int skip = 0;

    size_t line = aggregator::aggregate_store(file_path, handler::binance_handler, store_path, price_interval, time_interval, skip);

    return 0;
}
```
In code above, `file_path = location of the file you want to aggregate`,

`store_path = where you want to store the aggregated file`

`time_interval = time frame of the each candle e.g 5m, 15m etc`,

`price_interval = difference between each footprint level`,

`skip = number of lines to skip before it starts aggregating. Why? Well, sometimes the first few rows of time and sales data contains other information e.g column names`

`handler::binance_handler = function that parses binance data`

Note that to aggregate data from other source other than binance the handler needs to change. You can write your handler by looking at the implementation of `handler::binance_handler`. Data handler for other sources would be added as time goes on

### How to load aggregated data
The aggregated that would be stored in a `Chart` class. It could also be stored in a `vector<CandleStick>` but there is no advantage in that. But if for some reason you need it in `vector<CandleStick>` form you can call `Chart::candles()`.

Code to load aggregated data:
```
#include "header/chart.hpp"

using namespace std;

int main(){
  const char *file_path = ".txt";

  Chart chart;
  chart.load(file_path);

  return 0;
}
```
In the code above,
`file_path = location where the aggregated data is`

### What next?
Here are some things you can do

To select a candlestick you can use `operator[int]`. The code below assumes you have loaded the data in chart
```
cout << chart[0].close(); // Print the close of the first candlestick

cout << chart[0].cot(); // Prints the most traded price i.e commitment of traders/ point of control

cout << chart[0].delta(); //Prints the delta of the candlestick

chart[0].print_fp(); //Prints the footprint of the candlestick

cout << chart[0].vah(); // Prints the value area high using a value area of 70% (default)

// To change the value area value, do the following
chart[0].set_va(0.8); // sets value area to 80%
```
You can also do "chart things" like add an indicator
```
chart.apply_sma(14, Source::open); //Using a built-in sma indicator. 14 day moving average appied to the open of candlestick

chart.custom_indicator("myindicator", data); // Applying a custom indicator. data is a vector containing the values of the indicator. Read documentation for more info

chart.select_indicator("myindicator"); //Select an indicator from the chart. You can have multiple indicator in a chart, all which have a corresponding name
```
### NOTE:
bids = aggressive buyers/ passive sellers while asks = aggressive sellers/ passive buyers. Some orderflow software and books do the opposite (i.e bids = aggressive sellers/ passive buyers; asks = aggressive buyers/ passive sellers).
