#include <cstdint>
#include <iostream>
#include <string>
#include <bitset>
#include <array>

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
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

struct bitBoards {
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

int add(int x, int y);

// TODO: Set bitboards to start of game state
void setup() {
    return;
}

// TODO: Display board in console
void printBoard() {
    return;
}


int main() {
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
        std::cout << add(1, 1);
        delete[] counts;
    }
    return 0;
}
// TIP See CLion help at <a
// href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>.
//  Also, you can try interactive lessons for CLion by selecting
//  'Help | Learn IDE Features' from the main menu.