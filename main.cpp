#include <cstdint>
#include <iostream>
#include <string>

enum PieceType {
    PAWN = 0,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NONE = -1  // for error fallback
};
const char* pieceNames[] = {"Pawn", "Knight", "Bishop", "Rook", "Queen", "King", "None"};
const uint16_t INVALID_MOVE = 0b1000000000000000;

inline uint64_t mask(int square);

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
struct {
    boardState state;
    BitBoards boards;
} game;

namespace MoveTables {
    uint64_t knightMoves[64];
    uint64_t kingMoves[64];

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

    inline void init() {
        initKnightMoves();
        initKingMoves();
    }
};

void setupFiles();
void setupRanks();
void setupStartingPosition();
void setupState();
void setup();
bool isValidMove(uint16_t nextMove, bool isWhiteTurn);
bool followsPieceMovementRules(PieceType pieceType, uint16_t nextMove, bool isWhiteTurn);
bool move(uint16_t nextMove, bool isWhiteTurn);
char getPieceAt(int square);
void printBoard();
int coordsToNum(std::string& input);
std::string squareName(int square);
uint16_t parseAlgebraicMove(std::string input, bool isWhiteTurn);
inline uint16_t encodeMove(int start, int end, int promo = 0);
inline int getStart(uint16_t move);
inline int getEnd(uint16_t move);
inline int getPromo(uint16_t move);

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
void setupStartingPosition() {
    game.boards.wPawns   = 0x000000000000FF00ULL;
    game.boards.bPawns   = 0x00FF000000000000ULL;
    game.boards.wKnights = 0x0000000000000042ULL;
    game.boards.bKnights = 0x4200000000000000ULL;
    game.boards.wRooks   = 0x0000000000000081ULL;
    game.boards.bRooks   = 0x8100000000000000ULL;
    game.boards.wBishops = 0x0000000000000024ULL;
    game.boards.bBishops = 0x2400000000000000ULL;
    game.boards.wQueens  = 0x0000000000000008ULL;
    game.boards.bQueens  = 0x0800000000000000ULL;
    game.boards.wKing    = 0x0000000000000010ULL;
    game.boards.bKing    = 0x1000000000000000ULL;
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
    MoveTables::init();
}

// Checks if the piece on startSquare can move to endSquare (considers player turn,
// piece movement path validity, and ending location, but not king checks
bool isValidMove(const uint16_t nextMove, const bool isWhiteTurn) {
    int startSquare = getStart(nextMove);
    int endSquare = getEnd(nextMove);
    uint64_t startMask = mask(startSquare);
    uint64_t endMask = mask(endSquare);
    // Player turn

    // If white didn't select a white piece
    if (isWhiteTurn && !(startMask & game.boards.wPieces)) {
        return false;
    }
    // If black didn't select a black piece
    if (!isWhiteTurn && !(startMask & game.boards.bPieces)) {
        return false;
    }

    // Ending location

    // If white is moving onto a white piece
    if (isWhiteTurn && (endMask & game.boards.wPieces)) {
        return false;
    }
    // If black is moving onto a black piece
    if (!isWhiteTurn && (endMask & game.boards.bPieces)) {
        return false;
    }

    // Piece movement path validity

    PieceType pieceType = NONE;
    uint64_t* bitboards[6] = {
        isWhiteTurn ? &game.boards.wPawns   : &game.boards.bPawns,
        isWhiteTurn ? &game.boards.wKnights : &game.boards.bKnights,
        isWhiteTurn ? &game.boards.wBishops : &game.boards.bBishops,
        isWhiteTurn ? &game.boards.wRooks   : &game.boards.bRooks,
        isWhiteTurn ? &game.boards.wQueens  : &game.boards.bQueens,
        isWhiteTurn ? &game.boards.wKing    : &game.boards.bKing
    };

    for (int i = 0; i < 6; ++i) {
        if (*bitboards[i] & startMask) {
            pieceType = static_cast<PieceType>(i);
            break;
        }
    }

    if (pieceType == NONE) {
        std::cout << "Error: couldn't determine piece type.\n";
        return false;
    }

    return followsPieceMovementRules(pieceType, nextMove, isWhiteTurn);
}

