#include "levels_agg.hpp"
#include "chart.hpp"
#include <filesystem>

using namespace std;

int main(){
    const char *file_path = ".csv";
    const char *store_path = ".txt";
    const int time_interval = 15;
    const Price price_interval = 8;
    size_t cols = 6;
    double file_size = filesystem::file_size(file_path)/(1024.0*1024);
    time_t start = time(0);
    size_t line = levels::agg_store_thread(file_path, cols, store_path, price_interval, time_interval, true);
    size_t duration = time(0)-start;
    cout << "speed : " << file_size/duration << " MB/s";
    return 0;
}