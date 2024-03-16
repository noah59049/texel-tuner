//
// Created by Noah Holbrook on 3/8/24.
//

#ifndef AMETHYST_CHESS_PARAMETERCHESSBOARD_H
#define AMETHYST_CHESS_PARAMETERCHESSBOARD_H

#include <iostream>
#include <cstdint>
#include <cassert>
#include "ZobristHashing.h"
#include "MagicBitboards.h"
#include "CheckBlockers.h"
#include "FastBitLogarithm.h"
#include "Flags.h"
#include "EmptyMagics.h"
#include "KingSafetyZones.h"
#include "PawnStructure.h"

/**
 * A chess board class designed to be used to store the board state in matches between 2 engines.
 * It uses the same move generation logic as ChessBoard11, but evaluation, SEE, special move generation functions, etc have been removed.
 * This class supports FEN notation and SAN.
 * It is intended as a replacement for SafeChessBoard.
 */

class ParameterChessBoard {
private:

    uint8_t whiteKingPosition;
    uint8_t blackKingPosition;

    unsigned long whitePieceTypes[5] = {1ULL << 24, (1ULL << 0) + (1ULL << 56), (1ULL << 16) + (1ULL << 40),
                                        (1ULL << 8) + (1ULL << 48), 0x0202020202020202ULL};
    // As initialized in default constructor to start up a new game.
    unsigned long allWhitePieces;

    unsigned long blackPieceTypes[5] = {1ULL << 31, (1ULL << 7) + (1ULL << 63), (1ULL << 23) + (1ULL << 47),
                                        (1ULL << 15) + (1ULL << 55), 0x4040404040404040ULL};
    // As initialized in default constructor to start up a new game.
    unsigned long allBlackPieces;

    /**
     * 0 if the a-pawn moved two squares, 4 if the e-pawn moved two squares, 7 if the h-pawn moved two squares, 255 if no pawn moved two squares.
     * This is used for en passant captures.
     */
    uint8_t whichPawnMovedTwoSquares;

    bool isItWhiteToMove;
    bool canWhiteCastleShort;
    bool canWhiteCastleLong;
    bool canBlackCastleShort;
    bool canBlackCastleLong;

    bool drawByInsufficientMaterial;

    unsigned long zobristCode;

    uint8_t pieceGivingCheck;
    constexpr const static uint8_t DOUBLE_CHECK_CODE = 128;
    constexpr const static uint8_t NOT_IN_CHECK_CODE = 255;
    bool drawByStalemate;
    bool whiteWonByCheckmate;
    bool blackWonByCheckmate;

    int halfmoveClock;

    void updatePieceGivingCheck() {
        pieceGivingCheck = calculatePieceGivingCheck();
    }

    void updateMates() {
        if (!areThereLegalMoves()) {
            if (pieceGivingCheck == NOT_IN_CHECK_CODE)
                drawByStalemate = true;
            else {
                if (isItWhiteToMove)
                    blackWonByCheckmate = true;
                else
                    whiteWonByCheckmate = true;
            }
        }
    }

    void manuallyInitializeZobristCode() {
        // Step 0: Initizlize zobrist code to 0
        zobristCode = 0;
        // Step 1: Is it white to move
        if (isItWhiteToMove)
            zobristCode ^= zobrist_hashing::IS_IT_WHITE_TO_MOVE_CODE;
        // Step 2: Castling rights
        if (canWhiteCastleShort)
            zobristCode ^= zobrist_hashing::WHITE_CAN_CASTLE_SHORT_CODE;
        if (canBlackCastleShort)
            zobristCode ^= zobrist_hashing::BLACK_CAN_CASTLE_SHORT_CODE;
        if (canWhiteCastleLong)
            zobristCode ^= zobrist_hashing::WHITE_CAN_CASTLE_LONG_CODE;
        if (canBlackCastleLong)
            zobristCode ^= zobrist_hashing::BLACK_CAN_CASTLE_LONG_CODE;
        // Step 3: En passant rights
        zobristCode ^= zobrist_hashing::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[whichPawnMovedTwoSquares];
        // Step 4: King positions
        zobristCode ^= zobrist_hashing::WHITE_KING_CODES[whiteKingPosition];
        zobristCode ^= zobrist_hashing::BLACK_KING_CODES[blackKingPosition];
        // Step 5: Piece positions
        for (int pieceType = QUEEN_CODE; pieceType <= PAWN_CODE; pieceType++) {
            unsigned long whitePiecesRemaining = whitePieceTypes[pieceType];
            unsigned long thisWhitePieceMask;
            unsigned long thisWhitePieceSquare;
            while (whitePiecesRemaining != 0ULL) {
                thisWhitePieceMask = whitePiecesRemaining & -whitePiecesRemaining;
                whitePiecesRemaining -= thisWhitePieceMask;
                thisWhitePieceSquare = FastLogarithm::log2(thisWhitePieceMask);
                zobristCode ^= zobrist_hashing::WHITE_PIECE_TYPE_CODES[pieceType][thisWhitePieceSquare];
            }

            unsigned long blackPiecesRemaining = blackPieceTypes[pieceType];
            unsigned long thisBlackPieceMask;
            unsigned long thisBlackPieceSquare;
            while (blackPiecesRemaining != 0ULL) {
                thisBlackPieceMask = blackPiecesRemaining & -blackPiecesRemaining;
                blackPiecesRemaining -= thisBlackPieceMask;
                thisBlackPieceSquare = FastLogarithm::log2(thisBlackPieceMask);
                zobristCode ^= zobrist_hashing::BLACK_PIECE_TYPE_CODES[pieceType][thisBlackPieceSquare];
            }
        } // end for loop over piece type
    } // end manuallyInitializeZobristCode