bool followsPieceMovementRules(const PieceType pieceType, const uint16_t nextMove, const bool isWhiteTurn) {
    int startSquare = getStart(nextMove);
    int endSquare = getEnd(nextMove);
    switch (pieceType) {
        case PAWN: {
            uint64_t startMask = mask(startSquare);
            uint64_t endMask = mask(endSquare);
            uint64_t occupied = game.boards.wPieces | game.boards.bPieces;

            // Standard move
            if (isWhiteTurn) {
                if (endMask == startMask << 8) {
                    return !(endMask & occupied);
                }
            }
            else {
                if (endMask == startMask >> 8) {
                    return !(endMask & occupied);
                }
            }

            // Double move
            if (isWhiteTurn) {
                if (startMask & ranks.SECOND_RANK && endMask == startMask << 16) {
                    uint64_t midMask = startMask << 8;
                    return !(midMask & occupied) && !(endMask & occupied);
                }
            }
            else {
                if (startMask & ranks.SEVENTH_RANK && endMask == startMask >> 16) {
                    uint64_t midMask = startMask >> 8;
                    return !(midMask & occupied) && !(endMask & occupied);
                }
            }

            // Capture
            uint64_t opponentPieces = isWhiteTurn ? game.boards.bPieces : game.boards.wPieces;
            if (isWhiteTurn) {
                // Left
                if ((startMask & ~files.A_FILE) && (endMask == startMask << 7)) {
                    return endMask & opponentPieces;
                }
                // Right
                if ((startMask & ~files.H_FILE) && (endMask == startMask << 9)) {
                    return endMask & opponentPieces;
                }
            }
            else {
                // Left
                if ((startMask & ~files.H_FILE) && (endMask == startMask >> 9)) {
                    return endMask & opponentPieces;
                }
                // Right
                if ((startMask & ~files.A_FILE) && (endMask == startMask >> 7)) {
                    return endMask & opponentPieces;
                }
            }

            return false;
        }
        case KNIGHT: {
            // return (dx * dx == 4 && dy * dy == 1) ||
            //        (dx * dx == 1 && dy * dy == 4);
            return MoveTables::knightMoves[startSquare] & mask(endSquare);
        }
        case BISHOP: {
            // TODO: Bishop move
            return false;
        }
        case ROOK: {
            // TODO: Rook move
            return true;
        }
        case QUEEN: {
            return followsPieceMovementRules(BISHOP, nextMove, isWhiteTurn) ||
                   followsPieceMovementRules(ROOK,   nextMove, isWhiteTurn);
        }
        case KING: {
            // return dx * dx <= 1 && dy * dy <= 1;
            return MoveTables::kingMoves[startSquare] & mask(endSquare);

            // TODO: Castling
        }
        default:
            return true;
    }
}

