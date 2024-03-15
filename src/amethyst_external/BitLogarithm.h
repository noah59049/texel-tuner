//
// Created by Noah Holbrook on 12/17/23.
//

#ifndef AMETHYST_CHESS_BITLOGARITHM_H
#define AMETHYST_CHESS_BITLOGARITHM_H
#include <cstdlib>

class Log2Table {
private:
    int* table;
public:
    Log2Table() {
        table = new int[67];
        for (int i = 0; i < 64; i++) {
            table[(1ULL << i) % 67] = i;
        }
    }
    ~Log2Table() {
        delete table;
    }
    inline int get(const unsigned long value) const {
        return table[value % 67];
    }
};

const static Log2Table LOG_2_TABLE;

#endif //AMETHYST_CHESS_BITLOGARITHM_H
