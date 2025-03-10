/*
 This is file contains code necessary to read time and sales data specifically from binance. 
*/
#pragma once

#include "defs.hpp"
#include "safequeue.hpp"
#include "rowdata.hpp"
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
        void open_except(const std::string &__s, std::ios_base::openmode __mode) {
            open(__s, __mode);
            if (!is_open()) throw std::logic_error("cause = File::open_except() : No such file\n");
        }
    };
    
    void thread_stream(SafeQueue<RowData> &buffer, FileStream &file,  RowData (*func) (data::FileStream &)){

        while (!file.eof()){
            RowData &&da = func(file);
            buffer.push(da);
        }
    }    
}

