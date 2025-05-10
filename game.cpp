#include "game.h"
#include <fstream>
#include <iostream>

// ~~~~~~~~~~~~~~~~ Board Setup and Game Cycle Section ~~~~~~~~~~~~~~~~

FileMasks files;
RankMasks ranks;
GameData game;

void setupFiles() {
    files.A_FILE = 0x0101010101010101ULL;
    files.B_FILE = 0x0202020202020202ULL;
    files.C_FILE = 0x0404040404040404ULL;
    files.D_FILE = 0x0808080808080808ULL;
    files.E_FILE = 0x1010101010101010ULL;
    files.F_FILE = 0x2020202020202020ULL;
    files.G_FILE = 0x4040404040404040ULL;
    files.H_FILE = 0x8080808080808080ULL;
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
void setupState() {
    game.state.isWhiteTurn = true;
    game.state.epSquare = -1;
    game.state.moveCounter = 0;
    game.state.castling = 0b1111;
}
void setup() {
    setupFiles();
    setupRanks();
    setupStartingPosition();
    setupState();
    MoveTables::init();
}
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
[[noreturn]] void runInConsole() {
    setup();
    while (true) {
        std::string moveStr;

        bool successful = false;
        while (!successful) {
            printBoard();
            std::cout << "Enter move (e.g. e4, Nf3, Qxe5): ";
            std::cin >> moveStr;
            uint16_t nextMove = parseAlgebraicMove(moveStr, game.state.isWhiteTurn);
            if (isInvalidMove(nextMove)) {
                std::cout << "ERR: parseAlgebraicMove() returned INVALID_MOVE.\n";
                continue;
            }
            successful = move(nextMove, game.state.isWhiteTurn);
        }
        game.state.isWhiteTurn = !game.state.isWhiteTurn;
    }
}

// ~~~~~~~~~~~~~~~~ Move Calculating/Parsing section ~~~~~~~~~~~~~~~~

// Checks if the piece on startSquare can move to endSquare (considers player turn,
// piece movement path validity, and ending location, but not king checks
bool isValidMove(const uint16_t nextMove, const bool isWhiteTurn) {
    int startSquare = getStart(nextMove);
    int endSquare = getEnd(nextMove);
    uint64_t startMask = mask(startSquare);
    uint64_t endMask = mask(endSquare);

    // If white didn't select a white piece
    if (isWhiteTurn && !(startMask & game.boards.wPieces)) {
        return false;
    }
    // If black didn't select a black piece
    if (!isWhiteTurn && !(startMask & game.boards.bPieces)) {
        return false;
    }
    // If white is moving onto a white piece
    if (isWhiteTurn && (endMask & game.boards.wPieces)) {
        return false;
    }
    // If black is moving onto a black piece
    if (!isWhiteTurn && (endMask & game.boards.bPieces)) {
        return false;
    }

    PieceType pieceType = PieceType::None;
    uint64_t* bitboards[6] = {
        isWhiteTurn ? &game.boards.wPawns   : &game.boards.bPawns,
        isWhiteTurn ? &game.boards.wKnights : &game.boards.bKnights,
        isWhiteTurn ? &game.boards.wBishops : &game.boards.bBishops,
        isWhiteTurn ? &game.boards.wRooks   : &game.boards.bRooks,
        isWhiteTurn ? &game.boards.wQueens  : &game.boards.bQueens,
        isWhiteTurn ? &game.boards.wKing    : &game.boards.bKing
    };

    for (int i = 0; i < NUM_PIECE_TYPES; ++i) {
        if (*bitboards[i] & startMask) {
            pieceType = static_cast<PieceType>(i);
            break;
        }
    }

    if (pieceType == PieceType::None) {
        std::cout << "Error: couldn't determine piece type.\n";
        return false;
    }

    // Castling
    uint64_t occupied = game.boards.wPieces | game.boards.bPieces;
    if (pieceType == PieceType::King && abs(endSquare - startSquare) == 2) {
        // White
        if (startSquare == 4 && endSquare == 6 && (game.state.castling & CASTLE_WK)) {
            return !(mask(5) & occupied) && !(mask(6) & occupied);
        }
        if (startSquare == 4 && endSquare == 2 && (game.state.castling & CASTLE_WQ)) {
            return !(mask(3) & occupied) && !(mask(2) & occupied) && !(mask(1) & occupied);
        }
        // Black
        if (startSquare == 60 && endSquare == 62 && (game.state.castling & CASTLE_BK)) {
            return !(mask(61) & occupied) && !(mask(62) & occupied);
        }
        if (startSquare == 60 && endSquare == 58 && (game.state.castling & CASTLE_BQ)) {
            return !(mask(59) & occupied) && !(mask(58) & occupied) && !(mask(57) & occupied);
        }
    }

    return followsPieceMovementRules(pieceType, nextMove, isWhiteTurn);
}

bool followsPieceMovementRules(const PieceType pieceType, const uint16_t nextMove, const bool isWhiteTurn) {
    int startSquare = getStart(nextMove);
    int endSquare = getEnd(nextMove);
    switch (pieceType) {
        case PieceType::Pawn: {
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
                    return endMask & opponentPieces || endSquare == game.state.epSquare;
                }
                // Right
                if ((startMask & ~files.H_FILE) && (endMask == startMask << 9)) {
                    return endMask & opponentPieces || endSquare == game.state.epSquare;
                }
            }
            else {
                // Left
                if ((startMask & ~files.A_FILE) && (endMask == startMask >> 9)) {
                    return endMask & opponentPieces || endSquare == game.state.epSquare;
                }
                // Right
                if ((startMask & ~files.H_FILE) && (endMask == startMask >> 7)) {
                    return endMask & opponentPieces || endSquare == game.state.epSquare;
                }
            }

            return false;
        }
        case PieceType::Knight: {
            return MoveTables::knightMoves[startSquare] & mask(endSquare);
        }
        case PieceType::Bishop: {
            // Encode the blocker pattern into an index
            int index = 0;
            uint64_t occupied = game.boards.wPieces | game.boards.bPieces;
            uint64_t blockerMask = MoveTables::generateBishopBlockerMask(startSquare);
            int bitPos = 0;
            for (int sq = 0; sq < 64; sq++) {
                if (blockerMask & mask(sq)) {
                    if (occupied & mask(sq)) {
                        index |= (1 << bitPos);
                    }
                    bitPos++;
                }
            }

            // Array lookup
            return (MoveTables::bishopMoves[startSquare][index] & mask(endSquare));
        }
        case PieceType::Rook: {
            // Encode the blocker pattern into an index
            int index = 0;
            uint64_t occupied = game.boards.wPieces | game.boards.bPieces;
            uint64_t blockerMask = MoveTables::generateRookBlockerMask(startSquare);
            int bitPos = 0;
            for (int sq = 0; sq < 64; sq++) {
                if (blockerMask & mask(sq)) {
                    if (occupied & mask(sq)) {
                        index |= (1 << bitPos);
                    }
                    bitPos++;
                }
            }
            // Array lookup
            return (MoveTables::rookMoves[startSquare][index] & mask(endSquare));
        }
        case PieceType::Queen: {
            return followsPieceMovementRules(PieceType::Bishop, nextMove, isWhiteTurn) ||
                   followsPieceMovementRules(PieceType::Rook,   nextMove, isWhiteTurn);
        }
        case PieceType::King: {
            // Castling is handled in isValidMove()
            return MoveTables::kingMoves[startSquare] & mask(endSquare);
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
        return false;
    }

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
    PieceType pieceType = PieceType::None;
    for (int i = 0; i < NUM_PIECE_TYPES; i++) {
        if (*myBitboards[i] & startMask) {
            pieceType = static_cast<PieceType>(i);
            break;
        }
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
    // En Passant
    if (pieceType == PieceType::Pawn && endSquare == game.state.epSquare) {
        int capturedPawnSquare = isWhiteTurn ? endSquare - 8 : endSquare + 8;
        uint64_t capturedPawnMask = mask(capturedPawnSquare);
        (isWhiteTurn ? boards.bPawns : boards.wPawns) &= ~capturedPawnMask;
        (isWhiteTurn ? boards.bPieces : boards.wPieces) &= ~capturedPawnMask;
    }

    // Step 3: Move the piece — clear from startSquare, set at endSquare. Handle pawn promotion.
    int promoType = getPromo(nextMove);
    if (pieceType == PieceType::Pawn && promoType != 0) {
        // Remove Pawn
        *myBitboards[static_cast<int>(pieceType)] &= ~startMask;
        // Add the new piece
        switch (promoType) {
            case 1: { // Knight
                (isWhiteTurn ? boards.wKnights : boards.bKnights) |= endMask;
                break;
            }
            case 2: { // Bishop
                (isWhiteTurn ? boards.wBishops : boards.bBishops) |= endMask;
                break;
            }
            case 3: { // Rook
                (isWhiteTurn ? boards.wRooks : boards.bRooks) |= endMask;
                break;
            }
            case 4: { // Queen
                (isWhiteTurn ? boards.wQueens : boards.bQueens) |= endMask;
                break;
            }
            default: break;
        }
    }
    else {
        int pieceIdx = static_cast<int>(pieceType);
        *myBitboards[pieceIdx] &= ~startMask;  // Remove piece from start
        *myBitboards[pieceIdx] |=  endMask;    // Place piece at end
    }


    // Update wPieces or bPieces to reflect the move.
    uint64_t& ownPieces = isWhiteTurn ? boards.wPieces : boards.bPieces;
    ownPieces &= ~startMask;
    ownPieces |=  endMask;

    // Move the Rook if castling
    if (pieceType == PieceType::King && abs(endSquare - startSquare) == 2) {
        if (endSquare == 6) {  // White kingside
            boards.wRooks &= ~mask(7);
            boards.wRooks |= mask(5);
            boards.wPieces &= ~mask(7);
            boards.wPieces |= mask(5);
        } else if (endSquare == 2) {  // White queenside
            boards.wRooks &= ~mask(0);
            boards.wRooks |= mask(3);
            boards.wPieces &= ~mask(0);
            boards.wPieces |= mask(3);
        } else if (endSquare == 62) {  // Black kingside
            boards.bRooks &= ~mask(63);
            boards.bRooks |= mask(61);
            boards.bPieces &= ~mask(63);
            boards.bPieces |= mask(61);
        } else if (endSquare == 58) {  // Black queenside
            boards.bRooks &= ~mask(56);
            boards.bRooks |= mask(59);
            boards.bPieces &= ~mask(56);
            boards.bPieces |= mask(59);
        }
    }

    // Update game state data
    updateCastlingRights(startSquare, endSquare);
    game.state.epSquare = -1;
    if (pieceType == PieceType::Pawn && abs(endSquare - startSquare) == 16) {
        game.state.epSquare = startSquare + (isWhiteTurn ? 8 : -8);
    }

    return true;
}

void updateCastlingRights(int startSquare, int endSquare) {
    switch (startSquare) {
        // King
        case 4:  game.state.castling &= ~(CASTLE_WQ | CASTLE_WK); break;
        case 60: game.state.castling &= ~(CASTLE_BQ | CASTLE_BK); break;
        // Rook
        case 0:  game.state.castling &= ~CASTLE_WQ; break;
        case 7:  game.state.castling &= ~CASTLE_WK; break;
        case 56: game.state.castling &= ~CASTLE_BQ; break;
        case 63: game.state.castling &= ~CASTLE_BK; break;
        default: break;
    }
    switch (endSquare) {
        case 0:  game.state.castling &= ~CASTLE_WQ; break;
        case 7:  game.state.castling &= ~CASTLE_WK; break;
        case 56: game.state.castling &= ~CASTLE_BQ; break;
        case 63: game.state.castling &= ~CASTLE_BK; break;
        default: break;
    }
}

// Returns the piece at square (for displaying board in console)
char getPieceAt(const int square) {
    const uint64_t sqMask = mask(square);

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

// Interprets a two-character string as a square number using algebraic notation
int coordsToNum(const std::string& input) {
    if (input.size() != 2) return -1;
    const char file = input[0];
    const char rank = input[1];
    if (file < 'a' || file > 'h') return -1;
    if (rank < '1' || rank > '8') return -1;

    const int fileIndex = file - 'a';
    const int rankIndex = rank - '1';

    return rankIndex * 8 + fileIndex;
}

// Interprets a square number as a two-character string using algebraic notation
std::string squareName(const int square) {
    const char file = static_cast<char>('a' + (square % 8));
    const char rank = static_cast<char>('1' + (square / 8));
    return std::string() + file + rank;
}

uint16_t parseAlgebraicMove(std::string input, const bool isWhiteTurn) {
    if (input == "O-O") {
        return isWhiteTurn ? encodeMove(4, 6, 0) : encodeMove(60, 62, 0);
    }
    if (input == "O-O-O") {
        return isWhiteTurn ? encodeMove(4, 2, 0) : encodeMove(60, 58, 0);
    }

    // Remove trailing '+' or '#' (check or checkmate)
    while (!input.empty() && (input.back() == '+' || input.back() == '#')) {
        input.pop_back();
    }
    // Remove 'x' if it's a capture
    if (const size_t xPos = input.find('x'); xPos != std::string::npos) {
        input.erase(xPos, 1);
    }


    char pieceChar = 'P'; // default is pawn

    // Handle pawn promotion
    char promoChar = '\0';
    if (const size_t eqPos = input.find('='); eqPos != std::string::npos && eqPos + 1 < input.size()) {
        promoChar = input[eqPos + 1];
        input.erase(eqPos);  // Remove "=Q" from input so the rest parses cleanly
    }

    // Determine destination square (last two characters)
    if (input.length() < 2) return INVALID_MOVE;

    const std::string destStr = input.substr(input.size() - 2);
    input.erase(input.size() - 2);
    const int endSquare = coordsToNum(destStr);
    if (endSquare == -1) return INVALID_MOVE;

    // Determine piece type (if not a pawn)
    if (isupper(input[0]) && input[0] != 'O') {
        pieceChar = input[0];
        input = input.substr(1);
    }

    // Detect pawn promotion type
    int promoType = 0;
    switch (promoChar) {
        case 'N': {
            promoType = 1;
            break;
        }
        case 'B': {
            promoType = 2;
            break;
        }
        case 'R': {
            promoType = 3;
            break;
        }
        case 'Q': {
            promoType = 4;
            break;
        }
        case '\0': {
            break;
        }
        default: return INVALID_MOVE;
    }

    // Only allow promotion by pawns on the last rank
    if (promoType != 0) {
        if (pieceChar != 'P') return INVALID_MOVE;
        if (isWhiteTurn && getRank(endSquare) != 7) return INVALID_MOVE;
        if (!isWhiteTurn && getRank(endSquare) != 0) return INVALID_MOVE;
    }
    // Make pawns on the last rank promote
    else {
        if (isWhiteTurn && getRank(endSquare) == 7) return INVALID_MOVE;
        if (!isWhiteTurn && getRank(endSquare) == 0) return INVALID_MOVE;
    }

    // Disambiguation (e.g. Nbd2 or similar notation)
    int disambigFile = -1;
    int disambigRank = -1;
    if (!input.empty()) {
        if (input[0] >= 'a' && input[0] <= 'h') disambigFile = input[0] - 'a';
        else if (input[0] >= '1' && input[0] <= '8') disambigRank = input[0] - '1';
        if (input.size() > 1) {
             if (input[1] >= '1' && input[1] <= '8') disambigRank = input[1] - '1';
        }
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
    // Fails if the move is ambiguous
    bool uniqueMatchFound = false;
    uint16_t possibleMove = INVALID_MOVE;
    for (int sq = 0; sq < 64; sq++) {
        if ((*pieceBB) & mask(sq)) {
            // Check disambiguation info
            if (disambigFile != -1 && getFile(sq) != disambigFile) continue;
            if (disambigRank != -1 && getRank(sq) != disambigRank) continue;
            // Check if piece movement works
            if (const uint16_t candidateMove = encodeMove(sq, endSquare, promoType); isValidMove(candidateMove, isWhiteTurn)) {
                if (uniqueMatchFound) return INVALID_MOVE;
                uniqueMatchFound = true;
                possibleMove = candidateMove;
            }
        }
    }
    return possibleMove;
}

bool isSquareAttacked(int square, bool byWhite, const BitBoards& b) {
    const uint64_t targetMask = mask(square);

    // Pawns
    if (byWhite) {
        if (((targetMask & ~files.A_FILE) >> 7) & b.wPawns) return true;  // left capture
        if (((targetMask & ~files.H_FILE) >> 9) & b.wPawns) return true;  // right capture
    }
    else {
        if (((targetMask & ~files.H_FILE) << 7) & b.bPawns) return true;
        if (((targetMask & ~files.A_FILE) << 9) & b.bPawns) return true;
    }

    // Knights
    if (MoveTables::knightMoves[square] & (byWhite ? b.wKnights : b.bKnights)) return true;

    // Kings
    if (MoveTables::kingMoves[square] & (byWhite ? b.wKing : b.bKing)) return true;

    // Bishop/Queen attacks
    uint64_t blockers = b.wPieces | b.bPieces;
    uint64_t bishopMask = MoveTables::generateBishopBlockerMask(square);
    int index = MoveTables::getBlockerIndex(bishopMask, blockers);
    if (MoveTables::bishopMoves[square][index] & (byWhite ? b.wBishops | b.wQueens : b.bBishops | b.bQueens)) return true;

    // Rook/Queens
    uint64_t rookMask = MoveTables::generateRookBlockerMask(square);
    index = MoveTables::getBlockerIndex(rookMask, blockers);
    if (MoveTables::rookMoves[square][index] & (byWhite ? b.wRooks | b.wQueens : b.bRooks | b.bQueens)) return true;

    return false;
}