// Move a piece from startSquare to endSquare. Calls isValidMove to check validity
// Returns whether the move was successful or not
bool move(const uint16_t nextMove, const bool isWhiteTurn) {
    // Check basic move validity
    if (!isValidMove(nextMove, isWhiteTurn)) {
        std::cout << "Error: Move is invalid.\n";
        return false;
    }

    // TODO: Handle Promotions
    // TODO: Handle Castling
    // TODO: Handle En Passant

    // TODO: Check if the move puts the king in check

    int startSquare    = getStart(nextMove);
    int endSquare      = getEnd(nextMove);
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
    PieceType pieceType = NONE;
    for (int i = 0; i < 6; i++) {
        if (*myBitboards[i] & startMask) {
            pieceType = static_cast<PieceType>(i);
            break;
        }
    }

    // For testing. pieceType should never be -1 because isValidMove() should catch this.
    if (pieceType == NONE) {
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
    uint64_t sqMask = 0x1ULL << square;

    if (game.boards.wPawns   & sqMask) return 'P';
    if (game.boards.wKnights & sqMask) return 'N';
    if (game.boards.wBishops & sqMask) return 'B';
    if (game.boards.wRooks   & sqMask) return 'R';
    if (game.boards.wQueens  & sqMask) return 'Q';
    if (game.boards.wKing    & sqMask) return 'K';

    if (game.boards.bPawns   & sqMask) return 'p';
    if (game.boards.bKnights & sqMask) return 'n';
    if (game.boards.bBishops & sqMask) return 'b';
    if (game.boards.bRooks   & sqMask) return 'r';
    if (game.boards.bQueens  & sqMask) return 'q';
    if (game.boards.bKing    & sqMask) return 'k';

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
int coordsToNum(std::string& input) {
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
std::string squareName(const int square) {
    const char file = static_cast<char>('a' + (square % 8));
    const char rank = static_cast<char>('1' + (square / 8));
    return std::string() + file + rank;
}

uint16_t parseAlgebraicMove(std::string input, const bool isWhiteTurn) {
    // Remove trailing '+' or '#' (check or checkmate)
    while (!input.empty() && (input.back() == '+' || input.back() == '#')) {
        input.pop_back();
    }
    // Remove 'x' if it's a capture
    size_t xPos = input.find('x');
    if (xPos != std::string::npos) {
        input.erase(xPos, 1);
    }

    int startSquare = -1;
    char pieceChar = 'P'; // default is pawn

    // Determine destination square (last two characters)
    if (input.length() < 2) return INVALID_MOVE;
    std::string destStr = input.substr(input.length() - 2);
    const int endSquare = coordsToNum(destStr);
    if (endSquare == -1) return INVALID_MOVE;

    // Determine piece type (if not a pawn)
    if (isupper(input[0]) && input[0] != 'O') {
        pieceChar = input[0];
        input = input.substr(1);
    }

    // Identify candidate pieces of that type
    uint64_t* pieceBB = nullptr;
    switch (pieceChar) {
        case 'N': pieceBB = isWhiteTurn ? &game.boards.wKnights : &game.boards.bKnights; break;
        case 'B': pieceBB = isWhiteTurn ? &game.boards.wBishops : &game.boards.bBishops; break;
        case 'R': pieceBB = isWhiteTurn ? &game.boards.wRooks   : &game.boards.bRooks;   break;
        case 'Q': pieceBB = isWhiteTurn ? &game.boards.wQueens  : &game.boards.bQueens;  break;
        case 'K': pieceBB = isWhiteTurn ? &game.boards.wKing    : &game.boards.bKing;    break;
        case 'P': pieceBB = isWhiteTurn ? &game.boards.wPawns   : &game.boards.bPawns;   break;
        default: return INVALID_MOVE; // Unknown piece
    }

    // Try all pieces of that type and return the one that can legally move to endSquare
    for (int sq = 0; sq < 64; sq++) {
        if ((*pieceBB) & mask(sq)) {
            uint16_t candidateMove = encodeMove(sq, endSquare);
            if (isValidMove(candidateMove, isWhiteTurn)) {
                return candidateMove;
            }
        }
    }
    return INVALID_MOVE;
}

inline uint16_t encodeMove(const int start, const int end, const int promo) {
    if (start < 0 || start >= 64 || end < 0 || end >= 64 || promo < 0 || promo >= 8) {
        return INVALID_MOVE;
    }
    return (promo << 12) | (end << 6) | start;
}

inline int getStart(uint16_t nextMove) {
    return nextMove & 0b111111;
}
inline int getEnd(uint16_t nextMove) {
    return (nextMove >> 6) & 0b111111;
}
inline int getPromo(uint16_t nextMove) {
    return (nextMove >> 12) & 0b111;
}

[[noreturn]] int main() {
    setup();

    while (true) {
        std::string moveStr;

        bool successful = false;
        while (!successful) {
            printBoard();
            std::cout << "Enter move (e.g. e4, Nf3, Qxe5): ";
            std::cin >> moveStr;
            uint16_t nextMove = parseAlgebraicMove(moveStr, game.state.isWhiteTurn);
            if (nextMove == INVALID_MOVE) {
                std::cout << "Invalid input.\n";
                continue;
            }
            successful = move(nextMove, game.state.isWhiteTurn);
        }
        game.state.isWhiteTurn = !game.state.isWhiteTurn;
    }
}