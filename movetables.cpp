#include "game.h"

namespace MoveTables {
    uint64_t knightMoves[64];
    uint64_t kingMoves[64];
    uint64_t rookMoves[64][4096];
    uint64_t bishopMoves[64][512];

    void initKnightMoves() {
        for (int sq = 0; sq < 64; sq++) {
            int x = sq % 8;
            int y = sq / 8;
            knightMoves[sq] = 0;

            int dx[] = {1, 2,  2,  1, -1, -2, -2, -1};
            int dy[] = {2, 1, -1, -2, -2, -1,  1,  2};

            for (int i = 0; i < 8; i++) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                    knightMoves[sq] |= mask(ny * 8 + nx);
                }
            }
        }
    }
    void initKingMoves() {
        for (int sq = 0; sq < 64; sq++) {
            int x = sq % 8;
            int y = sq / 8;
            kingMoves[sq] = 0;

            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                        kingMoves[sq] |= mask(ny * 8 + nx);
                    }
                }
            }
        }
    }

    int getBlockerIndex(uint64_t movementMask, uint64_t blockers) {
        int index = 0;
        int bit = 0;
        for (int sq = 0; sq < 64; ++sq) {
            if (movementMask & mask(sq)) {
                if (blockers & mask(sq)) {
                    index |= 1ULL << bit;
                }
                ++bit;
            }
        }
        return index;
    }
    uint64_t setBlockersFromIndex(uint64_t movementMask, int index) {
        uint64_t blockers = 0ULL;
        int bitPos = 0;

        for (int square = 0; square < 64; ++square) {
            if (movementMask & mask(square)) {
                if (index & 1ULL << bitPos) {
                    blockers |= mask(square);
                }
                bitPos++;
            }
        }

        return blockers;
    }

    // ~~~~~~~~~~~~~~~~ Rook move generation section ~~~~~~~~~~~~~~~~

    uint64_t generateRookBlockerMask(int square) {
        uint64_t blockerMask = 0ULL;
        int rank = square / 8;
        int file = square % 8;

        // Vertical (file), skip top and bottom edges
        for (int r = rank + 1; r <= 6; r++) {
            blockerMask |= mask(r * 8 + file);
        }
        for (int r = rank - 1; r >= 1; r--) {
            blockerMask |= mask(r * 8 + file);
        }

        // Horizontal (rank), skip left and right edges
        for (int f = file + 1; f <= 6; f++) {
            blockerMask |= mask(rank * 8 + f);
        }
        for (int f = file - 1; f >= 1; f--) {
            blockerMask |= mask(rank * 8 + f);
        }

        return blockerMask;
    }
    uint64_t computeRookAttacks(int square, uint64_t blockers) {
        uint64_t attacks = 0ULL;
        int rank = square / 8;
        int file = square % 8;

        // Up
        for (int r = rank + 1; r <= 7; r++) {
            uint64_t sqMask = mask(r * 8 + file);
            attacks |= sqMask;
            if (blockers & sqMask) break;
        }
        // Down
        for (int r = rank - 1; r >= 0; r--) {
            uint64_t sqMask = mask(r * 8 + file);
            attacks |= sqMask;
            if (blockers & sqMask) break;
        }
        // Right
        for (int f = file + 1; f <= 7; f++) {
            uint64_t sqMask = mask(rank * 8 + f);
            attacks |= sqMask;
            if (blockers & sqMask) break;
        }
        // Left
        for (int f = file - 1; f >= 0; f--) {
            uint64_t sqMask = mask(rank * 8 + f);
            attacks |= sqMask;
            if (blockers & sqMask) break;
        }

        return attacks;
    }
    void initRookMoves() {
        for (int square = 0; square < 64; square++) {
            uint64_t mask = generateRookBlockerMask(square);
            int numBits = __builtin_popcountll(mask);

            for (int index = 0; index < (1ULL << numBits); index++) {
                uint64_t blockers = setBlockersFromIndex(mask, index);
                rookMoves[square][index] = computeRookAttacks(square, blockers);
            }
        }
    }

    // ~~~~~~~~~~~~~~~~ Bishop move generation section ~~~~~~~~~~~~~~~~

    uint64_t generateBishopBlockerMask(int square) {
        uint64_t blockerMask = 0ULL;
        int rank = square / 8;
        int file = square % 8;

        // Top-right
        for (int r = rank + 1, f = file + 1; r <= 6 && f <= 6; ++r, ++f) {
            blockerMask |= mask(r * 8 + f);
        }
        // Top-left
        for (int r = rank + 1, f = file - 1; r <= 6 && f >= 1; ++r, --f) {
            blockerMask |= mask(r * 8 + f);
        }
        // Bottom-right
        for (int r = rank - 1, f = file + 1; r >= 1 && f <= 6; --r, ++f) {
            blockerMask |= mask(r * 8 + f);
        }
        // Bottom-left
        for (int r = rank - 1, f = file - 1; r >= 1 && f >= 1; --r, --f) {
            blockerMask |= mask(r * 8 + f);
        }

        return blockerMask;
    }
    uint64_t computeBishopAttacks(int square, uint64_t blockers) {
        uint64_t attacks = 0ULL;
        int rank = square / 8;
        int file = square % 8;

        // Up Right
        for (int r = rank + 1, f = file + 1; r <= 7 && f <= 7; ++r, ++f) {
            uint64_t sqMask = mask(r * 8 + f);
            attacks |= sqMask;
            if (blockers & sqMask) break;
        }
        // Down Right
        for (int r = rank - 1, f = file + 1; r >= 0 && f <= 7; --r, ++f) {
            uint64_t sqMask = mask(r * 8 + f);
            attacks |= sqMask;
            if (blockers & sqMask) break;
        }
        // Up Left
        for (int r = rank + 1, f = file - 1; r <= 7 && f >0; ++r, --f) {
            uint64_t sqMask = mask(r * 8 + f);
            attacks |= sqMask;
            if (blockers & sqMask) break;
        }
        // Down Left
        for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; --r, --f) {
            uint64_t sqMask = mask(r * 8 + f);
            attacks |= sqMask;
            if (blockers & sqMask) break;
        }

        return attacks;
    }
    void initBishopMoves() {
        for (int square = 0; square < 64; square++) {
            uint64_t mask = generateBishopBlockerMask(square);
            int numBits = __builtin_popcountll(mask);

            for (int index = 0; index < (1ULL << numBits); index++) {
                uint64_t blockers = setBlockersFromIndex(mask, index);
                bishopMoves[square][index] = computeBishopAttacks(square, blockers);
            }
        }
    }

    void init() {
        initKnightMoves();
        initKingMoves();
        initRookMoves();
        initBishopMoves();
    }
};