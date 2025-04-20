#include <cstdint>
#include <iostream>
#include <string>

void setupFiles();
void setupRanks();
void setupStartingPosition();
void setupState();
void setup();
bool isValidMove(int startSquare, int endSquare, bool isWhiteTurn);
bool move(int startSquare, int endSquare, bool isWhiteTurn);
char getPieceAt(int square);
inline uint64_t mask(int square);
void printBoard();
int algebraicToNum(std::string& input);
std::string squareName(int square);

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
    // At all times, [bitboard] & [other bitboard] == 0 unless one of the bitboards is a pieces board
    // and wPawns | wKnights | wRooks | wBishops | wQueens | wKing == wPieces (and same with black)
    uint64_t wPawns   = 0;
    uint64_t bPawns   = 0;
    uint64_t wKnights = 0;
    uint64_t bKnights = 0;
    uint64_t wRooks   = 0;
    uint64_t bRooks   = 0;
    uint64_t wBishops = 0;
    uint64_t bBishops = 0;
    uint64_t wQueens  = 0;
    uint64_t bQueens  = 0;
    uint64_t wKing    = 0;
    uint64_t bKing    = 0;
    uint64_t wPieces  = 0;
    uint64_t bPieces  = 0;
};

struct {
    uint64_t A_FILE = 0;
    uint64_t B_FILE = 0;
    uint64_t C_FILE = 0;
    uint64_t D_FILE = 0;
    uint64_t E_FILE = 0;
    uint64_t F_FILE = 0;
    uint64_t G_FILE = 0;
    uint64_t H_FILE = 0;
} files;

struct {
    uint64_t FIRST_RANK   = 0;
    uint64_t SECOND_RANK  = 0;
    uint64_t THIRD_RANK   = 0;
    uint64_t FOURTH_RANK  = 0;
    uint64_t FIFTH_RANK   = 0;
    uint64_t SIXTH_RANK   = 0;
    uint64_t SEVENTH_RANK = 0;
    uint64_t EIGHTH_RANK  = 0;
} ranks;

struct Game {
    boardState state;
    BitBoards boards;
};

Game game;

// TODO: Check if masks are correct
void setupFiles() {
    files.A_FILE = 0x8080808080808080ULL;
    files.B_FILE = 0x4040404040404040ULL;
    files.C_FILE = 0x2020202020202020ULL;
    files.D_FILE = 0x1010101010101010ULL;
    files.E_FILE = 0x0808080808080808ULL;
    files.F_FILE = 0x0404040404040404ULL;
    files.G_FILE = 0x0202020202020202ULL;
    files.H_FILE = 0x0101010101010101ULL;
}

// TODO: Check if masks are correct
void setupRanks() {
    ranks.FIRST_RANK   = 0x00000000000000FFULL;
    ranks.SECOND_RANK  = 0x000000000000FF00ULL;
    ranks.THIRD_RANK   = 0x0000000000FF0000ULL;
    ranks.FOURTH_RANK  = 0x00000000FF000000ULL;
    ranks.FIFTH_RANK   = 0x000000FF00000000ULL;
    ranks.SIXTH_RANK   = 0x0000FF0000000000ULL;
    ranks.SEVENTH_RANK = 0x00FF000000000000ULL;
    ranks.EIGHTH_RANK  = 0xFF00000000000000ULL;
}

// TODO: Check if masks are correct
void setupStartingPosition() {
    game.boards.wPawns   = 0x000000000000FF00ULL;
    game.boards.bPawns   = 0x00FF000000000000ULL;
    game.boards.wKnights = 0x0000000000000042ULL;
    game.boards.bKnights = 0x4200000000000000ULL;
    game.boards.wRooks   = 0x0000000000000081ULL;
    game.boards.bRooks   = 0x8100000000000000ULL;
    game.boards.wBishops = 0x0000000000000024ULL;
    game.boards.bBishops = 0x2400000000000000ULL;
    game.boards.wQueens  = 0x0000000000000010ULL;
    game.boards.bQueens  = 0x1000000000000000ULL;
    game.boards.wKing    = 0x0000000000000008ULL;
    game.boards.bKing    = 0x0800000000000000ULL;
    game.boards.wPieces  = 0x000000000000FFFFULL;
    game.boards.bPieces  = 0xFFFF000000000000ULL;
}

// TODO: Setup state
void setupState() {
    game.state.isWhiteTurn = true;
    game.state.epSquare = -1;
    game.state.moveCounter = 0;
    game.state.castling = 0;
}

void setup() {
    setupFiles();
    setupRanks();
    setupStartingPosition();
    setupState();
}

// Checks if the piece on startSquare can move to endSquare (considers player turn,
// piece movement path validity, and ending location, but not king checks
bool isValidMove(int startSquare, int endSquare, bool isWhiteTurn) {
    // Player turn

    // If white didn't select a white piece
    if (isWhiteTurn && !(mask(startSquare) & game.boards.wPieces)) {
        return false;
    }
    // If black didn't select a black piece
    if (!isWhiteTurn && !(mask(startSquare) & game.boards.bPieces)) {
        return false;
    }

    // Ending location

    // If white is moving onto a white piece
    if (isWhiteTurn && (mask(endSquare) & game.boards.wPieces)) {
        return false;
    }
    // If black is moving onto a black piece
    if (!isWhiteTurn && (mask(endSquare) & game.boards.bPieces)) {
        return false;
    }

    // TODO: Check piece movement path validity
    return true;
}

