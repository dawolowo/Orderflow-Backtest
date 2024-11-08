/*
 This is file contains code necessary to read time and sales data specifically from binance. 
*/
#ifndef DATA_HPP
#define DATA_HPP

#include "defs.hpp"

namespace data{
    std::fstream file_in;
    std::streampos last_position;

    inline void open_file(const char *path){
        file_in.open(path, std::ios::in);
        if (!file_in) throw std::logic_error("cause = data::open_file() : File was not opened\n");
    }

    inline void close_file(){ 
        file_in.close();
    }

    /*streams a csv file and fills the row parameter with the content. Reads a single row.
    @param ncol number of columns to read
    @param row vector that will be filled with the read row of the csv file. Empty vector should be passed
    */
    inline void stream_file(const size_t ncol, std::vector<std::string> &row){
        char x;
        std::string temp;
        while (file_in.get(x)){
            if (x == '\n') break;
            if (x == ',' && row.size() < ncol){
                row.push_back(temp);
                temp = "";
            }
            else temp += x;
        }
    }
    
    /*streams a csv file and fills the row parameter with the content. Reads a single line.
    @param cols vector containing the column names
    @param row unordered map that will be filled with the read row of the csv file. Its key contains the column names 
    and value contains data
    */
    inline void stream_file(const std::vector<const char *> &cols, std::unordered_map<const char *, std::string> &row){
        size_t n = 0;
        char x;
        while (file_in.get(x)){
            if (x == '\n') break;
            if (x == ',' ) n++;
            else if (n < cols.size()){
                row[cols[n]] += x;
            }
        }
    }
}

#endif
