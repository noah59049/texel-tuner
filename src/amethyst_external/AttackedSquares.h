//
// Created by Noah Holbrook on 12/16/23.
//

#ifndef AMETHYST_CHESS_ATTACKEDSQUARES_H
#define AMETHYST_CHESS_ATTACKEDSQUARES_H
#include "RankFileBitmasks.h"

unsigned long getKingAttackedSquares (const int square) {
    const int rank = square % 8;
    const int file = square / 8;

    unsigned long attackedSquares = 0ULL;

    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 and dy == 0)
                continue;
            int newRank = rank + dy;
            int newFile = file + dx;
            if (0 <= newRank and newRank <= 7 && 0 <= newFile and newFile <= 7)
                attackedSquares |= 1ULL << (8 * newFile + newRank);
        }
    }
    return attackedSquares;
}

unsigned long getKnightAttackedSquares (const int square) {
    const unsigned long startSquare = 1ULL << square;
    const int rank = square % 8;
    const int file = square / 8;

    unsigned long attackedSquares = 0ULL;

    // Check forwards moves (from white's perspective)
    if (rank < 7) {
        // Check moves that are 2 sideways and 1 forwards
        if (file > 1) // we can go Nc2-a3
            attackedSquares |= startSquare >> 15;
        if (file < 6) // We can go Nf2-h3
            attackedSquares |= startSquare << 17;

        if (rank < 6) {
            // Check moves that are 2 forwards and 1 sideways
            if (file > 0)
                attackedSquares |= startSquare >> 6;
            if (file < 7)
                attackedSquares |= startSquare << 10;
        }
    }

    if (rank > 0) {
        // Check moves that are 2 sideways and 1 backwards
        if (file > 1) // We can go Nc2-a1
            attackedSquares |= startSquare >> 17;
        if (file < 6)
            attackedSquares |= startSquare << 15;

        if (rank > 1) {
            // check moves that are 2 backwards and 1 sideways
            if (file > 0) // we can go Nb3-a1
                attackedSquares |= startSquare >> 10;
            if (file < 7) // we can go Ng3-h1
                attackedSquares |= startSquare << 6;
        }
    }

    return attackedSquares;
}

unsigned long getRookAttackedSquares (const int square) {
    const int rank = square % 8;
    const int file = square / 8;

    return (A_FILE << (file * 8) | FIRST_RANK << rank) - (1ULL << square);
}

unsigned long getBishopAttackedSquares (const int square) {
    const int rank = square % 8;
    const int file = square / 8;

    unsigned long attackedSquares = 0ULL;

    for (int dx : {-1,1}) {
        for (int dy : {-1,1}) {
            int x = file + dx;
            int y = rank + dy;
            while (0 <= x and x <= 7 && 0 <= y and y <= 7) {
                attackedSquares |= 1ULL << (8 * x + y);
                x += dx;
                y += dy;
            }
        }
    }

    return attackedSquares;
}

unsigned long getQueenAttackedSquares (const int square) {
    return getRookAttackedSquares(square) | getBishopAttackedSquares(square);
}

unsigned long getRookPotentialBlockers (int square) {
    const int rank = square % 8;
    const int file = square / 8;

    const unsigned long fileBlockers = A_FILE << (file * 8) & INNER_RANKS;
    const unsigned long rankBlockers = (FIRST_RANK << rank) & INNER_FILES;

    return (fileBlockers | rankBlockers) & ~(1ULL << square);
}

unsigned long getBishopPotentialBlockers (int square) {
    return getBishopAttackedSquares(square) & INNER_36;
}

unsigned long getBishopLegalMoves (int square, unsigned long blockers) {
    //assert(((blockers & getBishopPotentialBlockers(square)) == blockers));
    blockers &= getBishopPotentialBlockers(square);
    const int rank = square % 8;
    const int file = square / 8;

    unsigned long attackedSquares = 0ULL;

    for (int dx : {-1,1}) {
        for (int dy : {-1,1}) {
            int x = file + dx;
            int y = rank + dy;
            while (0 <= x and x <= 7 && 0 <= y and y <= 7) {
                attackedSquares |= 1ULL << (8 * x + y);
                if (blockers & (1ULL << (8 * x + y)))
                    break;
                x += dx;
                y += dy;
            }
        }
    }

    return attackedSquares;
}

unsigned long getRookLegalMoves (int square, unsigned long blockers) {
    //assert(((blockers & getRookPotentialBlockers(square)) == blockers));
    blockers &= getRookPotentialBlockers(square);
    const int rank = square % 8;
    const int file = square / 8;

    unsigned long attackedSquares = 0ULL;

    int DXS[4] = {0,0,-1,1};
    int DYS[4] = {1,-1,0,0};

    for (int i = 0; i < 4; i++) {
        int dx = DXS[i];
        int dy = DYS[i];
        int x = file + dx;
        int y = rank + dy;
        while (0 <= x and x <= 7 && 0 <= y and y <= 7) {
            attackedSquares |= 1ULL << (8 * x + y);
            if (blockers & (1ULL << (8 * x + y)))
                break;
            x += dx;
            y += dy;
        }
    }

    return attackedSquares;
}

unsigned long getQueenLegalMoves (int square, unsigned long blockers) {
    return getRookLegalMoves(square, blockers) | getBishopLegalMoves(square,blockers);
}

#endif //AMETHYST_CHESS_ATTACKEDSQUARES_H
