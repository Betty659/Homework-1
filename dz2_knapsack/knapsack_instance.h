#pragma once

#include "item.h"
#include <istream>
#include <vector>

class KnapsackInstance {
public:
    int item_count = 0;
    long long capacity = 0;
    std::vector<Item> items;

    bool ReadFromStream(std::istream& in);
};
