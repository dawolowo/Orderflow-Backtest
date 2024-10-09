/*
 This is file contains code necessary to read time and sales data specifically from binance. 
*/
#ifndef DATA_H
#define DATA_H

#include "defs.h"

namespace data{
    std::fstream file_in;
    std::streampos last_position;

    void open_file(const char *path){
        file_in.open(path, std::ios::in);
        if (!file_in) throw std::logic_error("cause = data::open_file() : File was not opened\n");
    }

    void close_file(){ 
        file_in.close();
    }
    /*seprates a csv line into the column components and stores it in row parameter(vector)
    @tparam line string containing csv line
    @tparam ncols number of columns to read 
    @tparam row initially empty vector that will contain data as values
    */
    void separate(std::string &line, const size_t ncols, std::vector<std::string> &row){
        int n = 0;
        std::string temp = "";
        for (int i = 0; i < line.size(); i++){
            char &x = line[i];
            if (x == ',' ){
                row.push_back(temp);
                temp = "";
                n++;
            }
            else temp += x;
            if (i == line.size()-1) row.push_back(temp);
            if (n == ncols) break;
        }
    }
    /* separates the csv line into columns and stores it in row parameter (map)
    @tparam line string containing a csv line
    @tparam cols vector containing the column names
    @tparam row initially empty unordered map that will contain the columns as keys and data as values
    */
    void separate(std::string &line, const std::vector<const char *> &cols, std::unordered_map<const char *, std::string> &row){
        int n = 0;
        std::string temp = "";
        for (int i = 0; i < line.size(); i++){
            char &x = line[i];
            if (x == ',' ){
                row[cols[n]] = temp;
                temp = "";
                n++;
            }
            else temp += x;
            if (i == line.size()-1) row[cols[n]] = temp;
            if (n == cols.size()) break;
        }
    }
    /*streams a csv file and fills the row parameter with the content. Reads a single row.
    @tparam ncol number of columns to read
    @tparam row vector that will be filled with the read row of the csv file
    */
    void stream_file(const size_t &ncol, std::vector<std::string> &row){
        std::string line;
        file_in >> line;
        separate(line, ncol, row);
    }
    /*streams a csv file and fills the row parameter with the content. Reads a single line.
    @tparam cols vector containing the column names
    @tparam row unordered map that will be filled with the read row of the csv file. Its key contains the column names 
    and value contains data
    */
    void stream_file(const std::vector<const char *> &cols, std::unordered_map<const char *, std::string> &row){
        std::string line;
        file_in >> line;
        separate(line, cols, row);
    }
}

#endif