    void makemoveInternal(uint16_t move) {
        if (((allWhitePieces | allBlackPieces) >> getEndSquare(move) & 1ULL) == 1ULL or
            ((whitePieceTypes[PAWN_CODE] | blackPieceTypes[PAWN_CODE]) >> getStartSquare(move) & 1ULL) == 1ULL)
            halfmoveClock = 0; // that move is a capture or pawn move
        else
            halfmoveClock++;

        if (whichPawnMovedTwoSquares < 8) {
            zobristCode ^= zobrist_hashing::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[whichPawnMovedTwoSquares] ^
                           zobrist_hashing::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[255];
            whichPawnMovedTwoSquares = 255;
        }
        uint16_t startSquare = getStartSquare(move);
        uint16_t endSquare = getEndSquare(move);
        uint16_t flag = getFlag(move);
        bool isCapture = false;
        if (isItWhiteToMove) {
            if (((allBlackPieces >> endSquare) & 1ULL) == 1ULL) { // then this move is a capture
                isCapture = true;
                if (blackKingPosition == endSquare) {
                    cout << toString() << endl << "Black king position equals end square; assertion failed" << endl;
                    exit(1);
                }
                if (PROMOTE_TO_QUEEN_FLAG <= flag and flag <= PROMOTE_TO_KNIGHT_FLAG) {
                    for (int capturedPieceType = QUEEN_CODE;
                         capturedPieceType <= KNIGHT_CODE; capturedPieceType++) {
                        if (((blackPieceTypes[capturedPieceType] >> endSquare) & 1ULL) ==
                            1ULL) { // this is the pieceType we are capturing
                            blackPieceTypes[capturedPieceType] -= 1ULL << endSquare;
                            allBlackPieces -= 1ULL << endSquare;
                            zobristCode ^= zobrist_hashing::BLACK_PIECE_TYPE_CODES[capturedPieceType][endSquare];
                            break;
                        }
                    }
                } else {
                    allBlackPieces -= 1ULL << endSquare;
                    blackPieceTypes[flag - CAPTURE_QUEEN_FLAG + QUEEN_CODE] -= 1ULL << endSquare;
                    zobristCode ^= zobrist_hashing::BLACK_PIECE_TYPE_CODES[flag - CAPTURE_QUEEN_FLAG +
                                                                           QUEEN_CODE][endSquare];
                }
            }

            if (move == WHITE_SHORT_CASTLE) {
                whiteKingPosition = 48;
                whitePieceTypes[ROOK_CODE] -= 72056494526300160; // add f1, get rid of h1
                allWhitePieces -= 71775023844556800; // add f1 and g1, get rid of e1 and h1
                canWhiteCastleShort = false;
                if (canWhiteCastleLong) {
                    canWhiteCastleLong = false;
                    zobristCode ^= zobrist_hashing::WHITE_CAN_CASTLE_LONG_CODE;
                }
                isItWhiteToMove = false;

                zobristCode ^= zobrist_hashing::WHITE_PIECE_TYPE_CODES[ROOK_CODE][40] ^
                               zobrist_hashing::WHITE_PIECE_TYPE_CODES[ROOK_CODE][56] ^
                               zobrist_hashing::WHITE_KING_CODES[32] ^ zobrist_hashing::WHITE_KING_CODES[48] ^
                               zobrist_hashing::WHITE_CAN_CASTLE_SHORT_CODE ^
                               zobrist_hashing::IS_IT_WHITE_TO_MOVE_CODE;

                return;
                // We did this because when we're not castling, we update allWhitePieces at the very end, but if we were castling, we would update it in here.
                // So we don't want to update it twice.
            } else if (move == WHITE_LONG_CASTLE) {
                whiteKingPosition = 16;
                whitePieceTypes[ROOK_CODE] += 16777215; // add d1, get rid of a1
                allWhitePieces -= 4278124545; // add c1 and d1, get rid of e1 and a1
                if (canWhiteCastleShort) {
                    canWhiteCastleShort = false;
                    zobristCode ^= zobrist_hashing::WHITE_CAN_CASTLE_SHORT_CODE;
                }
                canWhiteCastleLong = false;
                isItWhiteToMove = false;

                zobristCode ^= zobrist_hashing::WHITE_PIECE_TYPE_CODES[ROOK_CODE][0] ^
                               zobrist_hashing::WHITE_PIECE_TYPE_CODES[ROOK_CODE][24] ^
                               zobrist_hashing::WHITE_KING_CODES[32] ^ zobrist_hashing::WHITE_KING_CODES[16] ^
                               zobrist_hashing::WHITE_CAN_CASTLE_LONG_CODE ^
                               zobrist_hashing::IS_IT_WHITE_TO_MOVE_CODE;

                return;
                // We did this because when we're not castling, we update allWhitePieces at the very end, but if we were castling, we would update it in here.
                // So we don't want to update it twice.
            } else if (flag == EN_PASSANT_FLAG) {
                blackPieceTypes[PAWN_CODE] -= 1ULL << (endSquare - 1);
                allBlackPieces -= 1ULL << (endSquare - 1);
                zobristCode ^= zobrist_hashing::BLACK_PIECE_TYPE_CODES[PAWN_CODE][endSquare - 1];
                whitePieceTypes[PAWN_CODE] |= 1ULL << endSquare;
                whitePieceTypes[PAWN_CODE] -= 1ULL << startSquare;
                zobristCode ^= zobrist_hashing::WHITE_PIECE_TYPE_CODES[PAWN_CODE][startSquare] ^
                               zobrist_hashing::WHITE_PIECE_TYPE_CODES[PAWN_CODE][endSquare];
            } else if (PROMOTE_TO_QUEEN_FLAG <= flag and flag <= PROMOTE_TO_KNIGHT_FLAG) {
                whitePieceTypes[PAWN_CODE] -= 1ULL << startSquare;
                zobristCode ^= zobrist_hashing::WHITE_PIECE_TYPE_CODES[PAWN_CODE][startSquare];
                whitePieceTypes[QUEEN_CODE + flag - PROMOTE_TO_QUEEN_FLAG] |= 1ULL << endSquare;
                zobristCode ^= zobrist_hashing::WHITE_PIECE_TYPE_CODES[QUEEN_CODE + flag -
                                                                       PROMOTE_TO_QUEEN_FLAG][endSquare];
            } else {
                if (flag == PAWN_PUSH_TWO_SQUARES_FLAG) {
                    whitePieceTypes[PAWN_CODE] += 6ULL << (startSquare & 56);
                    zobristCode ^= zobrist_hashing::WHITE_PIECE_TYPE_CODES[PAWN_CODE][startSquare] ^
                                   zobrist_hashing::WHITE_PIECE_TYPE_CODES[PAWN_CODE][endSquare];
                    whichPawnMovedTwoSquares = startSquare >> 3;
                    zobristCode ^= zobrist_hashing::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[255] ^
                                   zobrist_hashing::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[whichPawnMovedTwoSquares];
                } else if (whiteKingPosition == startSquare) {
                    whiteKingPosition = endSquare;
                    zobristCode ^= zobrist_hashing::WHITE_KING_CODES[endSquare] ^
                                   zobrist_hashing::WHITE_KING_CODES[startSquare];
                    if (canWhiteCastleShort) {
                        canWhiteCastleShort = false;
                        zobristCode ^= zobrist_hashing::WHITE_CAN_CASTLE_SHORT_CODE;
                    }
                    if (canWhiteCastleLong) {
                        canWhiteCastleLong = false;
                        zobristCode ^= zobrist_hashing::WHITE_CAN_CASTLE_LONG_CODE;
                    }
                } else if (((whitePieceTypes[QUEEN_CODE] >> startSquare) & 1ULL) == 1ULL) {
                    whitePieceTypes[QUEEN_CODE] |= 1ULL << endSquare;
                    whitePieceTypes[QUEEN_CODE] -= 1ULL << startSquare;
                    zobristCode ^= zobrist_hashing::WHITE_PIECE_TYPE_CODES[QUEEN_CODE][startSquare] ^
                                   zobrist_hashing::WHITE_PIECE_TYPE_CODES[QUEEN_CODE][endSquare];
                } else if (((whitePieceTypes[ROOK_CODE] >> startSquare) & 1ULL) == 1ULL) {
                    whitePieceTypes[ROOK_CODE] |= 1ULL << endSquare;
                    whitePieceTypes[ROOK_CODE] -= 1ULL << startSquare;
                    zobristCode ^= zobrist_hashing::WHITE_PIECE_TYPE_CODES[ROOK_CODE][startSquare] ^
                                   zobrist_hashing::WHITE_PIECE_TYPE_CODES[ROOK_CODE][endSquare];
                } else if (((whitePieceTypes[BISHOP_CODE] >> startSquare) & 1ULL) == 1ULL) {
                    whitePieceTypes[BISHOP_CODE] |= 1ULL << endSquare;
                    whitePieceTypes[BISHOP_CODE] -= 1ULL << startSquare;
                    zobristCode ^= zobrist_hashing::WHITE_PIECE_TYPE_CODES[BISHOP_CODE][startSquare] ^
                                   zobrist_hashing::WHITE_PIECE_TYPE_CODES[BISHOP_CODE][endSquare];
                } else if (((whitePieceTypes[KNIGHT_CODE] >> startSquare) & 1ULL) == 1ULL) {
                    whitePieceTypes[KNIGHT_CODE] |= 1ULL << endSquare;
                    whitePieceTypes[KNIGHT_CODE] -= 1ULL << startSquare;
                    zobristCode ^= zobrist_hashing::WHITE_PIECE_TYPE_CODES[KNIGHT_CODE][startSquare] ^
                                   zobrist_hashing::WHITE_PIECE_TYPE_CODES[KNIGHT_CODE][endSquare];
                } else if (((whitePieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL) {
                    whitePieceTypes[PAWN_CODE] |= 1ULL << endSquare;
                    whitePieceTypes[PAWN_CODE] -= 1ULL << startSquare;
                    zobristCode ^= zobrist_hashing::WHITE_PIECE_TYPE_CODES[PAWN_CODE][startSquare] ^
                                   zobrist_hashing::WHITE_PIECE_TYPE_CODES[PAWN_CODE][endSquare];
                } else {
                    cout << toString() << endl << "White to move. Attempted move " << move << endl;
                    bool thereWasAPieceAtStartSquare = false;
                    assert(thereWasAPieceAtStartSquare);
                }
            }
            allWhitePieces -= 1ULL << startSquare;
            allWhitePieces |= 1ULL << endSquare;
            isItWhiteToMove = false;
            zobristCode ^= zobrist_hashing::IS_IT_WHITE_TO_MOVE_CODE;
        } // end if it is white to move
        else {
            if (((allWhitePieces >> endSquare) & 1ULL) == 1ULL) { // then this move is a capture
                isCapture = true;
                if (whiteKingPosition == endSquare) {
                    cout << toString() << endl << "White king position equals end square; assertion failed" << endl;
                    exit(1);
                }
                if (PROMOTE_TO_QUEEN_FLAG <= flag and flag <= PROMOTE_TO_KNIGHT_FLAG) {
                    for (int capturedPieceType = QUEEN_CODE;
                         capturedPieceType <= KNIGHT_CODE; capturedPieceType++) {
                        if (((whitePieceTypes[capturedPieceType] >> endSquare) & 1ULL) ==
                            1ULL) { // this is the pieceType we are capturing
                            whitePieceTypes[capturedPieceType] -= 1ULL << endSquare;
                            allWhitePieces -= 1ULL << endSquare;
                            zobristCode ^= zobrist_hashing::WHITE_PIECE_TYPE_CODES[capturedPieceType][endSquare];
                            break;
                        }
                    }
                } else {
                    allWhitePieces -= 1ULL << endSquare;
                    whitePieceTypes[flag - CAPTURE_QUEEN_FLAG + QUEEN_CODE] -= 1ULL << endSquare;
                    zobristCode ^= zobrist_hashing::WHITE_PIECE_TYPE_CODES[flag - CAPTURE_QUEEN_FLAG +
                                                                           QUEEN_CODE][endSquare];
                }
            }

            if (move == BLACK_SHORT_CASTLE) {
                blackKingPosition = 55;
                blackPieceTypes[ROOK_CODE] -= 9223231299366420480; // add f8, get rid of h8
                allBlackPieces -= 9187203052103270400; // add f8 and g8, get rid of e8 and h8
                canBlackCastleShort = false;
                if (canBlackCastleLong) {
                    canBlackCastleLong = false;
                    zobristCode ^= zobrist_hashing::BLACK_CAN_CASTLE_LONG_CODE;
                }
                isItWhiteToMove = true;

                zobristCode ^= zobrist_hashing::BLACK_PIECE_TYPE_CODES[ROOK_CODE][47] ^
                               zobrist_hashing::BLACK_PIECE_TYPE_CODES[ROOK_CODE][63] ^
                               zobrist_hashing::BLACK_KING_CODES[39] ^ zobrist_hashing::BLACK_KING_CODES[55] ^
                               zobrist_hashing::BLACK_CAN_CASTLE_SHORT_CODE ^
                               zobrist_hashing::IS_IT_WHITE_TO_MOVE_CODE;

                return;
                // We did this because when we're not castling, we update allBlackPieces at the very end, but if we were castling, we would update it in here.
                // So we don't want to update it twice.
            } else if (move == BLACK_LONG_CASTLE) {
                blackKingPosition = 23;
                blackPieceTypes[ROOK_CODE] += 2147483520; // add d8, get rid of a8
                allBlackPieces -= 547599941760; // add c8 and d8, get rid of e8 and a8
                if (canBlackCastleShort) {
                    canBlackCastleShort = false;
                    zobristCode ^= zobrist_hashing::BLACK_CAN_CASTLE_SHORT_CODE;
                }
                canBlackCastleLong = false;
                isItWhiteToMove = true;

                zobristCode ^= zobrist_hashing::BLACK_PIECE_TYPE_CODES[ROOK_CODE][7] ^
                               zobrist_hashing::BLACK_PIECE_TYPE_CODES[ROOK_CODE][31] ^
                               zobrist_hashing::BLACK_KING_CODES[39] ^ zobrist_hashing::BLACK_KING_CODES[23] ^
                               zobrist_hashing::BLACK_CAN_CASTLE_LONG_CODE ^
                               zobrist_hashing::IS_IT_WHITE_TO_MOVE_CODE;

                return;
                // We did this because when we're not castling, we update allBlackPieces at the very end, but if we were castling, we would update it in here.
                // So we don't want to update it twice.
            } else if (flag == EN_PASSANT_FLAG) {
                whitePieceTypes[PAWN_CODE] -= 2ULL << endSquare;
                allWhitePieces -= 2ULL << endSquare;
                zobristCode ^= zobrist_hashing::WHITE_PIECE_TYPE_CODES[PAWN_CODE][endSquare + 1];
                blackPieceTypes[PAWN_CODE] |= 1ULL << endSquare;
                blackPieceTypes[PAWN_CODE] -= 1ULL << startSquare;
                zobristCode ^= zobrist_hashing::BLACK_PIECE_TYPE_CODES[PAWN_CODE][endSquare] ^
                               zobrist_hashing::BLACK_PIECE_TYPE_CODES[PAWN_CODE][startSquare];
            } else if (PROMOTE_TO_QUEEN_FLAG <= flag and flag <= PROMOTE_TO_KNIGHT_FLAG) {
                blackPieceTypes[PAWN_CODE] -= 1ULL << startSquare;
                zobristCode ^= zobrist_hashing::BLACK_PIECE_TYPE_CODES[PAWN_CODE][startSquare];
                blackPieceTypes[QUEEN_CODE + flag - PROMOTE_TO_QUEEN_FLAG] |= 1ULL << endSquare;
                zobristCode ^= zobrist_hashing::BLACK_PIECE_TYPE_CODES[QUEEN_CODE + flag -
                                                                       PROMOTE_TO_QUEEN_FLAG][endSquare];
            } else {
                if (flag == PAWN_PUSH_TWO_SQUARES_FLAG) {
                    blackPieceTypes[PAWN_CODE] -= 48ULL << (startSquare & 56);
                    zobristCode ^= zobrist_hashing::BLACK_PIECE_TYPE_CODES[PAWN_CODE][endSquare] ^
                                   zobrist_hashing::BLACK_PIECE_TYPE_CODES[PAWN_CODE][startSquare];
                    whichPawnMovedTwoSquares = startSquare >> 3;
                    zobristCode ^= zobrist_hashing::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[255] ^
                                   zobrist_hashing::WHICH_PAWN_MOVED_TWO_SQUARES_CODES[whichPawnMovedTwoSquares];
                } else if (blackKingPosition == startSquare) {
                    blackKingPosition = endSquare;
                    zobristCode ^= zobrist_hashing::BLACK_KING_CODES[endSquare] ^
                                   zobrist_hashing::BLACK_KING_CODES[startSquare];
                    if (canBlackCastleShort) {
                        canBlackCastleShort = false;
                        zobristCode ^= zobrist_hashing::BLACK_CAN_CASTLE_SHORT_CODE;
                    }
                    if (canBlackCastleLong) {
                        canBlackCastleLong = false;
                        zobristCode ^= zobrist_hashing::BLACK_CAN_CASTLE_LONG_CODE;
                    }
                } else if (((blackPieceTypes[QUEEN_CODE] >> startSquare) & 1ULL) == 1ULL) {
                    blackPieceTypes[QUEEN_CODE] |= 1ULL << endSquare;
                    blackPieceTypes[QUEEN_CODE] -= 1ULL << startSquare;
                    zobristCode ^= zobrist_hashing::BLACK_PIECE_TYPE_CODES[QUEEN_CODE][endSquare] ^
                                   zobrist_hashing::BLACK_PIECE_TYPE_CODES[QUEEN_CODE][startSquare];
                } else if (((blackPieceTypes[ROOK_CODE] >> startSquare) & 1ULL) == 1ULL) {
                    blackPieceTypes[ROOK_CODE] |= 1ULL << endSquare;
                    blackPieceTypes[ROOK_CODE] -= 1ULL << startSquare;
                    zobristCode ^= zobrist_hashing::BLACK_PIECE_TYPE_CODES[ROOK_CODE][endSquare] ^
                                   zobrist_hashing::BLACK_PIECE_TYPE_CODES[ROOK_CODE][startSquare];
                } else if (((blackPieceTypes[BISHOP_CODE] >> startSquare) & 1ULL) == 1ULL) {
                    blackPieceTypes[BISHOP_CODE] |= 1ULL << endSquare;
                    blackPieceTypes[BISHOP_CODE] -= 1ULL << startSquare;
                    zobristCode ^= zobrist_hashing::BLACK_PIECE_TYPE_CODES[BISHOP_CODE][endSquare] ^
                                   zobrist_hashing::BLACK_PIECE_TYPE_CODES[BISHOP_CODE][startSquare];
                } else if (((blackPieceTypes[KNIGHT_CODE] >> startSquare) & 1ULL) == 1ULL) {
                    blackPieceTypes[KNIGHT_CODE] |= 1ULL << endSquare;
                    blackPieceTypes[KNIGHT_CODE] -= 1ULL << startSquare;
                    zobristCode ^= zobrist_hashing::BLACK_PIECE_TYPE_CODES[KNIGHT_CODE][endSquare] ^
                                   zobrist_hashing::BLACK_PIECE_TYPE_CODES[KNIGHT_CODE][startSquare];
                } else if (((blackPieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL) {
                    blackPieceTypes[PAWN_CODE] |= 1ULL << endSquare;
                    blackPieceTypes[PAWN_CODE] -= 1ULL << startSquare;
                    zobristCode ^= zobrist_hashing::BLACK_PIECE_TYPE_CODES[PAWN_CODE][endSquare] ^
                                   zobrist_hashing::BLACK_PIECE_TYPE_CODES[PAWN_CODE][startSquare];
                } else {
                    cout << toString() << endl << "Black to move. Attempted move " << move << endl;
                    bool thereWasAPieceAtStartSquare = false;
                    assert(thereWasAPieceAtStartSquare);
                }
            }
            allBlackPieces -= 1ULL << startSquare;
            allBlackPieces |= 1ULL << endSquare;
            isItWhiteToMove = true;
            zobristCode ^= zobrist_hashing::IS_IT_WHITE_TO_MOVE_CODE;
        } // end if it is black to move

        // Now take away castling rights if the rooks are taken or moved
        if ((startSquare == 0 or endSquare == 0) and canWhiteCastleLong) {
            canWhiteCastleLong = false;
            zobristCode ^= zobrist_hashing::WHITE_CAN_CASTLE_LONG_CODE;
        }
        if ((startSquare == 7 or endSquare == 7) and canBlackCastleLong) {
            canBlackCastleLong = false;
            zobristCode ^= zobrist_hashing::BLACK_CAN_CASTLE_LONG_CODE;
        }
        if ((startSquare == 56 or endSquare == 56) and canWhiteCastleShort) {
            canWhiteCastleShort = false;
            zobristCode ^= zobrist_hashing::WHITE_CAN_CASTLE_SHORT_CODE;
        }
        if ((startSquare == 63 or endSquare == 63) and canBlackCastleShort) {
            canBlackCastleShort = false;
            zobristCode ^= zobrist_hashing::BLACK_CAN_CASTLE_SHORT_CODE;
        }

        if (isCapture)
            updateDrawByInsufficientMaterial();

    } // end makemoveInternal method

    void addLegalKingMoves(vector<uint16_t> &legalMoves, unsigned long kingLegalEndSquares) const { // Not castling
        const unsigned long enemyPieces = isItWhiteToMove ? allBlackPieces : allWhitePieces;
        uint8_t myKingPosition = isItWhiteToMove ? whiteKingPosition : blackKingPosition;
        unsigned long thisKingLegalEndSquareMask;
        int thisKingLegalEndSquare;
        while (kingLegalEndSquares != 0ULL) {
            thisKingLegalEndSquareMask = kingLegalEndSquares & -kingLegalEndSquares;
            kingLegalEndSquares -= thisKingLegalEndSquareMask;
            thisKingLegalEndSquare = FastLogarithm::log2(thisKingLegalEndSquareMask);
            if (thisKingLegalEndSquareMask & enemyPieces)
                legalMoves.push_back(getCaptureMove(myKingPosition, thisKingLegalEndSquare));
            else
                legalMoves.push_back(myKingPosition << 10 | thisKingLegalEndSquare << 4 | NORMAL_MOVE_FLAG);
        }
    }

    void addCastling(vector<uint16_t> &legalMoves, const unsigned long enemyAttackedSquares) const {
        unsigned long allPieces = allBlackPieces | allWhitePieces;
        if (isItWhiteToMove) {
            if (canWhiteCastleShort and (enemyAttackedSquares & E1_F1_G1) == 0ULL and
                (allPieces & E1_THROUGH_H1) == E1_H1)
                legalMoves.push_back(WHITE_SHORT_CASTLE);
            if (canWhiteCastleLong and (enemyAttackedSquares & E1_D1_C1) == 0ULL and
                (allPieces & E1_THROUGH_A1) == E1_A1)
                legalMoves.push_back(WHITE_LONG_CASTLE);
        } else {
            if (canBlackCastleShort and (enemyAttackedSquares & E8_F8_G8) == 0ULL and
                (allPieces & E8_THROUGH_H8) == E8_H8)
                legalMoves.push_back(BLACK_SHORT_CASTLE);
            if (canBlackCastleLong and (enemyAttackedSquares & E8_D8_C8) == 0ULL and
                (allPieces & E8_THROUGH_A8) == E8_A8)
                legalMoves.push_back(BLACK_LONG_CASTLE);
        }
    }

    void addEnPassant(vector<uint16_t> &legalMoves, const unsigned long effectiveEnemyBishops,
                      const unsigned long effectiveEnemyRooks) const {
        const unsigned long allPieces = allWhitePieces | allBlackPieces;
        if (pieceGivingCheck == NOT_IN_CHECK_CODE) {
            // Now we add en passant
            if (isItWhiteToMove) {
                if (whichPawnMovedTwoSquares < 8) { // REMINDER: The king is not in check
                    int endSquare = 8 * whichPawnMovedTwoSquares + 5;
                    // Consider left en passant
                    if (whichPawnMovedTwoSquares <
                        7) { // The pawn that pushed two squares is not on the h-file, so we can en passant left
                        int startSquare = 8 * whichPawnMovedTwoSquares + 12;
                        unsigned long allPiecesAfterCapture =
                                allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 4)) -
                                (1ULL << startSquare);
                        if (((whitePieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and
                            (getMagicBishopAttackedSquares(whiteKingPosition, allPiecesAfterCapture) &
                             effectiveEnemyBishops) == 0ULL and
                            (getMagicRookAttackedSquares(whiteKingPosition, allPiecesAfterCapture) &
                             effectiveEnemyRooks) == 0ULL) {
                            legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                        } // end if this en passant doesn't reveal an attack on our king
                    } // end if we can left en passant

                    // Consider right en passant
                    if (whichPawnMovedTwoSquares >
                        0) { // The pawn that pushed two squares is not on the a-file, so we can en passant right.
                        int startSquare = 8 * whichPawnMovedTwoSquares - 4;
                        unsigned long allPiecesAfterCapture =
                                allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 4)) -
                                (1ULL << startSquare);
                        if (((whitePieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and
                            (getMagicBishopAttackedSquares(whiteKingPosition, allPiecesAfterCapture) &
                             effectiveEnemyBishops) == 0ULL and
                            (getMagicRookAttackedSquares(whiteKingPosition, allPiecesAfterCapture) &
                             effectiveEnemyRooks) == 0ULL) {
                            legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                        } // end if this en passant doesn't reveal an attack on our king
                    } // end if we can right en passant
                } // end if we can en passant
            } // end if it is white to move
            else {
                if (whichPawnMovedTwoSquares < 8) { // REMINDER: The king is not in check
                    int endSquare = 8 * whichPawnMovedTwoSquares + 2;
                    // Consider left en passant
                    if (whichPawnMovedTwoSquares <
                        7) { // The pawn that pushed two squares is not on the h-file, so we can en passant left
                        int startSquare = 8 * whichPawnMovedTwoSquares + 11;
                        unsigned long allPiecesAfterCapture =
                                allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 3)) -
                                (1ULL << startSquare);
                        if (((blackPieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and
                            (getMagicBishopAttackedSquares(blackKingPosition, allPiecesAfterCapture) &
                             effectiveEnemyBishops) == 0ULL and
                            (getMagicRookAttackedSquares(blackKingPosition, allPiecesAfterCapture) &
                             effectiveEnemyRooks) == 0ULL) {
                            legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                        } // end if this en passant doesn't reveal an attack on our king
                    } // end if we can left en passant

                    // Consider right en passant
                    if (whichPawnMovedTwoSquares >
                        0) { // The pawn that pushed two squares is not on the a-file, so we can en passant right.
                        int startSquare = 8 * whichPawnMovedTwoSquares - 5;
                        unsigned long allPiecesAfterCapture =
                                allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 3)) -
                                (1ULL << startSquare);
                        if (((blackPieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and
                            (getMagicBishopAttackedSquares(blackKingPosition, allPiecesAfterCapture) &
                             effectiveEnemyBishops) == 0ULL and
                            (getMagicRookAttackedSquares(blackKingPosition, allPiecesAfterCapture) &
                             effectiveEnemyRooks) == 0ULL) {
                            legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                        } // end if this en passant doesn't reveal an attack on our king
                    } // end if we can right en passant
                } // end if we can en passant
            } // end if it is black to move
        } // end if we are not in check

        else { // we are in single check
            if (isItWhiteToMove) {
                if (whichPawnMovedTwoSquares < 8 and pieceGivingCheck == 8 * whichPawnMovedTwoSquares +
                                                                         4) { // REMINDER: We are in check by a pawn that we can en passant capture. En passant is never legal after a discovered check.
                    int endSquare = 8 * whichPawnMovedTwoSquares + 5;
                    // Left en passant
                    if (whichPawnMovedTwoSquares <
                        7) { // the pawn that pushed 2 squares is not the h-pawn, so we can en passant left.
                        int startSquare = 8 * whichPawnMovedTwoSquares + 12;
                        unsigned long allPiecesAfterCapture =
                                allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 4)) -
                                (1ULL << startSquare);\
                    if (((whitePieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and
                        (getMagicRookAttackedSquares(whiteKingPosition, allPiecesAfterCapture) & effectiveEnemyRooks) ==
                        0ULL)
                            legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                    } // end if we can en passant left

                    // Consider right en passant
                    if (whichPawnMovedTwoSquares >
                        0) { // The pawn that pushed two squares is not on the a-file, so we can en passant right.
                        int startSquare = 8 * whichPawnMovedTwoSquares - 4;
                        unsigned long allPiecesAfterCapture =
                                allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 4)) -
                                (1ULL << startSquare);
                        if (((whitePieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and
                            (getMagicRookAttackedSquares(whiteKingPosition, allPiecesAfterCapture) &
                             effectiveEnemyRooks) == 0ULL) {
                            legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                        } // end if this en passant doesn't reveal an attack on our king
                    } // end if we can right en passant
                } // end if we can en passant at all
            } // end if it is white to move
            else { // it is black to move
                if (whichPawnMovedTwoSquares < 8 and pieceGivingCheck == 8 * whichPawnMovedTwoSquares +
                                                                         3) { // REMINDER: We are in check by a pawn that we can en passant capture. En passant is never legal after a discovered check.
                    int endSquare = 8 * whichPawnMovedTwoSquares + 2;
                    // Left en passant
                    if (whichPawnMovedTwoSquares <
                        7) { // the pawn that pushed 2 squares is not the h-pawn, so we can en passant left.
                        int startSquare = 8 * whichPawnMovedTwoSquares + 11;
                        unsigned long allPiecesAfterCapture =
                                allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 3)) -
                                (1ULL << startSquare);\
                    if (((blackPieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and
                        (getMagicRookAttackedSquares(blackKingPosition, allPiecesAfterCapture) & effectiveEnemyRooks) ==
                        0ULL)
                            legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                    } // end if we can en passant left

                    // Consider right en passant
                    if (whichPawnMovedTwoSquares >
                        0) { // The pawn that pushed two squares is not on the a-file, so we can en passant right.
                        int startSquare = 8 * whichPawnMovedTwoSquares - 5;
                        unsigned long allPiecesAfterCapture =
                                allPieces + (1ULL << endSquare) - (1ULL << (8 * whichPawnMovedTwoSquares + 3)) -
                                (1ULL << startSquare);
                        if (((blackPieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL and
                            (getMagicRookAttackedSquares(blackKingPosition, allPiecesAfterCapture) &
                             effectiveEnemyRooks) == 0ULL) {
                            legalMoves.push_back((startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG);
                        } // end if this en passant doesn't reveal an attack on our king
                    } // end if we can right en passant
                } // end if we can en passant at all
            } // end if it is black to move
        } // end else (if we are in single check)
    } // end addEnPassant method

    explicit ParameterChessBoard(const string &fenNotation) {
        halfmoveClock = 0; // This is incorrect, but it isn't a big issue

        int x = 0;
        int y = 7;

        int charPointer = 0;
        char currentChar;
        // Read the actual pieces

        whiteKingPosition = 64;
        blackKingPosition = 64;
        for (int pieceType = QUEEN_CODE; pieceType <= PAWN_CODE; pieceType++) {
            whitePieceTypes[pieceType] = 0;
            blackPieceTypes[pieceType] = 0;
        }
        do {
            currentChar = fenNotation[charPointer];
            charPointer++;

            switch (currentChar) {
                case 'K':
                    whiteKingPosition = 8 * x + y;
                    x++;
                    break;
                case 'Q':
                    whitePieceTypes[QUEEN_CODE] |= 1ULL << (8 * x + y);
                    x++;
                    break;
                case 'R':
                    whitePieceTypes[ROOK_CODE] |= 1ULL << (8 * x + y);
                    x++;
                    break;
                case 'B':
                    whitePieceTypes[BISHOP_CODE] |= 1ULL << (8 * x + y);
                    x++;
                    break;
                case 'N':
                    whitePieceTypes[KNIGHT_CODE] |= 1ULL << (8 * x + y);
                    x++;
                    break;
                case 'P':
                    whitePieceTypes[PAWN_CODE] |= 1ULL << (8 * x + y);
                    x++;
                    break;
                case 'k':
                    blackKingPosition = 8 * x + y;
                    x++;
                    break;
                case 'q':
                    blackPieceTypes[QUEEN_CODE] |= 1ULL << (8 * x + y);
                    x++;
                    break;
                case 'r':
                    blackPieceTypes[ROOK_CODE] |= 1ULL << (8 * x + y);
                    x++;
                    break;
                case 'b':
                    blackPieceTypes[BISHOP_CODE] |= 1ULL << (8 * x + y);
                    x++;
                    break;
                case 'n':
                    blackPieceTypes[KNIGHT_CODE] |= 1ULL << (8 * x + y);
                    x++;
                    break;
                case 'p':
                    blackPieceTypes[PAWN_CODE] |= 1ULL << (8 * x + y);
                    x++;
                    break;
                case '1':
                    x += 1;
                    break;
                case '2':
                    x += 2;
                    break;
                case '3':
                    x += 3;
                    break;
                case '4':
                    x += 4;
                    break;
                case '5':
                    x += 5;
                    break;
                case '6':
                    x += 6;
                    break;
                case '7':
                    x += 7;
                    break;
                case '8':
                    x += 8;
                    break;
                case '/':
                    x = 0;
                    y--;
                    break;
                case ' ':
                    break;
                default:
                    cout << "Invalid FEN: Unexpected symbol in pieces: " << currentChar << endl;
                    exit(1);
                    break;
            }
        } while (currentChar != ' ');

        allWhitePieces = 1ULL << whiteKingPosition;
        allBlackPieces = 1ULL << blackKingPosition;
        for (int pieceType = QUEEN_CODE; pieceType <= PAWN_CODE; pieceType++) {
            allWhitePieces |= whitePieceTypes[pieceType];
            allBlackPieces |= blackPieceTypes[pieceType];
        }

        // now handle whose move it is
        currentChar = fenNotation[charPointer];
        charPointer++;

        switch (currentChar) {
            case 'w':
                isItWhiteToMove = true;
                break;
            case 'b':
                isItWhiteToMove = false;
                break;
            default:
                cout << "Invalid FEN: Unexpected character in whose turn it is" << endl;
                exit(1);
        }

        currentChar = fenNotation[charPointer];
        charPointer++;
        assert(currentChar == ' ');

        // handle who has castling rights where
        canWhiteCastleShort = false;
        canWhiteCastleLong = false;
        canBlackCastleShort = false;
        canBlackCastleLong = false;

        currentChar = fenNotation[charPointer];
        charPointer++;
        do {
            switch (currentChar) {
                case 'K':
                    canWhiteCastleShort = true;
                    break;
                case 'Q':
                    canWhiteCastleLong = true;
                    break;
                case 'k':
                    canBlackCastleShort = true;
                    break;
                case 'q':
                    canBlackCastleLong = true;
                    break;
                case '-':
                    break;
                default:
                    cout << "Invalid FEN: Unexpected character in castling rights: " << currentChar << endl;
                    break;
            }
            currentChar = fenNotation[charPointer];
            charPointer++;
        } while (currentChar != ' ');

        currentChar = fenNotation[charPointer];
        charPointer++;

        whichPawnMovedTwoSquares = currentChar - 'a';
        if (whichPawnMovedTwoSquares > 7)
            whichPawnMovedTwoSquares = 255;

        drawByInsufficientMaterial = false;
        drawByStalemate = false;
        whiteWonByCheckmate = false;
        blackWonByCheckmate = false;
        updateMates();

        // Check for draw by insufficient material
        updateDrawByInsufficientMaterial();
        manuallyInitializeZobristCode();
        updatePieceGivingCheck();
    }

    uint16_t getCaptureMove(int startSquare, int endSquare) const {
        if (isItWhiteToMove) {
            if (((blackPieceTypes[PAWN_CODE] >> endSquare) & 1ULL) == 1ULL)
                return (startSquare << 10) + (endSquare << 4) + CAPTURE_PAWN_FLAG;
            if (((blackPieceTypes[KNIGHT_CODE] >> endSquare) & 1ULL) == 1ULL)
                return (startSquare << 10) + (endSquare << 4) + CAPTURE_KNIGHT_FLAG;
            if (((blackPieceTypes[BISHOP_CODE] >> endSquare) & 1ULL) == 1ULL)
                return (startSquare << 10) + (endSquare << 4) + CAPTURE_BISHOP_FLAG;
            if (((blackPieceTypes[ROOK_CODE] >> endSquare) & 1ULL) == 1ULL)
                return (startSquare << 10) + (endSquare << 4) + CAPTURE_ROOK_FLAG;
            if (((blackPieceTypes[QUEEN_CODE] >> endSquare) & 1ULL) == 1ULL)
                return (startSquare << 10) + (endSquare << 4) + CAPTURE_QUEEN_FLAG;
        } else {
            if (((whitePieceTypes[PAWN_CODE] >> endSquare) & 1ULL) == 1ULL)
                return (startSquare << 10) + (endSquare << 4) + CAPTURE_PAWN_FLAG;
            if (((whitePieceTypes[KNIGHT_CODE] >> endSquare) & 1ULL) == 1ULL)
                return (startSquare << 10) + (endSquare << 4) + CAPTURE_KNIGHT_FLAG;
            if (((whitePieceTypes[BISHOP_CODE] >> endSquare) & 1ULL) == 1ULL)
                return (startSquare << 10) + (endSquare << 4) + CAPTURE_BISHOP_FLAG;
            if (((whitePieceTypes[ROOK_CODE] >> endSquare) & 1ULL) == 1ULL)
                return (startSquare << 10) + (endSquare << 4) + CAPTURE_ROOK_FLAG;
            if (((whitePieceTypes[QUEEN_CODE] >> endSquare) & 1ULL) == 1ULL)
                return (startSquare << 10) + (endSquare << 4) + CAPTURE_QUEEN_FLAG;
        }
        cout << toString() << endl << "is it white to move: " << isItWhiteToMove << endl;
        cout << "getCaptureMove isn't a move. Start square is " << startSquare << ". End square is " << endSquare << "."
             << endl;
        exit(1);
    }

    void updateDrawByInsufficientMaterial() {
        // Scenario 1: K vs K, K + B vs K, or each side has bishops and they are all on the same color.
        unsigned long allWhitePiecesExceptKings = allWhitePieces - (1ULL << whiteKingPosition);
        unsigned long allBlackPiecesExceptKings = allBlackPieces - (1ULL << blackKingPosition);
        if (allWhitePiecesExceptKings == whitePieceTypes[BISHOP_CODE] and
            allBlackPiecesExceptKings == blackPieceTypes[BISHOP_CODE] and (
                    ((allWhitePiecesExceptKings | allBlackPiecesExceptKings) & DARK_SQUARES) == DARK_SQUARES or
                    ((allWhitePiecesExceptKings | allBlackPiecesExceptKings) & LIGHT_SQUARES) == LIGHT_SQUARES)
                )
            drawByInsufficientMaterial = true;

            // Scenario 2: K + N vs K
        else if (
                (whitePieceTypes[KNIGHT_CODE] == (-whitePieceTypes[KNIGHT_CODE] & whitePieceTypes[KNIGHT_CODE])) and
                (whitePieceTypes[KNIGHT_CODE] | (1ULL << whiteKingPosition)) == allWhitePieces and
                allBlackPieces == (1ULL << blackKingPosition))
            drawByInsufficientMaterial = true;

            // Scenario 3: K vs K + N
        else if ((1ULL << whiteKingPosition) == allWhitePieces and
                 blackPieceTypes[KNIGHT_CODE] == (-blackPieceTypes[KNIGHT_CODE] & blackPieceTypes[KNIGHT_CODE]) and
                 ((1ULL << blackKingPosition) | blackPieceTypes[KNIGHT_CODE]) == allBlackPieces)
            drawByInsufficientMaterial = true;
    }

public:
    ParameterChessBoard() {
        whiteKingPosition = 32;
        allWhitePieces = 0x0303030303030303ULL;

        blackKingPosition = whiteKingPosition + 7;
        allBlackPieces = 0xc0c0c0c0c0c0c0c0ULL;

        isItWhiteToMove = true;
        canWhiteCastleShort = true;
        canWhiteCastleLong = true;
        canBlackCastleShort = true;
        canBlackCastleLong = true;

        whichPawnMovedTwoSquares = 255;
        drawByInsufficientMaterial = false;
        drawByStalemate = false;
        whiteWonByCheckmate = false;
        blackWonByCheckmate = false;

        pieceGivingCheck = NOT_IN_CHECK_CODE;
        halfmoveClock = 0;
        manuallyInitializeZobristCode();
    }

    static ParameterChessBoard boardFromFENNotation(const string &fenNotation) {
        return ParameterChessBoard(fenNotation);
    }

    string toFenNotation() const {
        string fenNotation;
        int numEmptySquares;
        // Step 1: Pieces
        for (int rank = 7; rank >= 0; rank--) {
            numEmptySquares = 0;
            for (int file = 0; file < 8; file++) {
                int square = 8 * file + rank;
                unsigned long squareMask = 1ULL << square;
                char piece;
                if (whiteKingPosition == square) {
                    piece = 'K';
                } else if (blackKingPosition == square) {
                    piece = 'k';
                } else if (squareMask & whitePieceTypes[QUEEN_CODE]) {
                    piece = 'Q';
                } else if (squareMask & blackPieceTypes[QUEEN_CODE]) {
                    piece = 'q';
                } else if (squareMask & whitePieceTypes[ROOK_CODE]) {
                    piece = 'R';
                } else if (squareMask & blackPieceTypes[ROOK_CODE]) {
                    piece = 'r';
                } else if (squareMask & whitePieceTypes[BISHOP_CODE]) {
                    piece = 'B';
                } else if (squareMask & blackPieceTypes[BISHOP_CODE]) {
                    piece = 'b';
                } else if (squareMask & whitePieceTypes[KNIGHT_CODE]) {
                    piece = 'N';
                } else if (squareMask & blackPieceTypes[KNIGHT_CODE]) {
                    piece = 'n';
                } else if (squareMask & whitePieceTypes[PAWN_CODE]) {
                    piece = 'P';
                } else if (squareMask & blackPieceTypes[PAWN_CODE]) {
                    piece = 'p';
                } else {
                    piece = ' ';
                }
                // We are done finding the piece
                if (piece == ' ') {
                    numEmptySquares++;
                    if (file == 7)
                        fenNotation += string(1, '0' + numEmptySquares);
                } else { // there is an actual piece there
                    if (numEmptySquares != 0) {
                        fenNotation += string(1, '0' + numEmptySquares);
                        numEmptySquares = 0;
                    }
                    fenNotation += string(1, piece);
                }
            } // end for loop over file
            fenNotation += "/";
        } // end for loop over rank

        // Step 2: Whose turn it is to move
        if (isItWhiteToMove) {
            fenNotation += " w ";
        } else {
            fenNotation += " b ";
        }

        // Step 3: Castling rights
        if (canWhiteCastleShort)
            fenNotation += "K";
        if (canWhiteCastleLong)
            fenNotation += "Q";
        if (canBlackCastleShort)
            fenNotation += "k";
        if (canBlackCastleLong)
            fenNotation += "q";
        if (!(canWhiteCastleShort or canWhiteCastleLong or canBlackCastleShort or canBlackCastleLong))
            fenNotation += "-";

        fenNotation += " ";

        // Step 4: En passant rights
        if (whichPawnMovedTwoSquares < 8) {
            fenNotation += string(1, 'a' + whichPawnMovedTwoSquares);
            if (isItWhiteToMove)
                fenNotation += "6";
            else
                fenNotation += "3";
        } else {
            fenNotation += "-";
        }

        fenNotation += " ";

        // Step 5: Halfmove clock

        fenNotation += to_string(halfmoveClock);

        // Step 6: Fullmove clock. We don't have the information to accurately do it, so we will output 0.
        fenNotation += " 1";

        // And now we return the result
        return fenNotation;
    }

    string toString() const {
        string boardString;
        char piece;
        for (int rank = 7; rank >= 0; rank--) {
            string row = "+---+---+---+---+---+---+---+---+\n|";
            for (int file = 0; file < 8; file++) {
                int square = 8 * file + rank;
                unsigned long squareMask = 1ULL << square;

                // Find what the piece is
                if (whiteKingPosition == square) {
                    piece = 'K';
                } else if (blackKingPosition == square) {
                    piece = 'k';
                } else if (squareMask & whitePieceTypes[QUEEN_CODE]) {
                    piece = 'Q';
                } else if (squareMask & blackPieceTypes[QUEEN_CODE]) {
                    piece = 'q';
                } else if (squareMask & whitePieceTypes[ROOK_CODE]) {
                    piece = 'R';
                } else if (squareMask & blackPieceTypes[ROOK_CODE]) {
                    piece = 'r';
                } else if (squareMask & whitePieceTypes[BISHOP_CODE]) {
                    piece = 'B';
                } else if (squareMask & blackPieceTypes[BISHOP_CODE]) {
                    piece = 'b';
                } else if (squareMask & whitePieceTypes[KNIGHT_CODE]) {
                    piece = 'N';
                } else if (squareMask & blackPieceTypes[KNIGHT_CODE]) {
                    piece = 'n';
                } else if (squareMask & whitePieceTypes[PAWN_CODE]) {
                    piece = 'P';
                } else if (squareMask & blackPieceTypes[PAWN_CODE]) {
                    piece = 'p';
                } else {
                    piece = ' ';
                }
                // We are done finding the piece

                row += " " + string(1, piece) + " |";
            }
            row += " " + to_string(rank + 1) + "\n";
            boardString += row;
        }
        boardString += "+-a-+-b-+-c-+-d-+-e-+-f-+-g-+-h-+";
        return boardString;
    }

    uint8_t calculatePieceGivingCheck() const {
        unsigned long allPieces = allWhitePieces | allBlackPieces;

        uint8_t checker = NOT_IN_CHECK_CODE;

        if (isItWhiteToMove) {
            //unsigned long blackPiecesGivingCheck;
            // Step 1: Pawns
            // Captures towards the a-file
            if ((((blackPieceTypes[PAWN_CODE] & NOT_A_FILE) >> (whiteKingPosition + 9)) & 1ULL) == 1ULL)
                checker = whiteKingPosition + 9;
                // Captures towards the h-file
            else if (((((blackPieceTypes[PAWN_CODE] & NOT_H_FILE) << 7) >> whiteKingPosition) & 1ULL) == 1ULL)
                checker = whiteKingPosition - 7;

            unsigned long blackKnightsGivingCheck =
                    getMagicKnightAttackedSquares(whiteKingPosition) & blackPieceTypes[KNIGHT_CODE];
            if (blackKnightsGivingCheck != 0ULL)
                checker = FastLogarithm::log2(blackKnightsGivingCheck);

            unsigned long blackRooksGivingCheck = getMagicRookAttackedSquares(whiteKingPosition, allPieces) &
                                                  (blackPieceTypes[ROOK_CODE] | blackPieceTypes[QUEEN_CODE]);
            if (blackRooksGivingCheck != 0ULL) {
                if (checker == NOT_IN_CHECK_CODE) {
                    checker = FastLogarithm::log2(blackRooksGivingCheck);
                    if (1ULL << checker != blackRooksGivingCheck)
                        return DOUBLE_CHECK_CODE;
                } else
                    return DOUBLE_CHECK_CODE;
            }

            unsigned long blackBishopsGivingCheck = getMagicBishopAttackedSquares(whiteKingPosition, allPieces) &
                                                    (blackPieceTypes[BISHOP_CODE] | blackPieceTypes[QUEEN_CODE]);
            if (blackBishopsGivingCheck != 0ULL) {
                if (checker == NOT_IN_CHECK_CODE)
                    return FastLogarithm::log2(blackBishopsGivingCheck);
                else
                    return DOUBLE_CHECK_CODE;
            }

            return checker;
        } // end if it is white to move
        else {
            unsigned long whitePiecesLeft;
            // Step 1: Pawns
            // Captures towards the a-file
            if (((((whitePieceTypes[PAWN_CODE] & NOT_A_FILE) >> 7) >> blackKingPosition) & 1ULL) == 1ULL)
                checker = blackKingPosition + 7;
                // Captures towards the h-file
            else if (((((whitePieceTypes[PAWN_CODE] & NOT_H_FILE) << 9) >> blackKingPosition) & 1ULL) == 1ULL)
                checker = blackKingPosition - 9;

            unsigned long whiteKnightsGivingCheck =
                    getMagicKnightAttackedSquares(blackKingPosition) & whitePieceTypes[KNIGHT_CODE];
            if (whiteKnightsGivingCheck != 0ULL)
                checker = FastLogarithm::log2(whiteKnightsGivingCheck);

            unsigned long whiteRooksGivingCheck = getMagicRookAttackedSquares(blackKingPosition, allPieces) &
                                                  (whitePieceTypes[ROOK_CODE] | whitePieceTypes[QUEEN_CODE]);
            if (whiteRooksGivingCheck != 0ULL) {
                if (checker == NOT_IN_CHECK_CODE) {
                    checker = FastLogarithm::log2(whiteRooksGivingCheck);
                    if (1ULL << checker != whiteRooksGivingCheck)
                        return DOUBLE_CHECK_CODE;
                } else
                    return DOUBLE_CHECK_CODE;
            }

            unsigned long whiteBishopsGivingCheck = getMagicBishopAttackedSquares(blackKingPosition, allPieces) &
                                                    (whitePieceTypes[BISHOP_CODE] | whitePieceTypes[QUEEN_CODE]);
            if (whiteBishopsGivingCheck != 0ULL) {
                if (checker == NOT_IN_CHECK_CODE)
                    return FastLogarithm::log2(whiteBishopsGivingCheck);
                else
                    return DOUBLE_CHECK_CODE;
            }

            return checker;
        } // end if it is black to move
    } // end calculatePieceGivingCheck

    unsigned long calculateWhiteAttackedSquares() const {
        // AKA: Squares where black can't move their king
        // This includes pinned pieces
        // This includes squares with the player's pieces
        // So if white has an unprotected knight, the square with the knight is not attacked because black can take that knight
        // However, if white has two knights protecting each other, they will both be included because black can't take either knight.
        unsigned long attackedSquares = getMagicKingAttackedSquares(whiteKingPosition);
        unsigned long meaningfulBlockers = (allWhitePieces | allBlackPieces) - (1ULL << blackKingPosition);

        unsigned long whitePiecesRemaining;
        unsigned long thisWhitePiece;

        for (int pieceType = QUEEN_CODE; pieceType <= KNIGHT_CODE; pieceType++) {
            whitePiecesRemaining = whitePieceTypes[pieceType];
            while (whitePiecesRemaining != 0ULL) {
                thisWhitePiece = whitePiecesRemaining & -whitePiecesRemaining;
                whitePiecesRemaining -= thisWhitePiece;
                attackedSquares |= getMagicWhiteAttackedSquares(pieceType, FastLogarithm::log2(thisWhitePiece),
                                                                meaningfulBlockers);
            }
        }

        // Step 5: Pawns
        attackedSquares |= (whitePieceTypes[PAWN_CODE] & NOT_A_FILE) >> 7;
        attackedSquares |= (whitePieceTypes[PAWN_CODE] & NOT_H_FILE) << 9;

        return attackedSquares;
    }

    unsigned long calculateBlackAttackedSquares() const {
        // AKA: Squares where white can't move their king
        // This includes pinned pieces
        // This includes squares with the player's pieces
        // So if white has an unprotected knight, the square with the knight is not attacked because black can take that knight
        // However, if white has two knights protecting each other, they will both be included because black can't take either knight.
        unsigned long attackedSquares = getMagicKingAttackedSquares(blackKingPosition);
        unsigned long meaningfulBlockers = (allWhitePieces | allBlackPieces) - (1ULL << whiteKingPosition);

        unsigned long blackPiecesRemaining;
        unsigned long thisblackPiece;

        for (int pieceType = QUEEN_CODE; pieceType <= KNIGHT_CODE; pieceType++) {
            blackPiecesRemaining = blackPieceTypes[pieceType];
            while (blackPiecesRemaining != 0ULL) {
                thisblackPiece = blackPiecesRemaining & -blackPiecesRemaining;
                blackPiecesRemaining -= thisblackPiece;
                attackedSquares |= getMagicBlackAttackedSquares(pieceType, FastLogarithm::log2(thisblackPiece),
                                                                meaningfulBlockers);
            }
        }

        // Step 5: Pawns
        attackedSquares |= (blackPieceTypes[PAWN_CODE] & NOT_A_FILE) >> 9;
        attackedSquares |= (blackPieceTypes[PAWN_CODE] & NOT_H_FILE) << 7;
        return attackedSquares;
    }

    void makemove(uint16_t move) {
        makemoveInternal(move);
        updatePieceGivingCheck();
        updateMates();
    }

    void getLegalMoves(vector<uint16_t> &legalMoves) const {
        const unsigned long diagonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicBishopAttackedSquares(
                whiteKingPosition) : getEmptyBoardMagicBishopAttackedSquares(blackKingPosition);
        const unsigned long orthogonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicRookAttackedSquares(
                whiteKingPosition) : getEmptyBoardMagicRookAttackedSquares(blackKingPosition);
        const unsigned long effectiveEnemyBishops = isItWhiteToMove ? diagonalSquaresFromKing &
                                                                      (blackPieceTypes[BISHOP_CODE] |
                                                                       blackPieceTypes[QUEEN_CODE]) :
                                                    diagonalSquaresFromKing &
                                                    (whitePieceTypes[BISHOP_CODE] | whitePieceTypes[QUEEN_CODE]);
        const unsigned long effectiveEnemyRooks = isItWhiteToMove ? orthogonalSquaresFromKing &
                                                                    (blackPieceTypes[ROOK_CODE] |
                                                                     blackPieceTypes[QUEEN_CODE]) :
                                                  orthogonalSquaresFromKing &
                                                  (whitePieceTypes[ROOK_CODE] | whitePieceTypes[QUEEN_CODE]);
        const unsigned long enemyAttackedSquares = isItWhiteToMove ? calculateBlackAttackedSquares()
                                                                   : calculateWhiteAttackedSquares();
        const uint8_t myKingPosition = isItWhiteToMove ? whiteKingPosition : blackKingPosition;
        const unsigned long myPieces = isItWhiteToMove ? allWhitePieces : allBlackPieces;
        const unsigned long enemyPieces = isItWhiteToMove ? allBlackPieces : allWhitePieces;
        const unsigned long allPieces = allWhitePieces | allBlackPieces;

        unsigned long bishopPinnedPieces = 0ULL;
        unsigned long rookPinnedPieces = 0ULL;

        unsigned long thisPinningPieceMask, interposingSquares, interposingOccupiedSquares;
        int thisPinningPieceSquare;
        unsigned long effectiveEnemyBishops1 = effectiveEnemyBishops, effectiveEnemyRooks1 = effectiveEnemyRooks;
        while (effectiveEnemyBishops1 != 0ULL) {
            thisPinningPieceMask = effectiveEnemyBishops1 & -effectiveEnemyBishops1;
            effectiveEnemyBishops1 -= thisPinningPieceMask;
            thisPinningPieceSquare = FastLogarithm::log2(thisPinningPieceMask);
            interposingSquares = lookupBishopCheckResponses(myKingPosition, thisPinningPieceSquare) -
                                 (1ULL << thisPinningPieceSquare);
            interposingOccupiedSquares = interposingSquares & allPieces;
            if ((1ULL << FastLogarithm::log2(interposingOccupiedSquares)) == interposingOccupiedSquares)
                bishopPinnedPieces |= interposingSquares;
            // In other words, if there is only one piece between the Bishop and the King, then it's pinned
            // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
        } // end while loop
        while (effectiveEnemyRooks1 != 0ULL) {
            thisPinningPieceMask = effectiveEnemyRooks1 & -effectiveEnemyRooks1;
            effectiveEnemyRooks1 -= thisPinningPieceMask;
            thisPinningPieceSquare = FastLogarithm::log2(thisPinningPieceMask);
            interposingSquares =
                    lookupRookCheckResponses(myKingPosition, thisPinningPieceSquare) - (1ULL << thisPinningPieceSquare);
            interposingOccupiedSquares = interposingSquares & allPieces;
            if ((1ULL << FastLogarithm::log2(interposingOccupiedSquares)) == interposingOccupiedSquares)
                rookPinnedPieces |= interposingSquares;
            // In other words, if there is only one piece between the Rook and the King, then it's pinned
            // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
        } // end while loop

        const unsigned long kingLegalEndSquares =
                getMagicKingAttackedSquares(myKingPosition) & ~enemyAttackedSquares & ~myPieces;
        addLegalKingMoves(legalMoves, kingLegalEndSquares);
        if (pieceGivingCheck == DOUBLE_CHECK_CODE)
            return;

        unsigned long legalCheckBlockSquares = ENTIRE_BOARD;
        addEnPassant(legalMoves, effectiveEnemyBishops, effectiveEnemyRooks);
        if (pieceGivingCheck == NOT_IN_CHECK_CODE)
            addCastling(legalMoves, enemyAttackedSquares);
        else
            legalCheckBlockSquares = lookupCheckResponses(myKingPosition, this->pieceGivingCheck);

        unsigned long piecesRemaining;
        unsigned long startSquareMask;
        int startSquare;
        unsigned long legalEndSquares;
        unsigned long endSquareMask;
        int endSquare;

        // Loop over all piece types, except pawns.
        for (int pieceType = KNIGHT_CODE; pieceType >= QUEEN_CODE; pieceType--) {
            piecesRemaining = isItWhiteToMove ? whitePieceTypes[pieceType] : blackPieceTypes[pieceType];
            while (piecesRemaining != 0ULL) {
                startSquareMask = piecesRemaining & -piecesRemaining;
                piecesRemaining -= startSquareMask;
                startSquare = FastLogarithm::log2(startSquareMask);
                legalEndSquares =
                        getMagicWhiteAttackedSquares(pieceType, startSquare, allPieces) & legalCheckBlockSquares &
                        ~myPieces;
                if (bishopPinnedPieces & startSquareMask)
                    legalEndSquares &= diagonalSquaresFromKing & getEmptyBoardMagicBishopAttackedSquares(startSquare);
                else if (rookPinnedPieces & startSquareMask)
                    legalEndSquares &= orthogonalSquaresFromKing & getEmptyBoardMagicRookAttackedSquares(startSquare);

                while (legalEndSquares != 0ULL) {
                    endSquareMask = legalEndSquares & -legalEndSquares;
                    legalEndSquares -= endSquareMask;
                    endSquare = FastLogarithm::log2(endSquareMask);
                    if (enemyPieces & endSquareMask)
                        legalMoves.push_back(getCaptureMove(startSquare, endSquare));
                    else
                        legalMoves.push_back(startSquare << 10 | endSquare << 4 | NORMAL_MOVE_FLAG);
                } // end while legalEndSquares is not 0
            } // end while piecesRemaining is not 0
        } // end for loop over all piece types

        const unsigned long myPawns = isItWhiteToMove ? whitePieceTypes[PAWN_CODE] : blackPieceTypes[PAWN_CODE];
        const int leftCaptureOffset = isItWhiteToMove ? 7 : 9;
        const int rightCaptureOffset = isItWhiteToMove ? 9 : 7; // but we are right shifting instead of left shifting
        const int pawnSinglePushOffset = isItWhiteToMove ? 1 : -1;
        const int pawnDoublePushOffset = isItWhiteToMove ? 2 : -2;

        // Pawns that can left capture
        piecesRemaining = myPawns & (enemyPieces & legalCheckBlockSquares) << leftCaptureOffset & ~rookPinnedPieces &
                          (~bishopPinnedPieces | effectiveEnemyBishops << leftCaptureOffset);
        while (piecesRemaining != 0) {
            startSquareMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= startSquareMask;
            startSquare = FastLogarithm::log2(startSquareMask);
            endSquare = startSquare - leftCaptureOffset;
            if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
                legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG);
                legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG);
                legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG);
                legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG);
            } else {
                legalMoves.push_back(getCaptureMove(startSquare, endSquare));
            }
        } // end left captures

        // Pawns that can right capture
        piecesRemaining = myPawns & (enemyPieces & legalCheckBlockSquares) >> rightCaptureOffset & ~rookPinnedPieces &
                          (~bishopPinnedPieces | effectiveEnemyBishops >> rightCaptureOffset);
        while (piecesRemaining != 0) {
            startSquareMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= startSquareMask;
            startSquare = FastLogarithm::log2(startSquareMask);
            endSquare = startSquare + rightCaptureOffset;
            if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
                legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG);
                legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG);
                legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG);
                legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG);
            } else {
                legalMoves.push_back(getCaptureMove(startSquare, endSquare));
            }
        } // end right captures

        // Pawn pushes one square
        piecesRemaining = isItWhiteToMove ?
                          myPawns & (~allPieces & legalCheckBlockSquares) >> 1 & ~bishopPinnedPieces &
                          (~rookPinnedPieces | 255ULL << (myKingPosition & 56)) :
                          myPawns & (~allPieces & legalCheckBlockSquares) << 1 & ~bishopPinnedPieces &
                          (~rookPinnedPieces | 255ULL << (myKingPosition & 56));

        while (piecesRemaining != 0ULL) {
            startSquareMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= startSquareMask;
            startSquare = FastLogarithm::log2(startSquareMask);
            endSquare = startSquare + pawnSinglePushOffset;
            if ((endSquare & 7) == 7 or (endSquare & 7) == 0) {
                legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG);
                legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG);
                legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG);
                legalMoves.push_back(startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG);
            } else {
                legalMoves.push_back(startSquare << 10 | endSquare << 4 | NORMAL_MOVE_FLAG);
            }
        } // end pawn pushes one square

        // Pawn pushes two squares
        piecesRemaining = isItWhiteToMove ?
                          myPawns & SECOND_RANK & ~allPieces >> 1 & ~allPieces >> 2 & legalCheckBlockSquares >> 2 &
                          ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56)) :
                          myPawns & SEVENTH_RANK & ~allPieces << 1 & ~allPieces << 2 & legalCheckBlockSquares << 2 &
                          ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56));

        while (piecesRemaining != 0ULL) {
            startSquareMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= startSquareMask;
            startSquare = FastLogarithm::log2(startSquareMask);
            endSquare = startSquare + pawnDoublePushOffset;
            legalMoves.push_back(startSquare << 10 | endSquare << 4 | PAWN_PUSH_TWO_SQUARES_FLAG);
        } // end pawn pushes two squares
    } // end getLegalMoves method

    bool isThisMoveLegal(const uint16_t move) const {
        vector<uint16_t> legalMoves;
        legalMoves.reserve(50);
        getLegalMoves(legalMoves);
        return std::count(legalMoves.begin(), legalMoves.end(), move) > 0;
    } // end isThisMoveLegal

    bool areThereLegalMoves() const {
        const unsigned long diagonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicBishopAttackedSquares(
                whiteKingPosition) : getEmptyBoardMagicBishopAttackedSquares(blackKingPosition);
        const unsigned long orthogonalSquaresFromKing = isItWhiteToMove ? getEmptyBoardMagicRookAttackedSquares(
                whiteKingPosition) : getEmptyBoardMagicRookAttackedSquares(blackKingPosition);
        const unsigned long effectiveEnemyBishops = isItWhiteToMove ? diagonalSquaresFromKing &
                                                                      (blackPieceTypes[BISHOP_CODE] |
                                                                       blackPieceTypes[QUEEN_CODE]) :
                                                    diagonalSquaresFromKing &
                                                    (whitePieceTypes[BISHOP_CODE] | whitePieceTypes[QUEEN_CODE]);
        const unsigned long effectiveEnemyRooks = isItWhiteToMove ? orthogonalSquaresFromKing &
                                                                    (blackPieceTypes[ROOK_CODE] |
                                                                     blackPieceTypes[QUEEN_CODE]) :
                                                  orthogonalSquaresFromKing &
                                                  (whitePieceTypes[ROOK_CODE] | whitePieceTypes[QUEEN_CODE]);
        const unsigned long enemyAttackedSquares = isItWhiteToMove ? calculateBlackAttackedSquares()
                                                                   : calculateWhiteAttackedSquares();
        const uint8_t myKingPosition = isItWhiteToMove ? whiteKingPosition : blackKingPosition;
        const unsigned long myPieces = isItWhiteToMove ? allWhitePieces : allBlackPieces;
        const unsigned long enemyPieces = isItWhiteToMove ? allBlackPieces : allWhitePieces;
        const unsigned long allPieces = allWhitePieces | allBlackPieces;

        unsigned long bishopPinnedPieces = 0ULL;
        unsigned long rookPinnedPieces = 0ULL;

        unsigned long thisPinningPieceMask, interposingSquares, interposingOccupiedSquares;
        int thisPinningPieceSquare;
        unsigned long effectiveEnemyBishops1 = effectiveEnemyBishops, effectiveEnemyRooks1 = effectiveEnemyRooks;
        while (effectiveEnemyBishops1 != 0ULL) {
            thisPinningPieceMask = effectiveEnemyBishops1 & -effectiveEnemyBishops1;
            effectiveEnemyBishops1 -= thisPinningPieceMask;
            thisPinningPieceSquare = FastLogarithm::log2(thisPinningPieceMask);
            interposingSquares = lookupBishopCheckResponses(myKingPosition, thisPinningPieceSquare) -
                                 (1ULL << thisPinningPieceSquare);
            interposingOccupiedSquares = interposingSquares & allPieces;
            if ((1ULL << FastLogarithm::log2(interposingOccupiedSquares)) == interposingOccupiedSquares)
                bishopPinnedPieces |= interposingSquares;
            // In other words, if there is only one piece between the Bishop and the King, then it's pinned
            // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
        } // end while loop
        while (effectiveEnemyRooks1 != 0ULL) {
            thisPinningPieceMask = effectiveEnemyRooks1 & -effectiveEnemyRooks1;
            effectiveEnemyRooks1 -= thisPinningPieceMask;
            thisPinningPieceSquare = FastLogarithm::log2(thisPinningPieceMask);
            interposingSquares =
                    lookupRookCheckResponses(myKingPosition, thisPinningPieceSquare) - (1ULL << thisPinningPieceSquare);
            interposingOccupiedSquares = interposingSquares & allPieces;
            if ((1ULL << FastLogarithm::log2(interposingOccupiedSquares)) == interposingOccupiedSquares)
                rookPinnedPieces |= interposingSquares;
            // In other words, if there is only one piece between the Rook and the King, then it's pinned
            // And we might have caught an opponent's piece that could give a discovered attack if it were their turn, but that won't affect anything.
        } // end while loop

        // Does the king have legal moves
        if ((getMagicKingAttackedSquares(myKingPosition) & ~enemyAttackedSquares & ~myPieces) != 0)
            return true;

        if (pieceGivingCheck == DOUBLE_CHECK_CODE)
            return false;

        const unsigned long legalCheckBlockSquares =
                pieceGivingCheck == NOT_IN_CHECK_CODE ? ENTIRE_BOARD : lookupCheckResponses(myKingPosition,
                                                                                            this->pieceGivingCheck);

        unsigned long piecesRemaining;
        unsigned long startSquareMask;
        int startSquare;
        unsigned long legalEndSquares;

        // Loop over all piece types, except pawns.
        for (int pieceType = KNIGHT_CODE; pieceType >= QUEEN_CODE; pieceType--) {
            piecesRemaining = isItWhiteToMove ? whitePieceTypes[pieceType] : blackPieceTypes[pieceType];
            while (piecesRemaining != 0ULL) {
                startSquareMask = piecesRemaining & -piecesRemaining;
                piecesRemaining -= startSquareMask;
                startSquare = FastLogarithm::log2(startSquareMask);
                legalEndSquares =
                        getMagicWhiteAttackedSquares(pieceType, startSquare, allPieces) & legalCheckBlockSquares &
                        ~myPieces;
                if (bishopPinnedPieces & startSquareMask)
                    legalEndSquares &= diagonalSquaresFromKing & getEmptyBoardMagicBishopAttackedSquares(startSquare);
                else if (rookPinnedPieces & startSquareMask)
                    legalEndSquares &= orthogonalSquaresFromKing & getEmptyBoardMagicRookAttackedSquares(startSquare);

                if (legalEndSquares != 0)
                    return true;
            } // end while piecesRemaining is not 0
        } // end for loop over all piece types

        const unsigned long myPawns = isItWhiteToMove ? whitePieceTypes[PAWN_CODE] : blackPieceTypes[PAWN_CODE];
        const int leftCaptureOffset = isItWhiteToMove ? 7 : 9;
        const int rightCaptureOffset = isItWhiteToMove ? 9 : 7; // but we are right shifting instead of left shifting
        const int pawnSinglePushOffset = isItWhiteToMove ? 1 : -1;
        const int pawnDoublePushOffset = isItWhiteToMove ? 2 : -2;

        if (isItWhiteToMove ?
            myPawns & (~allPieces & legalCheckBlockSquares) >> 1 & ~bishopPinnedPieces &
            (~rookPinnedPieces | 255ULL << (myKingPosition & 56)) :
            myPawns & (~allPieces & legalCheckBlockSquares) << 1 & ~bishopPinnedPieces &
            (~rookPinnedPieces | 255ULL << (myKingPosition & 56)))
            return true;
        if (isItWhiteToMove ?
            myPawns & SECOND_RANK & ~allPieces >> 1 & ~allPieces >> 2 & legalCheckBlockSquares >> 2 &
            ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56)) :
            myPawns & SEVENTH_RANK & ~allPieces << 1 & ~allPieces << 2 & legalCheckBlockSquares << 2 &
            ~bishopPinnedPieces & (~rookPinnedPieces | 255ULL << (myKingPosition & 56)))
            return true;

        if (myPawns & (enemyPieces & legalCheckBlockSquares) << leftCaptureOffset & ~rookPinnedPieces &
            (~bishopPinnedPieces | effectiveEnemyBishops << leftCaptureOffset))
            return true;
        if (myPawns & (enemyPieces & legalCheckBlockSquares) >> rightCaptureOffset & ~rookPinnedPieces &
            (~bishopPinnedPieces | effectiveEnemyBishops >> rightCaptureOffset))
            return true;

        vector<uint16_t> enPassantMoves;
        addEnPassant(enPassantMoves, effectiveEnemyBishops, effectiveEnemyRooks);
        return !enPassantMoves.empty();
    }

    int perft(const int depth) const {
        if (depth == 0)
            return 1;

        vector<uint16_t> legalMoves;

        legalMoves.reserve(52);


        getLegalMoves(legalMoves);

        int nodeCount = 0;
        for (uint16_t move: legalMoves) {
            ParameterChessBoard copy = *this;
            copy.makemove(move);
            nodeCount += copy.perft(depth - 1);
        }
        return nodeCount;
    }

    void decomposeMove(const uint16_t move, Piece &piece, bool &isCapture, int &startSquare, int &endSquare,
                       uint16_t &flag) const {
        if (isCastle(move)) {
            piece = KING;
            isCapture = false;
            startSquare = move >> 10;
            endSquare = (move >> 4) & 63;
            flag = getFlag(move);
            return;
        }
        startSquare = move >> 10;
        endSquare = (move >> 4) & 63;
        flag = getFlag(move);

        if (isItWhiteToMove)
            isCapture = ((allBlackPieces >> endSquare) & 1ULL) == 1ULL;
        else
            isCapture = ((allWhitePieces >> endSquare) & 1ULL) == 1ULL;

        if (isItWhiteToMove) {
            if (whiteKingPosition == startSquare)
                piece = KING;
            else if (((whitePieceTypes[QUEEN_CODE] >> startSquare) & 1ULL) == 1ULL)
                piece = QUEEN;
            else if (((whitePieceTypes[ROOK_CODE] >> startSquare) & 1ULL) == 1ULL)
                piece = ROOK;
            else if (((whitePieceTypes[BISHOP_CODE] >> startSquare) & 1ULL) == 1ULL)
                piece = BISHOP;
            else if (((whitePieceTypes[KNIGHT_CODE] >> startSquare) & 1ULL) == 1ULL)
                piece = KNIGHT;
            else if (((whitePieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL)
                piece = PAWN;
            else {
                cout << toString() << endl;
                cout << "FEN string is " << toFenNotation();
                cout << "tried to convert move " << move
                     << " to algebraic notation. No piece was found on the start square";
                assert(false);
            }
        } else {
            if (blackKingPosition == startSquare)
                piece = KING;
            else if (((blackPieceTypes[QUEEN_CODE] >> startSquare) & 1ULL) == 1ULL)
                piece = QUEEN;
            else if (((blackPieceTypes[ROOK_CODE] >> startSquare) & 1ULL) == 1ULL)
                piece = ROOK;
            else if (((blackPieceTypes[BISHOP_CODE] >> startSquare) & 1ULL) == 1ULL)
                piece = BISHOP;
            else if (((blackPieceTypes[KNIGHT_CODE] >> startSquare) & 1ULL) == 1ULL)
                piece = KNIGHT;
            else if (((blackPieceTypes[PAWN_CODE] >> startSquare) & 1ULL) == 1ULL)
                piece = PAWN;
            else {
                cout << toString() << endl;
                cout << "FEN string is " << toFenNotation();
                cout << "tried to convert move " << move
                     << " to algebraic notation. No piece was found on the start square";
                assert(false);
            }
        }
    } // end decomposeMove

    string moveToSAN(const uint16_t move) const {
        if (move == WHITE_SHORT_CASTLE or move == BLACK_SHORT_CASTLE)
            return "O-O";
        if (move == WHITE_LONG_CASTLE or move == BLACK_LONG_CASTLE)
            return "O-O-O";

        bool isCapture;
        Piece piece;
        int startSquare;
        int endSquare;
        u_int16_t flag;
        // We get this by passing everything in by reference to decomposeMove
        decomposeMove(move, piece, isCapture, startSquare, endSquare, flag);

        bool other_isCapture; // this is snamel case (cross of snake and camel). I don't see what's bad about that.
        Piece other_piece;
        int other_startSquare;
        int other_endSquare;
        u_int16_t other_flag;

        // The hard part: Find which other pieces could move here (There is more than one Rd1)
        bool mustSpecifyRank = false;
        bool mustSpecifyFile = false;
        bool mustSpecifySomething = false;

        vector<uint16_t> legalMoves;
        getLegalMoves(legalMoves);
        for (const uint16_t otherMove: legalMoves) {
            if (isCastle(otherMove))
                continue;
            if (otherMove == move)
                continue;
            decomposeMove(otherMove, other_piece, other_isCapture, other_startSquare, other_endSquare, other_flag);

            if (other_piece == piece and other_endSquare == endSquare and other_startSquare != startSquare) {
//                cout << otherMove << endl << move << endl;
                mustSpecifySomething = true;
                if (other_startSquare / 8 == startSquare / 8)
                    mustSpecifyRank = true;
                if (other_startSquare % 8 == startSquare % 8)
                    mustSpecifyFile = true;
            }
        }

        if (mustSpecifySomething and !mustSpecifyRank and !mustSpecifyFile)
            mustSpecifyFile = true;

        string prefix;
        if (piece == PAWN) {
            if (flag == EN_PASSANT_FLAG)
                isCapture = true;
            mustSpecifySomething = false;
            mustSpecifyRank = false;
            mustSpecifyFile = false;
            if (isCapture)
                prefix = string(1, 'a' + (startSquare / 8));
            else
                prefix = "";
        } else {
            if (piece == KING)
                prefix = "K";
            else if (piece == QUEEN)
                prefix = "Q";
            else if (piece == ROOK)
                prefix = "R";
            else if (piece == BISHOP)
                prefix = "B";
            else if (piece == KNIGHT)
                prefix = "N";
            else
                assert(false);

            if (mustSpecifyFile)
                prefix += string(1, 'a' + (startSquare / 8));
            if (mustSpecifyRank)
                prefix += string(1, '1' + (startSquare % 8));

        }

        // Now we add "x" if it's a capture

        if (isCapture)
            prefix += "x";

        // Now we add the ending square
        prefix += string(1, 'a' + (endSquare / 8));
        prefix += string(1, '1' + (endSquare % 8));

        // Now we add if it's a promotion

        if (flag == PROMOTE_TO_QUEEN_FLAG)
            prefix += "=Q";
        else if (flag == PROMOTE_TO_ROOK_FLAG)
            prefix += "=R";
        else if (flag == PROMOTE_TO_BISHOP_FLAG)
            prefix += "=B";
        else if (flag == PROMOTE_TO_KNIGHT_FLAG)
            prefix += "=N";

        bool isCheck = false;
        bool isCheckmate = false;

        ParameterChessBoard newBoard = *this;
        newBoard.makemove(move); // It had better not be illegal.

        isCheck = newBoard.getPieceGivingCheck() != NOT_IN_CHECK_CODE;
        isCheckmate = isCheck and (newBoard.isWhiteWonByCheckmate() or newBoard.isBlackWonByCheckmate());

        if (isCheckmate)
            prefix += "#";
        else if (isCheck)
            prefix += "+";

        return prefix;
    }

    Piece getMovingPiece(const uint16_t move) const {
        int startSquare = move >> 10;
        if (whiteKingPosition == startSquare or blackKingPosition == startSquare or move == WHITE_SHORT_CASTLE or
            move == BLACK_SHORT_CASTLE or move == WHITE_LONG_CASTLE or move == BLACK_LONG_CASTLE)
            return KING;

        if (isItWhiteToMove) {
            if ((whitePieceTypes[QUEEN_CODE] >> startSquare & 1ULL) == 1ULL)
                return QUEEN;
            if ((whitePieceTypes[ROOK_CODE] >> startSquare & 1ULL) == 1ULL)
                return ROOK;
            if ((whitePieceTypes[BISHOP_CODE] >> startSquare & 1ULL) == 1ULL)
                return BISHOP;
            if ((whitePieceTypes[KNIGHT_CODE] >> startSquare & 1ULL) == 1ULL)
                return KNIGHT;
            if ((whitePieceTypes[PAWN_CODE] >> startSquare & 1ULL) == 1ULL)
                return PAWN;
        } else {
            if ((blackPieceTypes[QUEEN_CODE] >> startSquare & 1ULL) == 1ULL)
                return QUEEN;
            if ((blackPieceTypes[ROOK_CODE] >> startSquare & 1ULL) == 1ULL)
                return ROOK;
            if ((blackPieceTypes[BISHOP_CODE] >> startSquare & 1ULL) == 1ULL)
                return BISHOP;
            if ((blackPieceTypes[KNIGHT_CODE] >> startSquare & 1ULL) == 1ULL)
                return KNIGHT;
            if ((blackPieceTypes[PAWN_CODE] >> startSquare & 1ULL) == 1ULL)
                return PAWN;
        }
        assert(false);
        // There are no pieces on the start square
    }

    uint16_t getMoveFromSAN(const string &SAN) const {
        if (SAN.find("O-O-O") != std::string::npos) {
            if (isItWhiteToMove)
                return WHITE_LONG_CASTLE;
            else
                return BLACK_LONG_CASTLE;
        } else if (SAN.find("O-O") != std::string::npos) {
            if (isItWhiteToMove)
                return WHITE_SHORT_CASTLE;
            else
                return BLACK_SHORT_CASTLE;
        }

        Piece piece;
        const char pieceInitial = SAN[0];
        switch (pieceInitial) {
            case 'K':
                piece = KING;
                break;
            case 'Q' :
                piece = QUEEN;
                break;
            case 'R':
                piece = ROOK;
                break;
            case 'B':
                piece = BISHOP;
                break;
            case 'N':
                piece = KNIGHT;
                break;
            case 'a':
                piece = PAWN;
                break;
            case 'b':
                piece = PAWN;
                break;
            case 'c':
                piece = PAWN;
                break;
            case 'd':
                piece = PAWN;
                break;
            case 'e':
                piece = PAWN;
                break;
            case 'f':
                piece = PAWN;
                break;
            case 'g':
                piece = PAWN;
                break;
            case 'h':
                piece = PAWN;
                break;
            default:
                cout << pieceInitial << endl;
                exit(65);
        }

        // Now we find the end square. It is always last, except for +, #, or =Q / =R / =B / =N
        // While we're doing this, we also find if it is a promotion

        Piece promotingPiece = KING; // this is the null piece. It is not used except if the move is actually a promotion

        unsigned int endSquareIndex = SAN.size() - 1;
        if (SAN[endSquareIndex] == '+' or SAN[endSquareIndex] == '#')
            endSquareIndex--;
        if (SAN[endSquareIndex - 1] == '=') {
            switch (SAN[endSquareIndex]) {
                case 'Q':
                    promotingPiece = QUEEN;
                    break;
                case 'R':
                    promotingPiece = ROOK;
                    break;
                case 'B':
                    promotingPiece = BISHOP;
                    break;
                case 'N':
                    promotingPiece = KNIGHT;
                    break;
                default:
                    assert(false);
            }
            endSquareIndex -= 2;
        }

        assert('1' <= SAN[endSquareIndex] and SAN[endSquareIndex] <= '8');
        assert('a' <= SAN[endSquareIndex - 1] and SAN[endSquareIndex - 1] <= 'h');
        int endRank = SAN[endSquareIndex] - '1';
        int endFile = SAN[endSquareIndex - 1] - 'a';

        int endSquare = 8 * endFile + endRank;

        if (promotingPiece != KING) {
            assert(piece == PAWN);
            assert(endRank == (isItWhiteToMove ? 7 : 0));
        }

        int startSquare = 64;
        bool isCapture = SAN.find('x') != std::string::npos;

        if (piece == KING) {
            startSquare = isItWhiteToMove ? whiteKingPosition : blackKingPosition;
        } else if (piece == PAWN) {
            unsigned long startSquareMask;
            bool isEnPassant = false;
            bool isDoublePawnPush = false;
            if (isCapture) {
                startSquareMask = isItWhiteToMove ? getMagicBlackAttackedSquares(PAWN_CODE, endSquare, 0)
                                                  : getMagicWhiteAttackedSquares(PAWN_CODE, endSquare, 0);
                startSquareMask &= A_FILE << (8 * (SAN[0] - 'a'));
                startSquare = LOG_2_TABLE.get(startSquareMask);
                assert(1ULL << startSquare == startSquareMask);
                if (isItWhiteToMove)
                    isEnPassant = endSquare == whichPawnMovedTwoSquares * 8 + 5;
                else
                    isEnPassant = endSquare == whichPawnMovedTwoSquares * 8 + 2;
            } else { // it is not a capture
                if (isItWhiteToMove) {
                    if ((whitePieceTypes[PAWN_CODE] >> (endSquare - 1) & 1ULL) == 1ULL)
                        startSquare = endSquare - 1;
                    else {
                        assert((allWhitePieces >> (endSquare - 1) & 1ULL) == 0ULL);
                        assert((whitePieceTypes[PAWN_CODE] >> (endSquare - 2) & 1ULL) == 1ULL);
                        startSquare = endSquare - 2;
                        isDoublePawnPush = true;
                    } // end if it isn't a pawn pushed by 1
                } // end if it is white to move
                else { // it is black to move
                    if ((blackPieceTypes[PAWN_CODE] >> (endSquare + 1) & 1ULL) == 1ULL)
                        startSquare = endSquare + 1;
                    else {
                        assert((allBlackPieces >> (endSquare + 1) & 1ULL) == 0ULL);
                        assert((blackPieceTypes[PAWN_CODE] >> (endSquare + 2) & 1ULL) == 1ULL);
                        startSquare = endSquare + 2;
                        isDoublePawnPush = true;
                    } // end if it isn't a pawn pushed by 1
                } // end if it is black to move
            } // end if it is not a capture
            if (isDoublePawnPush)
                return startSquare << 10 | endSquare << 4 | PAWN_PUSH_TWO_SQUARES_FLAG;
            else if (isEnPassant)
                return startSquare << 10 | endSquare << 4 | EN_PASSANT_FLAG;
            else if (promotingPiece == QUEEN)
                return startSquare << 10 | endSquare << 4 | PROMOTE_TO_QUEEN_FLAG;
            else if (promotingPiece == ROOK)
                return startSquare << 10 | endSquare << 4 | PROMOTE_TO_ROOK_FLAG;
            else if (promotingPiece == BISHOP)
                return startSquare << 10 | endSquare << 4 | PROMOTE_TO_BISHOP_FLAG;
            else if (promotingPiece == KNIGHT)
                return startSquare << 10 | endSquare << 4 | PROMOTE_TO_KNIGHT_FLAG;
        } // end if the piece is a pawn
        else { // the piece is one of QRBN
            vector<uint16_t> legalMoves;
            getLegalMoves(legalMoves);
            unsigned long possibleStartSquaresMask = 0;
            for (uint16_t legalMove: legalMoves) {
                if (getMovingPiece(legalMove) == piece and getEndSquare(legalMove) == endSquare)
                    possibleStartSquaresMask |= 1ULL << getStartSquare(legalMove);
            }

            // Now we have to deal with if there are other pieces that can go to the same end square
            // So we figure out which one of them is moving
            int rankFileSpecifierIndex = 1;
            while (!FastLogarithm::isSingleBit(possibleStartSquaresMask)) {
                if (rankFileSpecifierIndex >= 3) {
                    cout << toString() << endl << toFenNotation() << endl << SAN << endl;
                    cout << "rankFileSpecifierIndex is >= 3" << endl;
                    exit(1);
                }
                char specifier = SAN[rankFileSpecifierIndex];
                if ('a' <= specifier and specifier <= 'h') {
                    int file = specifier - 'a';
                    possibleStartSquaresMask &= A_FILE << 8 * file;
                } else if ('1' <= specifier and specifier <= '8') {
                    int rank = specifier - '1';
                    possibleStartSquaresMask &= FIRST_RANK << rank;
                }
                rankFileSpecifierIndex++;
            }

            startSquare = FastLogarithm::log2(possibleStartSquaresMask);
        } // end else (if the piece is one of QRBN)

        assert(startSquare != 64);
        assert(endSquare != 64);
        if (isCapture)
            return getCaptureMove(startSquare, endSquare);
        else
            return startSquare << 10 | endSquare << 4 | NORMAL_MOVE_FLAG;
    } // end getMoveFromSAN method

    void makeSANMove(const string &SAN) {
        uint16_t move = getMoveFromSAN(SAN);

        if (!isThisMoveLegal(move)) {
            cout << toString() << endl;
            cout << SAN << endl;
            cout << move << endl;
            cout << "This move was illegally attempted" << endl;
            exit(1);
        }

        makemove(move);
    }

    uint16_t getMoveFromPureAlgebraicNotation(const string &pureAlgebraicNotation) const {
        int startSquare;
        int endSquare;
        uint16_t flag;
        if (pureAlgebraicNotation.length() == 4) {
            int startFile = pureAlgebraicNotation[0] - 'a';
            int startRank = pureAlgebraicNotation[1] - '1';
            int endFile = pureAlgebraicNotation[2] - 'a';
            int endRank = pureAlgebraicNotation[3] - '1';

            for (int coordinate: {startFile, startRank, endFile, endRank}) {
                assert(0 <= coordinate and coordinate <= 7);
            }

            startSquare = 8 * startFile + startRank;
            endSquare = 8 * endFile + endRank;

            // handle en passant and castling
            const unsigned long allPawns = whitePieceTypes[PAWN_CODE] | blackPieceTypes[PAWN_CODE];
            const bool isAPawnMoving = (allPawns >> startSquare & 1ULL) == 1ULL;

            if (isAPawnMoving and isItWhiteToMove and endSquare == 8 * whichPawnMovedTwoSquares + 5)
                return (startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG;
            if (isAPawnMoving and !isItWhiteToMove and endSquare == 8 * whichPawnMovedTwoSquares + 2)
                return (startSquare << 10) + (endSquare << 4) + EN_PASSANT_FLAG;
            else if (isItWhiteToMove and pureAlgebraicNotation == "e1g1" and canWhiteCastleShort)
                return WHITE_SHORT_CASTLE;
            else if (isItWhiteToMove and pureAlgebraicNotation == "e1c1" and canWhiteCastleLong)
                return WHITE_LONG_CASTLE;
            else if (!isItWhiteToMove and pureAlgebraicNotation == "e8g8" and canBlackCastleShort)
                return BLACK_SHORT_CASTLE;
            else if (!isItWhiteToMove and pureAlgebraicNotation == "e8c8" and canBlackCastleLong)
                return BLACK_LONG_CASTLE;
            else {
                uint16_t baseMove = startSquare << 10 | endSquare << 4 | NORMAL_MOVE_FLAG;
                if (((allWhitePieces | allBlackPieces) >> endSquare & 1ULL) == 1ULL)
                    return getCaptureMove(startSquare, endSquare);
                else {
                    if (startSquare % 8 == 1 and endSquare % 8 == 3 and getMovingPiece(baseMove) == PAWN)
                        return startSquare << 10 | endSquare << 4 | PAWN_PUSH_TWO_SQUARES_FLAG;
                    else if (startSquare % 8 == 6 and endSquare % 8 == 4 and getMovingPiece(baseMove) == PAWN)
                        return startSquare << 10 | endSquare << 4 | PAWN_PUSH_TWO_SQUARES_FLAG;
                    else
                        return baseMove;
                }
            } // end else (the move is not castling
        } // end if string length is 4
        else if (pureAlgebraicNotation.length() == 5) {
            int startFile = pureAlgebraicNotation[0] - 'a';
            int startRank = pureAlgebraicNotation[1] - '1';
            int endFile = pureAlgebraicNotation[2] - 'a';
            int endRank = pureAlgebraicNotation[3] - '1';

            for (int coordinate: {startFile, startRank, endFile, endRank}) {
                assert(0 <= coordinate and coordinate <= 7);
            }

            startSquare = 8 * startFile + startRank;
            endSquare = 8 * endFile + endRank;

            switch (pureAlgebraicNotation[4]) {
                case 'q':
                    return (startSquare << 10) + (endSquare << 4) + PROMOTE_TO_QUEEN_FLAG;
                case 'r':
                    return (startSquare << 10) + (endSquare << 4) + PROMOTE_TO_ROOK_FLAG;
                case 'b':
                    return (startSquare << 10) + (endSquare << 4) + PROMOTE_TO_BISHOP_FLAG;
                case 'n':
                    return (startSquare << 10) + (endSquare << 4) + PROMOTE_TO_KNIGHT_FLAG;
                default:
                    assert(false);
            }
        } else {
            assert(false);
        }
    }

    bool makePureAlgebraicNotationMove(const string &pureAlgebraicNotation) {
        uint16_t move = getMoveFromPureAlgebraicNotation(pureAlgebraicNotation);
        if (!isThisMoveLegal(move))
            return false;
        makemove(move);
        return true;
    }

    bool operator==(const ParameterChessBoard &other) const {
        return
                whiteKingPosition == other.whiteKingPosition and
                whitePieceTypes[QUEEN_CODE] == other.whitePieceTypes[QUEEN_CODE] and
                whitePieceTypes[ROOK_CODE] == other.whitePieceTypes[ROOK_CODE] and
                whitePieceTypes[BISHOP_CODE] == other.whitePieceTypes[BISHOP_CODE] and
                whitePieceTypes[KNIGHT_CODE] == other.whitePieceTypes[KNIGHT_CODE] and
                whitePieceTypes[PAWN_CODE] == other.whitePieceTypes[PAWN_CODE] and

                blackKingPosition == other.blackKingPosition and
                blackPieceTypes[QUEEN_CODE] == other.blackPieceTypes[QUEEN_CODE] and
                blackPieceTypes[ROOK_CODE] == other.blackPieceTypes[ROOK_CODE] and
                blackPieceTypes[BISHOP_CODE] == other.blackPieceTypes[BISHOP_CODE] and
                blackPieceTypes[KNIGHT_CODE] == other.blackPieceTypes[KNIGHT_CODE] and
                blackPieceTypes[PAWN_CODE] == other.blackPieceTypes[PAWN_CODE] and

                canWhiteCastleShort == other.canWhiteCastleShort and
                canWhiteCastleLong == other.canWhiteCastleLong and
                canBlackCastleShort == other.canBlackCastleShort and
                canBlackCastleLong == other.canBlackCastleLong and

                whichPawnMovedTwoSquares == other.whichPawnMovedTwoSquares and

                isItWhiteToMove == other.isItWhiteToMove;
    } // end == operator

    bool isDrawByInsufficientMaterial() const {
        return drawByInsufficientMaterial;
    }

    bool isDrawByStalemate() const {
        return drawByStalemate;
    }

    bool isWhiteWonByCheckmate() const {
        return whiteWonByCheckmate;
    }

    bool isBlackWonByCheckmate() const {
        return blackWonByCheckmate;
    }

    bool isDrawBy50MoveRule() const {
        return halfmoveClock >= 100 and !isWhiteWonByCheckmate() and !isBlackWonByCheckmate();
    }

    bool hasGameEnded() const {
        return isDrawBy50MoveRule() or isDrawByInsufficientMaterial() or isDrawByStalemate() or
               isWhiteWonByCheckmate() or isBlackWonByCheckmate();
    }

    unsigned long getZobristCode() const {
        return zobristCode;
    }

    bool getIsItWhiteToMove() const {
        return isItWhiteToMove;
    }

    unsigned long getAllBlackPieces() const {
        return allBlackPieces;
    }

    uint8_t getPieceGivingCheck() const {
        return pieceGivingCheck;
    }
private:
    /*
     * Supplementary eval tuning functions
     */
    bool isThisWhitePawnIsolated (int square) const {
        return (whitePieceTypes[PAWN_CODE] & isolated_pawns::PAWN_NEIGHBOR_FILES[square >> 3]) == 0ULL;
    }
    bool isThisBlackPawnIsolated (int square) const {
        return (blackPieceTypes[PAWN_CODE] & isolated_pawns::PAWN_NEIGHBOR_FILES[square >> 3]) == 0ULL;
    }

    bool isThisWhitePawnDoubled (int square) const {
        return (~(1ULL << square) & whitePieceTypes[PAWN_CODE] & 255ULL << (square & 56)) != 0ULL;
    }
    bool isThisBlackPawnDoubled (int square) const {
        return (~(1ULL << square) & blackPieceTypes[PAWN_CODE] & 255ULL << (square & 56)) != 0ULL;
    }
    
    bool isThisWhitePawnPassed (int square) const {
        return (blackPieceTypes[PAWN_CODE] & passed_pawns::getWhitePawnStoppingSquares(square)) == 0ULL;
    }
    bool isThisBlackPawnPassed (int square) const {
        return (whitePieceTypes[PAWN_CODE] & passed_pawns::getBlackPawnStoppingSquares(square)) == 0ULL;
    }

public:
    /*******************************************************
     * These are all the functions we use to tune the eval *
     ******************************************************/

    int getWhiteKingOnSquareCount (const int square) const {
        return whiteKingPosition == square;
    }
    int getWhitePieceTypeCount (const int pieceType) {
        return __builtin_popcountll(whitePieceTypes[pieceType]);
    }
    int getWhitePieceTypeOnSquareCount (const int pieceType, const int square) const {
        return int(whitePieceTypes[pieceType] >> square) & 1;
    }
    // Square is mirrored
    int getBlackKingOnSquareCount (const int square) const {
        return blackKingPosition == (square ^ 7);
    }
    int getBlackPieceTypeCount (const int pieceType) {
        return __builtin_popcountll(blackPieceTypes[pieceType]);
    }
    // Square is mirrored
    int getBlackPieceTypeOnSquareCount (const int pieceType, const int square) const {
        return int(blackPieceTypes[pieceType] >> (square ^ 7)) & 1;
    }
    
    int getWhitePieceTypeMobility (const int pieceType) const {
        const unsigned long allPieces = allWhitePieces | allBlackPieces;
        int mobility = 0;
        unsigned long piecesRemaining = whitePieceTypes[pieceType];
        unsigned long thisPieceMask;
        int thisPieceSquare;
        while (piecesRemaining != 0ULL) {
            thisPieceMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= thisPieceMask;
            thisPieceSquare = FastLogarithm::log2(thisPieceMask);
            mobility += __builtin_popcountll(getMagicWhiteAttackedSquares(pieceType,thisPieceSquare,allPieces) & ~allWhitePieces);
        }
        return mobility;
    }
    
    int getBlackPieceTypeMobility (const int pieceType) const {
        const unsigned long allPieces = allWhitePieces | allBlackPieces;
        int mobility = 0;
        unsigned long piecesRemaining = blackPieceTypes[pieceType];
        unsigned long thisPieceMask;
        int thisPieceSquare;
        while (piecesRemaining != 0ULL) {
            thisPieceMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= thisPieceMask;
            thisPieceSquare = FastLogarithm::log2(thisPieceMask);
            mobility += __builtin_popcountll(getMagicBlackAttackedSquares(pieceType,thisPieceSquare,allPieces) & ~allBlackPieces);
        }

        return mobility;
    }

    int getWhitePieceTypeKingAttackZone (const int pieceType) const {
        const unsigned long allPieces = allWhitePieces | allBlackPieces;
        const unsigned long kingZone = getMagicKingAttackedSquares(blackKingPosition) | 1ULL << blackKingPosition;
        int kingAttackCount = 0;
        unsigned long piecesRemaining = whitePieceTypes[pieceType];
        unsigned long thisPieceMask;
        int thisPieceSquare;
        while (piecesRemaining != 0ULL) {
            thisPieceMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= thisPieceMask;
            thisPieceSquare = FastLogarithm::log2(thisPieceMask);
            kingAttackCount += __builtin_popcountll(getMagicWhiteAttackedSquares(pieceType,thisPieceSquare,allPieces) & kingZone);
        }
        return kingAttackCount;
    }

    int getBlackPieceTypeKingAttackZone (const int pieceType) const {
        const unsigned long allPieces = allWhitePieces | allBlackPieces;
        const unsigned long kingZone = getMagicKingAttackedSquares(whiteKingPosition) | 1ULL << whiteKingPosition;
        int kingAttackCount = 0;
        unsigned long piecesRemaining = blackPieceTypes[pieceType];
        unsigned long thisPieceMask;
        int thisPieceSquare;
        while (piecesRemaining != 0ULL) {
            thisPieceMask = piecesRemaining & -piecesRemaining;
            piecesRemaining -= thisPieceMask;
            thisPieceSquare = FastLogarithm::log2(thisPieceMask);
            kingAttackCount += __builtin_popcountll(getMagicBlackAttackedSquares(pieceType,thisPieceSquare,allPieces) & kingZone);
        }
        return kingAttackCount;
    }

    int getWhitePieceShieldCount () const {
        return __builtin_popcountll(kingsafety::WHITE_SHIELD_ZONES[whiteKingPosition] & allWhitePieces) - 1; // we subtract 1 because the king doesn't count as shielding itself
    }
    int getBlackPieceShieldCount () const {
        return __builtin_popcountll(kingsafety::BLACK_SHIELD_ZONES[blackKingPosition] & allBlackPieces) - 1; // we subtract 1 because the king doesn't count as shielding itself
    }

    int getWhiteIsolatedPawnCount () const {
        int isolatedPawnCount = 0;
        unsigned long whitePawnsRemaining = whitePieceTypes[PAWN_CODE];
        unsigned long thisWhitePawnMask;
        while (whitePawnsRemaining != 0ULL) {
            thisWhitePawnMask = whitePawnsRemaining & -whitePawnsRemaining;
            whitePawnsRemaining -= thisWhitePawnMask;
            isolatedPawnCount += isThisWhitePawnIsolated(FastLogarithm::log2(thisWhitePawnMask));
        }
        return isolatedPawnCount;
    }
    int getBlackIsolatedPawnCount () const {
        int isolatedPawnCount = 0;
        unsigned long blackPawnsRemaining = blackPieceTypes[PAWN_CODE];
        unsigned long thisBlackPawnMask;
        while (blackPawnsRemaining != 0ULL) {
            thisBlackPawnMask = blackPawnsRemaining & -blackPawnsRemaining;
            blackPawnsRemaining -= thisBlackPawnMask;
            isolatedPawnCount += isThisBlackPawnIsolated(FastLogarithm::log2(thisBlackPawnMask));
        }
        return isolatedPawnCount;
    }

    int getWhiteDoubledPawnCount () const {
        int doubledPawnCount = 0;
        unsigned long whitePawnsRemaining = whitePieceTypes[PAWN_CODE];
        unsigned long thisWhitePawnMask;
        while (whitePawnsRemaining != 0ULL) {
            thisWhitePawnMask = whitePawnsRemaining & -whitePawnsRemaining;
            whitePawnsRemaining -= thisWhitePawnMask;
            doubledPawnCount += isThisWhitePawnDoubled(FastLogarithm::log2(thisWhitePawnMask));
        }
        return doubledPawnCount;
    }
    int getBlackDoubledPawnCount () const {
        int doubledPawnCount = 0;
        unsigned long blackPawnsRemaining = blackPieceTypes[PAWN_CODE];
        unsigned long thisBlackPawnMask;
        while (blackPawnsRemaining != 0ULL) {
            thisBlackPawnMask = blackPawnsRemaining & -blackPawnsRemaining;
            blackPawnsRemaining -= thisBlackPawnMask;
            doubledPawnCount += isThisBlackPawnDoubled(FastLogarithm::log2(thisBlackPawnMask));
        }
        return doubledPawnCount;
    }

    int getWhitePassedPawnCount () const {
        int passedPawnCount = 0;
        unsigned long whitePawnsRemaining = whitePieceTypes[PAWN_CODE];
        unsigned long thisWhitePawnMask;
        while (whitePawnsRemaining != 0ULL) {
            thisWhitePawnMask = whitePawnsRemaining & -whitePawnsRemaining;
            whitePawnsRemaining -= thisWhitePawnMask;
            passedPawnCount += isThisWhitePawnPassed(FastLogarithm::log2(thisWhitePawnMask));
        }
        return passedPawnCount;
    }
    int getBlackPassedPawnCount () const {
        int passedPawnCount = 0;
        unsigned long blackPawnsRemaining = blackPieceTypes[PAWN_CODE];
        unsigned long thisBlackPawnMask;
        while (blackPawnsRemaining != 0ULL) {
            thisBlackPawnMask = blackPawnsRemaining & -blackPawnsRemaining;
            blackPawnsRemaining -= thisBlackPawnMask;
            passedPawnCount += isThisBlackPawnPassed(FastLogarithm::log2(thisBlackPawnMask));
        }
        return passedPawnCount;
    }

    int getWhitePassedPawnCountOnRank (int rank) const {
        int passedPawnCount = 0;
        unsigned long whitePawnsRemaining = whitePieceTypes[PAWN_CODE];
        unsigned long thisWhitePawnMask;
        while (whitePawnsRemaining != 0ULL) {
            thisWhitePawnMask = whitePawnsRemaining & -whitePawnsRemaining;
            whitePawnsRemaining -= thisWhitePawnMask;
            passedPawnCount += (FIRST_RANK << rank & thisWhitePawnMask) != 0ULL and isThisWhitePawnPassed(FastLogarithm::log2(thisWhitePawnMask));
        }
        return passedPawnCount;
    }

    int getWhitePassedPawnCountOnFile (int file) const {
        int passedPawnCount = 0;
        unsigned long whitePawnsRemaining = whitePieceTypes[PAWN_CODE];
        unsigned long thisWhitePawnMask;
        while (whitePawnsRemaining != 0ULL) {
            thisWhitePawnMask = whitePawnsRemaining & -whitePawnsRemaining;
            whitePawnsRemaining -= thisWhitePawnMask;
            passedPawnCount += (A_FILE << 8 * file & thisWhitePawnMask) != 0ULL and isThisWhitePawnPassed(FastLogarithm::log2(thisWhitePawnMask));
        }
        return passedPawnCount;
    }

    int getBlackPassedPawnCountOnRank (int rank) const {
        rank ^= 7; // we flip the ranks for black

        int passedPawnCount = 0;
        unsigned long blackPawnsRemaining = blackPieceTypes[PAWN_CODE];
        unsigned long thisblackPawnMask;
        while (blackPawnsRemaining != 0ULL) {
            thisblackPawnMask = blackPawnsRemaining & -blackPawnsRemaining;
            blackPawnsRemaining -= thisblackPawnMask;
            passedPawnCount += (FIRST_RANK << rank & thisblackPawnMask) != 0ULL and isThisBlackPawnPassed(FastLogarithm::log2(thisblackPawnMask));
        }
        return passedPawnCount;
    }

    int getBlackPassedPawnCountOnFile (int file) const {
        int passedPawnCount = 0;
        unsigned long blackPawnsRemaining = blackPieceTypes[PAWN_CODE];
        unsigned long thisblackPawnMask;
        while (blackPawnsRemaining != 0ULL) {
            thisblackPawnMask = blackPawnsRemaining & -blackPawnsRemaining;
            blackPawnsRemaining -= thisblackPawnMask;
            passedPawnCount += (A_FILE << 8 * file & thisblackPawnMask) != 0ULL and isThisBlackPawnPassed(FastLogarithm::log2(thisblackPawnMask));
        }
        return passedPawnCount;
    }

    bool doesWhiteHaveBishopPair () const {
        return __builtin_popcountll(whitePieceTypes[BISHOP_CODE]) >= 2;
    }
    bool doesBlackHaveBishopPair () const {
        return __builtin_popcountll(blackPieceTypes[BISHOP_CODE]) >= 2;
    }

    int getWhiteProtectedPassedPawnCount () const {
        int passedPawnCount = 0;
        unsigned long whitePawnsRemaining = whitePieceTypes[PAWN_CODE];
        unsigned long thisWhitePawnMask;
        while (whitePawnsRemaining != 0ULL) {
            thisWhitePawnMask = whitePawnsRemaining & -whitePawnsRemaining;
            whitePawnsRemaining -= thisWhitePawnMask;
            if (getMagicBlackAttackedSquares(PAWN_CODE,FastLogarithm::log2(thisWhitePawnMask),0) & whitePieceTypes[PAWN_CODE])
                passedPawnCount += isThisWhitePawnPassed(FastLogarithm::log2(thisWhitePawnMask));
        }
        return passedPawnCount;
    }
    int getBlackProtectedPassedPawnCount () const {
        int passedPawnCount = 0;
        unsigned long blackPawnsRemaining = blackPieceTypes[PAWN_CODE];
        unsigned long thisBlackPawnMask;
        while (blackPawnsRemaining != 0ULL) {
            thisBlackPawnMask = blackPawnsRemaining & -blackPawnsRemaining;
            blackPawnsRemaining -= thisBlackPawnMask;
            if (getMagicWhiteAttackedSquares(PAWN_CODE,FastLogarithm::log2(thisBlackPawnMask),0) & blackPieceTypes[PAWN_CODE])
                passedPawnCount += isThisBlackPawnPassed(FastLogarithm::log2(thisBlackPawnMask));
        }
        return passedPawnCount;
    }

    vector<int16_t> getCoefficients () const {
        // Bishop pair diff (1)
        // Passed pawn on rank diff (8)
        // Passed pawn on file diff (8)
        // Protected passed pawn count diff (1)
        //// Pawn outside square diff (1)

        // is it white to move (1)
        //// passed pawn count diff (1)
        // doubled pawn count diff (1)
        // isolated pawn count diff (1)
        // king shelter diff (1)
        // king zone piece type attack diffs (5)
        // mobility diffs (5)
        // king position diffs (64)
        // pieces on squares diffs (320)

        vector<int16_t> diffs;
        // Bishop pair
        diffs.push_back(doesWhiteHaveBishopPair() - doesBlackHaveBishopPair());
        // Passed pawn on rank
        for (int rank = 0; rank < 8; rank++) {
            diffs.push_back(getWhitePassedPawnCountOnRank(rank) - getBlackPassedPawnCountOnRank(rank));
        }
        // Passed pawn on file
        for (int file = 0; file < 8; file++) {
            diffs.push_back(getWhitePassedPawnCountOnFile(file) - getBlackPassedPawnCountOnFile(file));
        }

        // Protected passed pawn count
        diffs.push_back(getWhiteProtectedPassedPawnCount() - getBlackProtectedPassedPawnCount());

        // Is it white to move
        diffs.push_back(isItWhiteToMove ? 1 : 0);
        // Pawn structure
        //diffs.push_back(getWhitePassedPawnCount() - getBlackPassedPawnCount());
        diffs.push_back(getWhiteDoubledPawnCount() - getBlackDoubledPawnCount());
        diffs.push_back(getWhiteIsolatedPawnCount() - getBlackIsolatedPawnCount());
        // Pawn shield
        diffs.push_back(getWhitePieceShieldCount() - getBlackPieceShieldCount());
        // Attacking king zone
        for (int pieceType = QUEEN_CODE; pieceType <= PAWN_CODE; pieceType++) {
            diffs.push_back(getWhitePieceTypeKingAttackZone(pieceType) - getBlackPieceTypeKingAttackZone(pieceType));
        }
        // Mobility
        for (int pieceType = QUEEN_CODE; pieceType <= PAWN_CODE; pieceType++) {
            diffs.push_back(getWhitePieceTypeMobility(pieceType) - getBlackPieceTypeMobility(pieceType));
        }

        // king piece square tables
        for (int square = 0; square < 64; square++) {
            diffs.push_back(getWhiteKingOnSquareCount(square) - getBlackKingOnSquareCount(square));
        }

        // non-king piece square tables
        for (int pieceType = QUEEN_CODE; pieceType <= PAWN_CODE; pieceType++) {
            for (int square = 0; square < 64; square++) {
                diffs.push_back(getWhitePieceTypeOnSquareCount(pieceType,square) - getBlackPieceTypeOnSquareCount(pieceType,square));
            }
        }
        return diffs;
    }
};

#endif //AMETHYST_CHESS_PARAMETERCHESSBOARD_H