// Move a piece from startSquare to endSquare. Calls isValidMove to check validity
bool move(int startSquare, int endSquare, bool isWhiteTurn) {
    // Check basic move validity
    if (!isValidMove(startSquare, endSquare, isWhiteTurn)) {
        std::cout << "Error: Move is invalid.\n";
        return false;
    }

    // TODO: Handle Promotions
    // TODO: Handle Castling
    // TODO: Handle En Passant

    // TODO: Check if the move puts the king in check

    uint64_t startMask = mask(startSquare);
    uint64_t endMask   = mask(endSquare);

    BitBoards& boards = game.boards;
    uint64_t* myBitboards[6] = {
        isWhiteTurn ? &boards.wPawns   : &boards.bPawns,
        isWhiteTurn ? &boards.wKnights : &boards.bKnights,
        isWhiteTurn ? &boards.wBishops : &boards.bBishops,
        isWhiteTurn ? &boards.wRooks   : &boards.bRooks,
        isWhiteTurn ? &boards.wQueens  : &boards.bQueens,
        isWhiteTurn ? &boards.wKing    : &boards.bKing
    };

    uint64_t* theirBitboards[6] = {
        isWhiteTurn ? &boards.bPawns   : &boards.wPawns,
        isWhiteTurn ? &boards.bKnights : &boards.wKnights,
        isWhiteTurn ? &boards.bBishops : &boards.wBishops,
        isWhiteTurn ? &boards.bRooks   : &boards.wRooks,
        isWhiteTurn ? &boards.bQueens  : &boards.wQueens,
        isWhiteTurn ? &boards.bKing    : &boards.wKing
    };

    // Step 1: Identify which piece is moving
    int pieceType = -1;
    for (int i = 0; i < 6; i++) {
        if (*myBitboards[i] & startMask) {
            pieceType = i;
            break;
        }
    }

    // For testing. pieceType should never be -1 because isValidMove() should catch this.
    if (pieceType == -1) {
        std::cout << "Error: No piece found at startSquare.\n";
        return false;
    }

    // Step 2: Handle capture by clearing the enemy bitboard if there's a piece at endSquare
    uint64_t& enemyPieces = isWhiteTurn ? boards.bPieces : boards.wPieces;
    for (uint64_t* bb : theirBitboards) {
        if (*bb & endMask) {
            *bb &= ~endMask;
            enemyPieces &= ~endMask;
            break;
        }
    }

    // Step 3: Move the piece — clear from startSquare, set at endSquare
    *myBitboards[pieceType] &= ~startMask;  // Remove piece from start
    *myBitboards[pieceType] |=  endMask;    // Place piece at end
    uint64_t& ownPieces = isWhiteTurn ? boards.wPieces : boards.bPieces;
    ownPieces &= ~startMask;
    ownPieces |=  endMask;

    return true;
}

// Returns the piece at square (for displaying board in console)
char getPieceAt(int square) {
    uint64_t mask = 0x1ULL << square;

    if (game.boards.wPawns   & mask) return 'P';
    if (game.boards.wRooks   & mask) return 'R';
    if (game.boards.wKnights & mask) return 'N';
    if (game.boards.wBishops & mask) return 'B';
    if (game.boards.wQueens  & mask) return 'Q';
    if (game.boards.wKing    & mask) return 'K';

    if (game.boards.bPawns   & mask) return 'p';
    if (game.boards.bRooks   & mask) return 'r';
    if (game.boards.bKnights & mask) return 'n';
    if (game.boards.bBishops & mask) return 'b';
    if (game.boards.bQueens  & mask) return 'q';
    if (game.boards.bKing    & mask) return 'k';

    return '.';
}

// Returns the mask for the given square
inline uint64_t mask(int square) {
    return 1ULL << square;
}

// Displays the board in the terminal
void printBoard() {
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << (rank + 1) << " | ";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            std::cout << getPieceAt(square) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "    _ _ _ _ _ _ _ _\n";
    std::cout << "    a b c d e f g h\n";

    std::cout << (game.state.isWhiteTurn ? "White" : "Black") << " to move.\n";
}

// Interprets a two-character string as a square number using algebraic notation
int algebraicToNum(std::string& input) {
    if (input.size() != 2) return -1;
    char file = input[0];
    char rank = input[1];
    if (file < 'a' || file > 'h') return -1;
    if (rank < '1' || rank > '8') return -1;

    int fileIndex = file - 'a';
    int rankIndex = rank - '1';

    return rankIndex * 8 + fileIndex;
}

// Interprets a square number as a two-character string using algebraic notation
std::string squareName(int square) {
    char file = static_cast<char>('a' + (square % 8));
    char rank = static_cast<char>('1' + (square / 8));
    return std::string() + file + rank;
}

[[noreturn]] int main() {
    setup();

    while (true) {
        std::string startStr, endStr;
        int start = 0, end = 0;
        bool successful = false;

        while (!successful) {
            printBoard();
            std::cout << "Enter move (e.g. e2 e4): ";
            std::cin >> startStr >> endStr;
            start = algebraicToNum(startStr);
            end = algebraicToNum(endStr);
            if (start == -1 || end == -1) {
                std::cout << "Invalid input.\n";
                continue;
            }
            successful = move(start, end, game.state.isWhiteTurn);
        }
        std::cout << "Moving from " << squareName(start)
                << " to " << squareName(end) << "\n\n\n";
        game.state.isWhiteTurn = !game.state.isWhiteTurn;
    }
}
