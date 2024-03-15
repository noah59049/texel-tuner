//
// Created by Noah Holbrook on 12/16/23.
//

#ifndef AMETHYST_CHESS_FLAGS_H
#define AMETHYST_CHESS_FLAGS_H
#include <cstdint>
constexpr const static uint16_t NORMAL_MOVE_FLAG = 0;
constexpr const static uint16_t PROMOTE_TO_QUEEN_FLAG = 1;
constexpr const static uint16_t PROMOTE_TO_ROOK_FLAG = 2;
constexpr const static uint16_t PROMOTE_TO_BISHOP_FLAG = 3;
constexpr const static uint16_t PROMOTE_TO_KNIGHT_FLAG = 4;
constexpr const static uint16_t EN_PASSANT_FLAG = 5;
constexpr const static uint16_t PAWN_PUSH_TWO_SQUARES_FLAG = 6; // Not used before ChessBoard3
constexpr const static uint16_t CAPTURE_QUEEN_FLAG = 7;
constexpr const static uint16_t CAPTURE_ROOK_FLAG = 8;
constexpr const static uint16_t CAPTURE_BISHOP_FLAG = 9;
constexpr const static uint16_t CAPTURE_KNIGHT_FLAG = 10;
constexpr const static uint16_t CAPTURE_PAWN_FLAG = 11;

constexpr const static uint16_t WHITE_SHORT_CASTLE = 49920;
constexpr const static uint16_t WHITE_LONG_CASTLE = 16640;
constexpr const static uint16_t BLACK_SHORT_CASTLE = 57200;
constexpr const static uint16_t BLACK_LONG_CASTLE = 23920;

constexpr const static uint16_t SEARCH_FAILED_MOVE_CODE = 9360;

const static uint16_t PROMOTION_FLAGS[4] = {PROMOTE_TO_QUEEN_FLAG,PROMOTE_TO_ROOK_FLAG,PROMOTE_TO_BISHOP_FLAG,PROMOTE_TO_KNIGHT_FLAG};

inline bool isCastle(const uint16_t move) {
    return move == WHITE_SHORT_CASTLE or move == BLACK_SHORT_CASTLE or move == WHITE_LONG_CASTLE or move == BLACK_LONG_CASTLE;
}

inline bool isPromotion(const uint16_t move) {
    return (move & 15) <= PROMOTE_TO_KNIGHT_FLAG and PROMOTE_TO_QUEEN_FLAG <= (move & 15);
}

inline bool isCapture(const uint16_t move) {
    // TODO: Include en passant captures in this
    return CAPTURE_QUEEN_FLAG <= (move & 15) and (move & 15) <= CAPTURE_PAWN_FLAG;
}

inline bool isCapturePromotion(const uint16_t move) {
    return isPromotion(move) and move >> 13 == ((move >> 7) & 7);
}

inline uint16_t getFlag(const uint16_t move) {
    return move & 15;
}

inline int getStartSquare(const uint16_t move) {
    return move >> 10;
}

inline int getEndSquare(const uint16_t move) {
    return move >> 4 & 63;
}
enum Piece {
    KING,
    QUEEN,
    ROOK,
    BISHOP,
    KNIGHT,
    PAWN,
};

// TODO: Should these be 8-bit ints?
constexpr const static int QUEEN_CODE = 0;
constexpr const static int ROOK_CODE = 1;
constexpr const static int BISHOP_CODE = 2;
constexpr const static int KNIGHT_CODE = 3;
constexpr const static int PAWN_CODE = 4;
#endif //AMETHYST_CHESS_FLAGS_H
