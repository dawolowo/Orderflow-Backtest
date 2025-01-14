# About
This is project is for algorithmic traders with knowledge of c++ who wants to backtest trading strategies. It is capable of backtesting price action, indicator and orderflow based strategies. It can also be used to aggregrate time and sales data (SEE `levels_agg.hpp` file)

By default it works with binance time and sales data gotten from: https://data.binance.vision but it can tweaked to work with data from other sources (check `loading_file.cpp` in `usage` folder to see how).

## Structure
Details of files in ```implementation```:
* `atomicqueue.hpp`: contains implementation of a queue that is thread safe.
* `backtest.hpp`: contains the backtest engine.
* `candlestick.hpp`: contains the `CandleStick` class. `CandleStick` is a structural representation of a realife candlestick.
* `chart.hpp`: contains the `Chart` class. `Chart` is a collection of `CandleStick` with additional functions.
* `data.hpp`: defines function to read a file.
* `level_info.hpp`: contains a struct and stores information on the price level.
* `levels_agg.hpp`: defines function to aggregrate time and sales data.
* `market_profile.hpp`: contains `profile` class which is used volume analysis. e.g value area, vwap, point of control etc.
* `order.hpp`: contains `Order` and `Trade` struct used in `backtest.hpp`.

### NOTE:
bids = aggressive buyers/ passive sellers while asks = aggressive sellers/ passive buyers. Some orderflow software and books do the opposite (i.e bids = aggressive sellers/ passive buyers; asks = aggressive buyers/ passive sellers).
