//
// Created by Noah Holbrook on 12/28/23.
//

#ifndef AMETHYST_CHESS_CHECKBLOCKERS_H
#define AMETHYST_CHESS_CHECKBLOCKERS_H

#include <cstdint>
#include "MagicBitboards.h"


unsigned long calculateRookBlockerSquaresManual (const int kingSquare, int rookSquare) {
    int LEFT = -8;
    int RIGHT = 8;
    int UP = 1;
    int DOWN = -1;
    if (((getMagicRookAttackedSquares(rookSquare,0ULL) >> kingSquare) & 1ULL) == 0ULL)
        return ENTIRE_BOARD;
    int dsquare;
    if (kingSquare % 8 == rookSquare % 8) {
        if (rookSquare > kingSquare)
            dsquare = LEFT;
        else
            dsquare = RIGHT;
    }
    else if (kingSquare / 8 == rookSquare / 8) {
        if (rookSquare > kingSquare)
            dsquare = DOWN;
        else
            dsquare = UP;
    }
    else {
        assert(false);
    }

    unsigned long blockerSquares = 0ULL;
    do {
        blockerSquares |= 1ULL << rookSquare;
        rookSquare += dsquare;
    } while (rookSquare != kingSquare);
    return blockerSquares;
}

unsigned long calculateBishopBlockerSquaresManual (const int kingSquare, int bishopSquare) {
    if (((getMagicBishopAttackedSquares(bishopSquare,0ULL) >> kingSquare) & 1ULL) == 0ULL)
        return ENTIRE_BOARD;

    int UPRIGHT = 9;
    int DOWNRIGHT = 7;
    int UPLEFT = -7;
    int DOWNLEFT = -9;

    int kingX = kingSquare / 8;
    int kingY = kingSquare % 8;
    int bishopX = bishopSquare / 8;
    int bishopY = bishopSquare % 8;

    int directionToKing;
    if (bishopX < kingX) {
        if (bishopY < kingY)
            directionToKing = UPRIGHT;
        else
            directionToKing = DOWNRIGHT;
    }
    else {
        if (bishopY < kingY)
            directionToKing = UPLEFT;
        else
            directionToKing = DOWNLEFT;
    }

    unsigned long blockerSquares = 0ULL;
    do {
        blockerSquares |= 1ULL << bishopSquare;
        bishopSquare += directionToKing;
    } while (bishopSquare != kingSquare);
    return blockerSquares;
} // end getBishopBlockerSquares

vector<unsigned long> getBishopBlockerButterfly () {
    vector<unsigned long> butterflyBoard(4096);
    for (int kingSquare = 0; kingSquare < 64; kingSquare++) {
        for (int bishopSquare = 0; bishopSquare < 64; bishopSquare++) {
            butterflyBoard[kingSquare * 64 + bishopSquare] = calculateBishopBlockerSquaresManual(kingSquare,bishopSquare);
        }
    }
    return butterflyBoard;
}

vector<unsigned long> getRookBlockerButterfly () {
    vector<unsigned long> butterflyBoard(4096);
    for (int kingSquare = 0; kingSquare < 64; kingSquare++) {
        for (int rookSquare = 0; rookSquare < 64; rookSquare++) {
            butterflyBoard[kingSquare * 64 + rookSquare] = calculateRookBlockerSquaresManual(kingSquare,rookSquare);
        }
    }
    return butterflyBoard;
}

const static vector<unsigned long> BISHOP_BLOCKER_BUTTERFLY = getBishopBlockerButterfly();
const static vector<unsigned long> ROOK_BLOCKER_BUTTERFLY = getRookBlockerButterfly();

// TODONE: Inline these once the project is more finalized
inline unsigned long lookupBishopCheckResponses(int kingSquare, int bishopSquare) {
    return BISHOP_BLOCKER_BUTTERFLY[kingSquare * 64 + bishopSquare];
}
inline unsigned long lookupRookCheckResponses(int kingSquare, int rookSquare) {
    return ROOK_BLOCKER_BUTTERFLY[kingSquare * 64 + rookSquare];
}

unsigned long getManualCheckResponses(int kingSquare, int pieceSquare) {
    unsigned long queenCheckBlockSquares = lookupBishopCheckResponses(kingSquare,pieceSquare) & lookupRookCheckResponses(kingSquare,pieceSquare);
    return queenCheckBlockSquares == ENTIRE_BOARD ? 1ULL << pieceSquare : queenCheckBlockSquares;
}

struct CheckResponsesButterfly {
    unsigned long butterfly[4096];

    CheckResponsesButterfly () {
        for (int kingSquare = 0; kingSquare < 64; kingSquare++) {
            for (int pieceSquare = 0; pieceSquare < 64; pieceSquare++) {
                butterfly[kingSquare * 64 + pieceSquare] = getManualCheckResponses(kingSquare,pieceSquare);
            }
        }
    }
};

const static CheckResponsesButterfly CHECK_RESPONSES_BUTTERFLY = CheckResponsesButterfly();

inline unsigned long lookupCheckResponses(int kingSquare, int pieceSquare) {
    return CHECK_RESPONSES_BUTTERFLY.butterfly[kingSquare * 64 + pieceSquare];
}
#endif //AMETHYST_CHESS_CHECKBLOCKERS_H
