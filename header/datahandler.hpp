#pragma once
#include "rowdata.hpp"
#include "data.hpp"

/*A data handler is a function that reads a single line of csv parses the information and returns the information in a RowData format*/

namespace handler{
    inline RowData binance_handler(data::FileStream &file){
        size_t n = 0;
        static std::string line, col_content;
        static RowData res;

        std::getline(file, line);
        for (const char &x : line){
            if (x == ','){
                /*Take a look at binance time and sales data and this block of code would make sense. 
                This is essentially what varies with diffeent exchanges*/

                if (n == 1)res.price = stof(col_content);
                else if (n == 2)res.volume = stof(col_content);
                else if (n == 4)res.timestamp = stoll(col_content); //In milliseconds
                else if (n == 5)res.buyer_is_taker = col_content[0] == 'F' || col_content[0] == 'f';
                n++;
                col_content = "";
            }
            else col_content += x;
        }
        col_content = ""; // Reset the col_content before appending to it, since it is static
        return res;
    }
}
