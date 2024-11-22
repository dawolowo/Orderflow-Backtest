# Orderflow-Backtest
This is project is for algorithmic traders with knowledge of c++ who wants to develop and backtest orderflow strategies with binance time and sales data. It is also for people who wants to learn.

By default it works with binance time and sales data gotten from: https://www.binance.com/en/landing/data but it can tweaked to work with data from other sources (check loading_file.cpp in usage folder).

### NOTE:
Keep in mind is that in the footprints, bids = aggressive buyers/ passive sellers while asks = aggressive sellers/ passive buyers. I've seen some orderflow software and books do the opposite (i.e bids = aggressive sellers/ passive buyers; asks = aggressive buyers/ passive sellers).
