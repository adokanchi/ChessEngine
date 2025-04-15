#include <cstdint>
#include <iostream>
#include <string>
#include <bitset>
#include <array>

struct boardState {
    // true if white's turn, false if black's turn
    bool isWhiteTurn;
    // moveCounter = moves since lsat 50-move-rule reset
    int moveCounter;
    // castling represents whether castling would be legal for each side (changes when kings/rooks are moved)
    int castling;
    // The square that en passant is available on (-1 if not available, 0-63 if available)
    int epSquare;
};

struct BitBoards {
    // At all times, [bitboard] & [other bitboard] == 0
    // and wPawns | wKnights | wRooks | wBishops | wQueens | wKing == wPieces (and same with black)
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
    uint64_t wKings;
    uint64_t bKings;
    uint64_t wPieces;
    uint64_t bPieces;
};

struct Files {
    uint64_t A_FILE;
    uint64_t B_FILE;
    uint64_t C_FILE;
    uint64_t D_FILE;
    uint64_t E_FILE;
    uint64_t F_FILE;
    uint64_t G_FILE;
    uint64_t H_FILE;
};

struct Ranks {
    uint64_t FIRST_RANK;
    uint64_t SECOND_RANK;
    uint64_t THIRD_RANK;
    uint64_t FOURTH_RANK;
    uint64_t FIFTH_RANK;
    uint64_t SIXTH_RANK;
    uint64_t SEVENTH_RANK;
    uint64_t EIGHTH_RANK;
};

struct Game {
    boardState state;
    BitBoards boards;
    Ranks ranks;
    Files files;
};

// TODO: Set masks
void setupFiles(Game game) {
    game.files.A_FILE = 0;
    game.files.B_FILE = 0;
    game.files.C_FILE = 0;
    game.files.D_FILE = 0;
    game.files.E_FILE = 0;
    game.files.F_FILE = 0;
    game.files.G_FILE = 0;
    game.files.H_FILE = 0;
}

// TODO: Set masks
void setupRanks(Game game) {
    game.ranks.FIRST_RANK = 0;
    game.ranks.SECOND_RANK = 0;
    game.ranks.THIRD_RANK = 0;
    game.ranks.FOURTH_RANK = 0;
    game.ranks.FIFTH_RANK = 0;
    game.ranks.SIXTH_RANK = 0;
    game.ranks.SEVENTH_RANK = 0;
    game.ranks.EIGHTH_RANK = 0;
}

// TODO: Set masks
void setupStartingPosition(Game game) {
    game.boards.wPawns = 0;
    game.boards.bPawns = 0;
    game.boards.wKnights = 0;
    game.boards.bKnights = 0;
    game.boards.wRooks = 0;
    game.boards.bRooks = 0;
    game.boards.wBishops = 0;
    game.boards.bBishops = 0;
    game.boards.wQueens = 0;
    game.boards.bQueens = 0;
    game.boards.wKings = 0;
    game.boards.bKings = 0;
    game.boards.wPieces = 0;
    game.boards.bPieces = 0;
}

// TODO: Setup state
void setupState(Game game) {
    game.state.isWhiteTurn = true;
    game.state.epSquare = -1;
    game.state.moveCounter = 0;
    game.state.castling = 0;
}

// TODO: Set bitboards to start of game state and initialize rank/file masks
void setup(Game game) {
    setupFiles(game);
    setupRanks(game);
    setupStartingPosition(game);
    setupState(game);
}

// TODO: Display board in console
void printBoard() {
}

int main() {
    Game game;
    setup(game);




    while (true) {
        int target;
        auto coins = {1, 5, 10, 25};

        std::cin >> target;
        if (target <= 0) break;

        long *counts = new long[target+1];
        for (int i = 0; i <= target; ++i) counts[i] = 0;

        counts[0] = 1;
        for (int coinVal : coins) {
            for (int i = coinVal; i <= target; i++) {
                counts[i] += counts[i - coinVal];
            }
        }

        std::cout << counts[target] << "\n~~~~~~~~\n";
        delete[] counts;
    }
    return 0;
}