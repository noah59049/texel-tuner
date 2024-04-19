//
// Created by Noah Holbrook on 3/14/24.
//

#include "amethyst_tapered.h"
#include "../amethyst_external/ParameterChessBoard.h"
#include "amethyst_config.h"
#include <cmath>

//using coefficients_t = std::vector<int16_t>;
// struct EvalResult
//{
//    coefficients_t coefficients;
//    tune_t score;
//    tune_t endgame_scale = 1;
//};

using namespace Toy;

#if TAPERED

EvalResult AmethystEvalTapered::get_fen_eval_result(const std::string& fen) {
    ParameterChessBoard board = ParameterChessBoard::boardFromFENNotation(fen);
    EvalResult result;
    result.coefficients = board.getCoefficients();
//    assert(result.coefficients.size() == 416);
    result.score = 0;
    return result;
}
EvalResult AmethystEvalTapered::get_external_eval_result(const chess::Board &board) {
    assert(false); // This is not supported
}

parameters_t AmethystEvalTapered::get_initial_parameters()
{
    parameters_t initialParameters;
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

    // king zone attack
    if constexpr (includeHeavisideKingZoneAttacks) {
        for (int i = 0; i < 5; i++) {
            initialParameters.push_back({20, 20});
        }
    }
    // Rooks on open files
    if constexpr (includeRooksOpenFiles)
        initialParameters.push_back({20,20});
    // Bishop pair
    if constexpr (includeBishopPair)
        initialParameters.push_back({50,50});

    // Passed pawn on rank and file
    if constexpr (includePassedPawnRanks) {
        for (int rank = 0; rank < 8; rank++) {
            initialParameters.push_back({25, 25});
        }
    }
    if constexpr (includePassedPawnFiles) {
        for (int file = 0; file < 8; file++) {
            initialParameters.push_back({25, 25});
        }
    }

    // Protected passed pawn
    if constexpr (includeProtectedPassedPawn)
    initialParameters.push_back({40,40});

    // is it white to move
    if constexpr (includeWhiteToMove)
        initialParameters.push_back({20,20});
    // passed pawns
    if constexpr (includePassedPawn)
        initialParameters.push_back({50,50});
    // doubled pawns
    if constexpr (includeDoubledPawn)
        initialParameters.push_back({-30,-30});
    // isolated pawns
    if constexpr (includeIsolatedPawn)
        initialParameters.push_back({-30,-30});
    // king shelter
    if constexpr (includeKingShelter)
        initialParameters.push_back({10,10});
    // king zone attack
    if constexpr (includeKingZoneAttacks) {
        for (int i = 0; i < 5; i++) {
            initialParameters.push_back({20, 20});
        }
    }
    // mobility
    if constexpr (includeMobility) {
        for (int i = 0; i < 5; i++) {
            initialParameters.push_back({5, 5});
        }
    }
    // PSTs
    if constexpr (includePSTs) {
        for (tune_t pstValue : {0,900,500,300,300,100}) {
            for (int square = 0; square < 64; square++) {
                initialParameters.push_back({pstValue,pstValue});
            }
        }
    }
    return initialParameters;
}

static void print_parameter(std::stringstream& ss, const pair_t parameter)
{
//    ss << "S(" << parameter[static_cast<int32_t>(PhaseStages::Midgame)] << ", " << parameter[static_cast<int32_t>(PhaseStages::Endgame)] << ")";
    ss << (uint64_t(uint16_t(int16_t(lround(parameter[static_cast<int32_t>(PhaseStages::Midgame)])))) << 32 | uint64_t(uint16_t(int16_t(lround(parameter[static_cast<int32_t>(PhaseStages::Endgame)]))))) << "ULL";
}

static void print_single(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name)
{
    ss << "constexpr uint64_t " << name << " = ";
    print_parameter(ss, parameters[index]);
    ss << ";" << endl;
    index++;
}

static void print_array(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name, int count)
{
    ss << "constexpr uint64_t " << name << "[] = {";
    for (auto i = 0; i < count; i++)
    {
        print_parameter(ss, parameters[index]);
        index++;

        if (i != count - 1)
        {
            ss << ", ";
        }
    }
    ss << "};" << endl;
}

static void print_array_2d(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name, int count1, int count2)
{
    ss << "constexpr uint64_t " << name << "[][" << count2 << "] = {\n";
    for (auto i = 0; i < count1; i++)
    {
        ss << "    {";
        for (auto j = 0; j < count2; j++)
        {
            print_parameter(ss, parameters[index]);
            index++;

            if (j != count2 - 1)
            {
                ss << ", ";
            }
        }
        ss << "},\n";
    }
    ss << "};\n";
}

void AmethystEvalTapered::print_parameters(const parameters_t& parameters)
{
//    print2_parameters(parameters);

    int index = 0;
    stringstream ss;
    if constexpr (includeRooksOpenFiles)
        print_single(ss,parameters,index,"rooks_open_files");
    if constexpr (includeBishopPair)
        print_single(ss, parameters,index, "bishop_pair");
    if constexpr (includePassedPawnRanks)
        print_array(ss, parameters, index, "passed_pawn_on_ranks", 8);
    if constexpr (includePassedPawnFiles)
        print_array(ss, parameters, index, "passed_pawn_on_file", 8);
    if constexpr (includeProtectedPassedPawn)
        print_single(ss, parameters, index, "protected_passed_pawn");
    if constexpr (includeWhiteToMove)
        print_single(ss, parameters, index, "white_to_move");
    if constexpr (includePassedPawn)
        print_single(ss, parameters, index, "passed_pawn");
    if constexpr (includeDoubledPawn)
        print_single(ss, parameters, index, "doubled_pawn");
    if constexpr (includeIsolatedPawn)
        print_single(ss, parameters, index, "isolated_pawn");
    if constexpr (includeKingShelter)
        print_single(ss, parameters, index, "king_shelter");
    if constexpr (includeKingZoneAttacks)
        print_array(ss, parameters, index, "king_zone_attacks", 5);
    if constexpr (includeMobility)
        print_array(ss, parameters, index, "mobility", 5);
    if constexpr (includePSTs) {
        print_array(ss, parameters, index, "king_psts", 64);
        print_array_2d(ss, parameters, index, "piece_type_psts", 5, 64);
//        print_array(ss, parameters, index, "queen_psts", 64);
//        print_array(ss, parameters, index, "rook_psts", 64);
//        print_array(ss, parameters, index, "bishop_psts", 64);
//        print_array(ss, parameters, index, "knight_psts", 64);
//        print_array(ss, parameters, index, "pawn_psts", 64);
    }
    cout << ss.str() << "\n";
}
#endif