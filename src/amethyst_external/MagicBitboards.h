//
// Created by Noah Holbrook on 12/26/23.
//

#ifndef AMETHYST_CHESS_MAGICBITBOARDS_H
#define AMETHYST_CHESS_MAGICBITBOARDS_H
#include <vector>
#include "AttackedSquares.h"
#include "BitLogarithm.h"
#include "MagicNumbers.h"
#include "Flags.h"
using namespace std;

vector<unsigned long> getKingAttackedSquaresTable () {
    vector<unsigned long> kingAttackedSquaresTable(64);
    for (int square = 0; square < 64; square++) {
        kingAttackedSquaresTable[square] = getKingAttackedSquares(square);
    }
    return kingAttackedSquaresTable;
}

vector<unsigned long> getKnightAttackedSquaresTable () {
    vector<unsigned long> knightAttackedSquaresTable(64);
    for (int square = 0; square < 64; square++) {
        knightAttackedSquaresTable[square] = getKnightAttackedSquares(square);
    }
    return knightAttackedSquaresTable;
}

vector<unsigned long> getAllSubBitsOf (unsigned long original) {
    // This function has been bug tested and is correct
    // Having good variable names for this function is almost impossible because literator just has so many things that mean different things.
    // Step 1: Find the 1 bits
    int bitCount = 0;
    vector<int> oneBits;
    for (int i = 0; i < 64; i++) {
        if (((original >> i) & 1ULL) == 1ULL) {
            bitCount++;
            oneBits.push_back(i);
        }
    }
    unsigned long remainingBits = original;
    int index = 0;
    while (remainingBits != 0ULL) {
        int smallestOneBit = LOG_2_TABLE.get(remainingBits & -remainingBits);
        oneBits[index] = smallestOneBit;
        index++;
        remainingBits -= 1L << smallestOneBit;
    }

    // Step 2: Construct all possibilities
    vector<unsigned long> possibilities(1 << bitCount);
    for (index = 0; index < possibilities.size(); index++) {
        // For every 1 bit in index, OR the thing from oneBits
        unsigned long subBits = 0ULL;
        for (int bit = 0; bit < bitCount; bit++) {
            if (((index >> bit) & 1) == 1)
                subBits |= 1L << oneBits[bit];
        } // end inner for loop
        possibilities[index] = subBits;
    } // end outer for loop
    return possibilities;
} // end getAllSubBitsOf

int bitCount (const unsigned long number) {
    int bitCount = 0;
    for (int i = 0; i < 64; i++) {
        if (((number >> i) & 1ULL) == 1ULL) {
            bitCount++;
        }
    }
    return bitCount;
}

vector<unsigned long> getRookMagicBitboardTable(int startSquare, unsigned long magicNumber, int numBits) {
    //unsigned long numBits = bitCount(getRookPotentialBlockers(startSquare));
    vector<unsigned long> magicBitboardTable(1 << numBits);

    for (unsigned long blockersMask : getAllSubBitsOf(getRookPotentialBlockers(startSquare))) {
        unsigned long result = getRookLegalMoves(startSquare,blockersMask);
        unsigned long bucket = (blockersMask * magicNumber) >> (64 - numBits);
        assert(magicBitboardTable[bucket] == 0ULL or magicBitboardTable[bucket] == result);
        magicBitboardTable[bucket] = result;
    }

    return magicBitboardTable;
}

vector<unsigned long> getBishopMagicBitboardTable(int startSquare, unsigned long magicNumber, int numBits) {
    vector<unsigned long> magicBitboardTable(1 << numBits);

    for (unsigned long blockersMask : getAllSubBitsOf(getBishopPotentialBlockers(startSquare))) {
        unsigned long result = getBishopLegalMoves(startSquare,blockersMask);
        unsigned long bucket = (blockersMask * magicNumber) >> (64 - numBits);
        assert(magicBitboardTable[bucket] == 0ULL or magicBitboardTable[bucket] == result);
        magicBitboardTable[bucket] = result;
    }

    return magicBitboardTable;
}

