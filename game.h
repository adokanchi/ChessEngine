#pragma once

#include <cstdint>
#include <string>

enum class PieceType {
    Pawn = 0,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    None = -1
};
constexpr uint16_t INVALID_MOVE = 0b1000000000000000;
constexpr int NUM_PIECE_TYPES = 6;
constexpr int CASTLE_WK = 1 << 0;
constexpr int CASTLE_WQ = 1 << 1;
constexpr int CASTLE_BK = 1 << 2;
constexpr int CASTLE_BQ = 1 << 3;

struct boardState {
    // true if white's turn, false if black's turn
    bool isWhiteTurn = false;
    // moveCounter = moves since lsat 50-move-rule reset
    int moveCounter = 0;
    // castling represents whether castling would be legal for each side (changes when kings/rooks are moved)
    int castling = 0;
    // The square that en passant is available on (-1 if not available, 0-63 if available)
    int epSquare = 0;
};
struct BitBoards {
    uint64_t wPawns;
    uint64_t bPawns;
    uint64_t wKnights;
    uint64_t bKnights;
    uint64_t wRooks;
    uint64_t bRooks;
    uint64_t wBishops;
    uint64_t bBishops;
    uint64_t wQueens;
    uint64_t bQueens;
    uint64_t wKing;
    uint64_t bKing;
    uint64_t wPieces;
    uint64_t bPieces;

    void removePieceAtSquare(int square);
};

struct FileMasks {
    uint64_t A_FILE;
    uint64_t B_FILE;
    uint64_t C_FILE;
    uint64_t D_FILE;
    uint64_t E_FILE;
    uint64_t F_FILE;
    uint64_t G_FILE;
    uint64_t H_FILE;
};
extern FileMasks files;

struct RankMasks {
    uint64_t FIRST_RANK;
    uint64_t SECOND_RANK;
    uint64_t THIRD_RANK;
    uint64_t FOURTH_RANK;
    uint64_t FIFTH_RANK;
    uint64_t SIXTH_RANK;
    uint64_t SEVENTH_RANK;
    uint64_t EIGHTH_RANK;
};
extern RankMasks ranks;

struct GameData{
    boardState state;
    BitBoards boards;
};
extern GameData game;

// ~~~~~~~~~~~~~~~~ Board Setup and Game Cycle Section ~~~~~~~~~~~~~~~~

namespace MoveTables {
    extern uint64_t knightMoves[64];
    extern uint64_t kingMoves[64];
    extern uint64_t rookMoves[64][4096];
    extern uint64_t bishopMoves[64][512];

    void initKnightMoves();
    void initKingMoves();

    int getBlockerIndex(uint64_t mask, uint64_t blockers);
    uint64_t setBlockersFromIndex(uint64_t mask, int index);

    // ~~~~~~~~~~~~~~~~ Rook move generation section ~~~~~~~~~~~~~~~~

    uint64_t generateRookBlockerMask(int square);
    uint64_t computeRookAttacks(int square, uint64_t blockers);
    void initRookMoves();

    // ~~~~~~~~~~~~~~~~ Bishop move generation section ~~~~~~~~~~~~~~~~

    uint64_t generateBishopBlockerMask(int square);
    uint64_t computeBishopAttacks(int square, uint64_t blockers);
    void initBishopMoves();

    void init();

}

void setupFiles();
void setupRanks();
void setupStartingPosition();
void setupState();
void setup();
void printBoard();
[[noreturn]] void runInConsole();

// ~~~~~~~~~~~~~~~~ Move Calculating/Parsing section ~~~~~~~~~~~~~~~~

bool isValidMove(uint16_t nextMove, bool isWhiteTurn);
bool followsPieceMovementRules(PieceType pieceType, uint16_t nextMove, bool isWhiteTurn);
bool move(uint16_t nextMove, bool isWhiteTurn);
void updateCastlingRights(GameData& g, uint64_t nextMove);
char getPieceAt(int square);
int coordsToNum(const std::string& input);
std::string squareName(int square);
uint16_t parseAlgebraicMove(std::string input, bool isWhiteTurn);
bool isSquareAttacked(int square, bool byWhite, const BitBoards& b);
bool isMoveLegal(uint16_t move, bool isWhiteTurn);
void makeMove(GameData& g, uint16_t move, bool isWhiteTurn);
bool hasLegalMoves(bool isWhiteTurn);
bool isCheckmate(bool isWhiteTurn);
bool isStalemate(bool isWhiteTurn);

// ~~~~~~~~~~~~~~~~ Utility section ~~~~~~~~~~~~~~~~

// Returns the mask for the given square
inline uint64_t mask(const int square) {
    return 1ULL << square;
}

inline uint16_t encodeMove(const int start, const int end, const int promo) {
    if (start < 0 || start >= 64 || end < 0 || end >= 64 || promo < 0 || promo >= 8) {
        return INVALID_MOVE;
    }
    return (promo << 12) | (end << 6) | start;
}

inline int getStart(const uint16_t nextMove) {
    return nextMove & 0b111111;
}

inline int getEnd(const uint16_t nextMove) {
    return (nextMove >> 6) & 0b111111;
}

inline int getPromo(const uint16_t nextMove) {
    return (nextMove >> 12) & 0b111;
}

inline bool isInvalidMove(uint16_t nextMove) {
    return nextMove & 0x8000;
}

inline int getFile(int square) {
    return square % 8;
}

inline int getRank(int square) {
    return square / 8;
}