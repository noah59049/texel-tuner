//
// Created by Noah Holbrook on 12/17/23.
//

#ifndef AMETHYST_CHESS_RANKFILEBITMASKS_H
#define AMETHYST_CHESS_RANKFILEBITMASKS_H

//using namespace std;

constexpr const static unsigned long INNER_RANKS = 0x7e7e7e7e7e7e7e7eULL;
constexpr const static unsigned long INNER_FILES = 0x00ffffffffffff00ULL;
constexpr const static unsigned long INNER_36    = 0x007e7e7e7e7e7e00ULL;

constexpr const static unsigned long A_FILE = 255ULL;
constexpr const static unsigned long H_FILE = 0xff00000000000000ULL;
constexpr const static unsigned long FIRST_RANK   = 0x0101010101010101ULL;
constexpr const static unsigned long SECOND_RANK  = 0x0202020202020202ULL;
constexpr const static unsigned long SEVENTH_RANK = 0x4040404040404040ULL;
constexpr const static unsigned long EIGHTH_RANK  = 0x8080808080808080ULL;

constexpr const static unsigned long NOT_A_FILE = ~A_FILE;
constexpr const static unsigned long NOT_H_FILE = ~H_FILE;

constexpr const static unsigned long E1_F1_G1 = 0x0001010100000000ULL;
constexpr const static unsigned long E1_D1_C1 = 0x0000000101010000ULL;
constexpr const static unsigned long E8_F8_G8 = 0x0080808000000000ULL;
constexpr const static unsigned long E8_D8_C8 = 0x0000008080800000ULL;

constexpr const static unsigned long E1_H1 = 0x0100000100000000ULL;
constexpr const static unsigned long E1_A1 = 0x0000000100000001ULL;
constexpr const static unsigned long E1_THROUGH_H1 = 0x0101010100000000ULL;
constexpr const static unsigned long E1_THROUGH_A1 = 0x0000000101010101ULL;
constexpr const static unsigned long E8_H8 = 0x8000008000000000ULL;
constexpr const static unsigned long E8_A8 = 0x0000008000000080ULL;
constexpr const static unsigned long E8_THROUGH_H8 = 0x8080808000000000ULL;
constexpr const static unsigned long E8_THROUGH_A8 = 0x0000008080808080ULL;

constexpr const static unsigned long LIGHT_SQUARES = 0x55AA55AA55AA55AAULL;
constexpr const static unsigned long DARK_SQUARES = 0xAA55AA55AA55AA55ULL;

constexpr const static unsigned long ENTIRE_BOARD = LIGHT_SQUARES | DARK_SQUARES;

//void printSquareMask (int specialSquare, unsigned long squares) {
//    string boardString;
//    char piece;
//    for (int rank = 7; rank >= 0; rank--) {
//        string row = "+---+---+---+---+---+---+---+---+\n|";
//        for (int file = 0; file < 8; file++) {
//            int square = 8 * file + rank;
//            if (square == specialSquare)
//                piece = '*';
//            else if (((squares >> square) & 1ULL) == 1ULL)
//                piece = '%';
//            else
//                piece = ' ';
//            row += " " + string(1,piece) + " |";
//        }
//        row += " " + to_string(rank + 1) + "\n";
//        boardString += row;
//    }
//    boardString += "+-a-+-b-+-c-+-d-+-e-+-f-+-g-+-h-+";
//    cout << boardString << endl;
//}

//void printSquareMask (unsigned long squares) {
//    printSquareMask(-1,squares);
//}

#endif //AMETHYST_CHESS_RANKFILEBITMASKS_H
