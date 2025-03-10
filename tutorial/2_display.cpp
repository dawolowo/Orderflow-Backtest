#include "header/chart.hpp"
/*
Tutorial: How to display the footprint, delta, bar of the candle
*/

using namespace std;

int main(){
    const char *file_path = ".txt"; //Location of the aggregated time and sales

    Chart chart;
    chart.load(file_path);
    cout << "Number of candles: " << chart.size() << '\n'
    << "Enter your desired index from 0 to " << chart.size()-1 << ". Enter -1 to exit program\n";

    int response = 0;
    
    while (response != -1){
        cout << "index: ";
        cin >> response;
        if (response >= 0 && response < chart.size())
            chart[response].print_fp();
            // chart[response].print_bar();
            // chart[response].print_delta();
    }

    return 0;
}
