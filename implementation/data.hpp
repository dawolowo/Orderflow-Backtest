/*
 This is file contains code necessary to read time and sales data specifically from binance. 
*/
#pragma once

#include "defs.hpp"
#include "atomicqueue.hpp"
#include <chrono>
#include <thread>

namespace data{
    /*Class inheriting from std::fstream and incorporating RAII.
    */
    class FileStream : public std::fstream{
    public:
        FileStream() = default;

        ~FileStream(){close();}

        /*
        Opens an external file, Raises an exception if not opened.

        @param __s The name of the file.
        @param __mode The open mode flags.

        Calls std::basic_filebuf::open(__s,__mode). If that function fails, failbit is set in the stream's error state.
        */
        void open_except(const char *__s, std::ios_base::openmode __mode) {
            open(__s, __mode);
            if (!is_open()) throw std::logic_error("cause = File::open_except() : No such file\n");
        }
    };
       
    //Limits the ram usage. The size is in bytes
    const size_t MAX_RAM_USE = 1000*1024*1024;

    /*streams a csv file and fills the row parameter with the content. Reads a single row.
    @param ncol number of columns to read
    @param row vector that will be filled with the read row of the csv file. Empty vector should be passed
    */
    inline void stream_file(const size_t &ncol, std::vector<std::string> &row, FileStream &file_in){
        char x;
        size_t n = 0;
        row[0] = "";
        while (file_in.get(x)){
            if (x == '\n') {
                break;
            }
            if (x == ',' && n+1 < ncol){
                row[++n] = "";
            }
            else if (x == ',')n++;
            else if (n < ncol) row[n] += x;
        }

    }
    
    void thread_stream(AtomicQueue<std::string> &buffer, FileStream &file){
        std::string line;
        char _;
        while (!file.eof()){
            std::getline(file, line);
            buffer.push(line);
            file.get(_);
            if (line.capacity()*buffer.size() > MAX_RAM_USE) std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }    
}

