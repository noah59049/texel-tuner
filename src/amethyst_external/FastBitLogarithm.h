//
// Created by Noah Holbrook on 1/31/24.
//

#ifndef AMETHYST_CHESS_FASTBITLOGARITHM_H
#define AMETHYST_CHESS_FASTBITLOGARITHM_H
namespace FastLogarithm {
    constexpr const static unsigned long magic = 0x07EDD5E59A4E28C2ULL;
    constexpr const static unsigned long lookupTable[64] = {63, 0, 58, 1, 59, 47, 53, 2, 60, 39, 48, 27, 54, 33, 42, 3, 61, 51, 37, 40, 49, 18, 28, 20, 55, 30, 34, 11, 43, 14, 22, 4, 62, 57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19, 29, 10, 13, 21, 56, 45, 25, 31, 35, 16, 9, 12, 44, 24, 15, 8, 23, 7, 6, 5};

    inline int log2(const unsigned long value) {
        return lookupTable[value * magic >> 58];
    }

    inline bool isSingleBit(const unsigned long value) {
        return 1ULL << log2(value) == value;
    }
}
#endif //AMETHYST_CHESS_FASTBITLOGARITHM_H
