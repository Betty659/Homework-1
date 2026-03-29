#include "knapsack_instance.h"

bool KnapsackInstance::ReadFromStream(std::istream& in) {
    if (!(in >> item_count >> capacity)) {
        return false;
    }

    items.clear();
    items.reserve(item_count);

    for (int i = 0; i < item_count; ++i) {
        long long value = 0;
        long long weight = 0;
        if (!(in >> value >> weight)) {
            return false;
        }
        items.push_back({i, value, weight});
    }

    return true;
}
