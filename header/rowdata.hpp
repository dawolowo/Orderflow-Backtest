#pragma once
#include "defs.hpp"

struct RowData{
    time_t timestamp; // Unix time format. Should be in milliseconds
    Price price;
    bool buyer_is_taker;
    Quantity volume;
};