// A 2D vector is not the fastest way to do this, but I will optimize literator later.
vector<vector<unsigned long>> getRookMagicBitboardTable() {
    vector<vector<unsigned long>> table(64);
    for (int square = 0; square < 64; square++) {
        int bits = rook_rellevant_bits[square];
        unsigned long magic = rook_magics[square];
        table[square] = getRookMagicBitboardTable(square,magic,bits);
    }
    return table;
}

vector<vector<unsigned long>> getBishopMagicBitboardTable() {
    vector<vector<unsigned long>> table(64);
    for (int square = 0; square < 64; square++) {
        int bits = bishop_rellevant_bits[square];
        unsigned long magic = bishop_magics[square];
        table[square] = getBishopMagicBitboardTable(square,magic,bits);
    }
    return table;
}

const unsigned long ROOK_RELEVANT_BLOCKERS[64] = {282578800148862,
                                                        565157600297596,
                                                        1130315200595066,
                                                        2260630401190006,
                                                        4521260802379886,
                                                        9042521604759646,
                                                        18085043209519166,
                                                        36170086419038334,
                                                        282578800180736,
                                                        565157600328704,
                                                        1130315200625152,
                                                        2260630401218048,
                                                        4521260802403840,
                                                        9042521604775424,
                                                        18085043209518592,
                                                        36170086419037696,
                                                        282578808340736,
                                                        565157608292864,
                                                        1130315208328192,
                                                        2260630408398848,
                                                        4521260808540160,
                                                        9042521608822784,
                                                        18085043209388032,
                                                        36170086418907136,
                                                        282580897300736,
                                                        565159647117824,
                                                        1130317180306432,
                                                        2260632246683648,
                                                        4521262379438080,
                                                        9042522644946944,
                                                        18085043175964672,
                                                        36170086385483776,
                                                        283115671060736,
                                                        565681586307584,
                                                        1130822006735872,
                                                        2261102847592448,
                                                        4521664529305600,
                                                        9042787892731904,
                                                        18085034619584512,
                                                        36170077829103616,
                                                        420017753620736,
                                                        699298018886144,
                                                        1260057572672512,
                                                        2381576680245248,
                                                        4624614895390720,
                                                        9110691325681664,
                                                        18082844186263552,
                                                        36167887395782656,
                                                        35466950888980736,
                                                        34905104758997504,
                                                        34344362452452352,
                                                        33222877839362048,
                                                        30979908613181440,
                                                        26493970160820224,
                                                        17522093256097792,
                                                        35607136465616896,
                                                        9079539427579068672,
                                                        8935706818303361536,
                                                        8792156787827803136,
                                                        8505056726876686336,
                                                        7930856604974452736,
                                                        6782456361169985536,
                                                        4485655873561051136,
                                                        9115426935197958144};

const unsigned long BISHOP_RELEVANT_BLOCKERS[64] = {18049651735527936,
70506452091904,
275415828992,
1075975168,
38021120,
8657588224,
2216338399232,
567382630219776,
9024825867763712,
18049651735527424,
70506452221952,
275449643008,
9733406720,
2216342585344,
567382630203392,
1134765260406784,
4512412933816832,
9024825867633664,
18049651768822272,
70515108615168,
2491752130560,
567383701868544,
1134765256220672,
2269530512441344,
2256206450263040,
4512412900526080,
9024834391117824,
18051867805491712,
637888545440768,
1135039602493440,
2269529440784384,
4539058881568768,
1128098963916800,
2256197927833600,
4514594912477184,
9592139778506752,
19184279556981248,
2339762086609920,
4538784537380864,
9077569074761728,
562958610993152,
1125917221986304,
2814792987328512,
5629586008178688,
11259172008099840,
22518341868716544,
9007336962655232,
18014673925310464,
2216338399232,
4432676798464,
11064376819712,
22137335185408,
44272556441600,
87995357200384,
35253226045952,
70506452091904,
567382630219776,
1134765260406784,
2832480465846272,
5667157807464448,
11333774449049600,
22526811443298304,
9024825867763712,
18049651735527936};

