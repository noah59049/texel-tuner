//
// Created by Noah Holbrook on 3/14/24.
//

#include "amethyst_tapered.h"
#include "../amethyst_external/ParameterChessBoard.h"

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
    assert(result.coefficients.size() == 416);
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

    // Bishop pair
    initialParameters.push_back({50,50});

    // Passed pawn on rank and file
    for (int rank = 0; rank < 8; rank++) {
        initialParameters.push_back({25,25});
    }
    for (int file = 0; file < 8; file++) {
        initialParameters.push_back({25,25});
    }

    // Protected passed pawn
    initialParameters.push_back({40,40});

    // is it white to move
    initialParameters.push_back({20,20});
    //// passed pawns
    //initialParameters.push_back({50,50});
    // doubled pawns
    initialParameters.push_back({-30,-30});
    // isolated pawns
    initialParameters.push_back({-30,-30});
    // king shelter
    initialParameters.push_back({10,10});
    // king zone attack
    for (int i = 0; i < 5; i++) {
        initialParameters.push_back({20,20});
    }
    // mobility
    for (int i = 0; i < 5; i++) {
        initialParameters.push_back({5,5});
    }
    // king piece squares
    for (tune_t pstValue : {0,900,500,300,300,100}) {
        for (int square = 0; square < 64; square++) {
            initialParameters.push_back({pstValue,pstValue});
        }
    }
    return initialParameters;
}

static void print_parameter(std::stringstream& ss, const pair_t parameter)
{
    ss << "S(" << parameter[static_cast<int32_t>(PhaseStages::Midgame)] << ", " << parameter[static_cast<int32_t>(PhaseStages::Endgame)] << ")";
}

static void print_single(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name)
{
    ss << "constexpr int " << name << " = ";
    print_parameter(ss, parameters[index]);
    ss << ";" << endl;
    index++;
}

static void print_array(std::stringstream& ss, const parameters_t& parameters, int& index, const std::string& name, int count)
{
    ss << "constexpr int " << name << "[] = {";
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

void AmethystEvalTapered::print_parameters(const parameters_t& parameters)
{
    int index = 0;
    stringstream ss;
    print_single(ss, parameters,index, "bishop_pair");
    print_array(ss, parameters, index, "passed_pawn_on_ranks", 8);
    print_array(ss, parameters, index, "passed_pawn_on_file", 8);
    print_single(ss, parameters, index, "protected_passed_pawn");
    print_single(ss, parameters, index, "white_to_move");
//    print_single(ss, parameters, index, "passed_pawn");
    print_single(ss, parameters, index, "doubled_pawn");
    print_single(ss, parameters, index, "isolated_pawn");
    print_single(ss, parameters, index, "king_shelter");
    print_array(ss, parameters, index, "king_zone_attacks", 5);
    print_array(ss, parameters, index, "mobility", 5);
    print_array(ss, parameters, index, "king_psts", 64);
    print_array(ss, parameters, index, "queen_psts", 64);
    print_array(ss, parameters, index, "rook_psts", 64);
    print_array(ss, parameters, index, "bishop_psts", 64);
    print_array(ss, parameters, index, "knight_psts", 64);
    print_array(ss, parameters, index, "pawn_psts", 64);

    cout << ss.str() << "\n";
}
#endif