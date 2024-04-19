//
// Created by Noah Holbrook on 3/24/24.
//

#ifndef TUNER_AMETHYST_CONFIG_H
#define TUNER_AMETHYST_CONFIG_H
constexpr const static bool includeHeavisideKingZoneAttacks = false;
constexpr const static bool includeRooksOpenFiles = true;
constexpr const static bool includeBishopPair = true;
constexpr const static bool includePassedPawnRanks = true;
constexpr const static bool includePassedPawnFiles = true;
constexpr const static bool includeProtectedPassedPawn = false;
constexpr const static bool includeWhiteToMove = false; // -11 Elo
constexpr const static bool includePassedPawn = false; // redundant
constexpr const static bool includeDoubledPawn = false; // -1 Elo
constexpr const static bool includeIsolatedPawn = true;
constexpr const static bool includeKingShelter = true;
constexpr const static bool includeKingZoneAttacks = true;
constexpr const static bool includeMobility = true;
constexpr const static bool includePSTs = true;

// If you add new terms, you need to include them in 3 places
// ParameterChessBoard coefficients
// amethyst_tapered get_initial_parameters
// amethyst_tapered print_parameters

// I think that should be enough

#endif //TUNER_AMETHYST_CONFIG_H