const vector<vector<unsigned long>> ROOK_MAGIC_BITBOARD_TABLE = getRookMagicBitboardTable();
const vector<vector<unsigned long>> BISHOP_MAGIC_BITBOARD_TABLE = getBishopMagicBitboardTable();
const vector<unsigned long> KING_ATTACKED_SQUARES_TABLE = getKingAttackedSquaresTable();
const vector<unsigned long> KNIGHT_ATTACKED_SQUARES_TABLE = getKnightAttackedSquaresTable();


// TODONE: Inline these functions at the very end of the project.
// NOTE: We still need to remove our own pieces from the attackedSquares for all of these. Otherwise we would end up taking our own pieces.

inline unsigned long getMagicKingAttackedSquares (const int startingSquare) {
    return KING_ATTACKED_SQUARES_TABLE[startingSquare];
}

inline unsigned long getMagicKnightAttackedSquares(const int startingSquare) {
    return KNIGHT_ATTACKED_SQUARES_TABLE[startingSquare];
}

inline unsigned long getMagicBishopAttackedSquares (const int startingSquare, unsigned long allPieces) {
    allPieces &= BISHOP_RELEVANT_BLOCKERS[startingSquare];
    return BISHOP_MAGIC_BITBOARD_TABLE[startingSquare][(allPieces * bishop_magics[startingSquare]) >> bishop_shifts[startingSquare]];
}

inline unsigned long getMagicRookAttackedSquares (const int startingSquare, unsigned long allPieces) {
    allPieces &= ROOK_RELEVANT_BLOCKERS[startingSquare];
    return ROOK_MAGIC_BITBOARD_TABLE[startingSquare][(allPieces * rook_magics[startingSquare]) >> rook_shifts[startingSquare]];
}

inline unsigned long getMagicQueenAttackedSquares (const int startingSquare, const unsigned long allPieces) {
    return getMagicRookAttackedSquares(startingSquare, allPieces) | getMagicBishopAttackedSquares(startingSquare,allPieces);
}

inline unsigned long calculateWhitePawnAttackedSquares(const int startingSquare) {
    return ((512ULL << startingSquare) & NOT_A_FILE) | (((1ULL << (startingSquare)) >> 7) & NOT_H_FILE);
}

inline unsigned long calculateBlackPawnAttackedSquares(const int startingSquare) {
    return ((128ULL << startingSquare) & NOT_A_FILE) | (((1ULL << (startingSquare)) >> 9) & NOT_H_FILE);
}

inline unsigned long getMagicWhiteAttackedSquares (const int pieceCode, const int startingSquare, const unsigned long allPieces) {
    switch(pieceCode) {
        case QUEEN_CODE: return getMagicQueenAttackedSquares(startingSquare,allPieces);
        case ROOK_CODE: return getMagicRookAttackedSquares(startingSquare,allPieces);
        case BISHOP_CODE: return getMagicBishopAttackedSquares(startingSquare,allPieces);
        case KNIGHT_CODE: return getMagicKnightAttackedSquares(startingSquare);
        case PAWN_CODE: return calculateWhitePawnAttackedSquares(startingSquare);
        default: assert(false);
    }
}

inline unsigned long getMagicBlackAttackedSquares (const int pieceCode, const int startingSquare, const unsigned long allPieces) {
    switch(pieceCode) {
        case QUEEN_CODE: return getMagicQueenAttackedSquares(startingSquare,allPieces);
        case ROOK_CODE: return getMagicRookAttackedSquares(startingSquare,allPieces);
        case BISHOP_CODE: return getMagicBishopAttackedSquares(startingSquare,allPieces);
        case KNIGHT_CODE: return getMagicKnightAttackedSquares(startingSquare);
        case PAWN_CODE: return calculateBlackPawnAttackedSquares(startingSquare);
        default: assert(false);
    }
}

#endif //AMETHYST_CHESS_MAGICBITBOARDS_H
