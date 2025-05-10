#include "gui.h"
#include <cstdint>
#include <bitset>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "game.h"
#include <iostream>
#include <unordered_map>
#include "textures.h"

constexpr int BOARD_SIZE = 8;
constexpr float CELL_SIZE = 100.0f;

static int selectedSquare = -1;
static int clickedSquare = -1;
static int lastMoveFrom = -1;
static int lastMoveTo = -1;
static bool awaitingPromotion = false;
static int promotionFrom = -1;
static int promotionTo = -1;
static bool promotionIsWhite = true;

void initGUI() {
    loadPieceTextures();
}

void handleBoardClicks(int x, int y) {
    int square = (7 - y) * 8 + x;

    if (selectedSquare == -1) {
        selectedSquare = square;
    }
    else {
        clickedSquare = square;

        char piece = getPieceAt(selectedSquare);
        bool isWhitePawn = (piece == 'P');
        bool isBlackPawn = (piece == 'p');
        bool isPromotion = (isWhitePawn && clickedSquare / 8 == 7) ||
                           (isBlackPawn && clickedSquare / 8 == 0);

        if (isPromotion) {
            awaitingPromotion = true;
            promotionFrom = selectedSquare;
            promotionTo = clickedSquare;
            promotionIsWhite = isWhitePawn;
        }
        else {
            uint16_t nextMove = encodeMove(selectedSquare, clickedSquare, 0);
            if (move(nextMove, game.state.isWhiteTurn)) {
                lastMoveFrom = selectedSquare;
                lastMoveTo = clickedSquare;
                game.state.isWhiteTurn = !game.state.isWhiteTurn;
            }
            // Reset selection
            selectedSquare = -1;
            clickedSquare = -1;
        }
    }
}

void drawChessboard() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 boardPos = ImGui::GetCursorScreenPos();
    ImVec2 mousePos = ImGui::GetIO().MousePos;
    bool clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            int square = (7 - y) * 8 + x;
            ImVec2 topLeft = ImVec2(boardPos.x + x * CELL_SIZE, boardPos.y + y * CELL_SIZE);
            ImVec2 bottomRight = ImVec2(topLeft.x + CELL_SIZE, topLeft.y + CELL_SIZE);
            bool hovered = mousePos.x >= topLeft.x && mousePos.x < bottomRight.x &&
                           mousePos.y >= topLeft.y && mousePos.y < bottomRight.y;

            // Click handling
            if (clicked && hovered) {
                handleBoardClicks(x, y);
            }

            // Draw base square
            ImU32 color = ((x + y) % 2 == 0)
                            ? IM_COL32(240, 217, 181, 255)  // light square
                            : IM_COL32(181, 136, 99, 255);  // dark square
            drawList->AddRectFilled(topLeft, bottomRight, color);

            // Highlight last move squares
            if (square == lastMoveFrom || square == lastMoveTo) {
                drawList->AddRectFilled(topLeft, bottomRight, IM_COL32(255, 255, 0, 80)); // yellow
            }

            // Highlight selected square
            if (square == selectedSquare) {
                drawList->AddRectFilled(topLeft, bottomRight, IM_COL32(0, 128, 255, 80)); // blue
            }

            char piece = getPieceAt(square);
            if (pieceTextures.count(piece)) {
                ImTextureID tex = (ImTextureID)(intptr_t)pieceTextures[piece];
                ImGui::GetWindowDrawList()->AddImage(tex, topLeft, bottomRight);
            }

        }
    }

    ImGui::Dummy(ImVec2(BOARD_SIZE * CELL_SIZE, BOARD_SIZE * CELL_SIZE));

    if (awaitingPromotion) {
        ImGui::OpenPopup("Promote to...");
    }

    // Calculate popup position centered on the board
    // ImVec2 popupSize(CELL_SIZE * 4.4f, CELL_SIZE * 1.4f);  // Adjust as needed
    float popupWidth  = CELL_SIZE * 4.4f + 20.0f;  // 4 pieces + spacing
    float popupHeight = CELL_SIZE * 1.4f + 20.0f;  // taller to give top/bottom room
    ImVec2 popupSize(popupWidth, popupHeight);
    ImVec2 boardTopLeft = ImGui::GetWindowPos();
    ImVec2 popupPos = ImVec2(
        boardTopLeft.x + (BOARD_SIZE * CELL_SIZE - popupSize.x) / 2.0f,
        boardTopLeft.y + (BOARD_SIZE * CELL_SIZE - popupSize.y) / 2.0f
    );

    ImGui::SetNextWindowPos(popupPos, ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Promote to...", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
        float iconSize = CELL_SIZE * 1.1f;

        auto promoteWith = [&](const char* label, char pieceChar, int promoCode) {
            if (pieceTextures.count(pieceChar)) {
                ImGui::PushID(label); // unique ID to prevent ImGui collision
                if (ImGui::ImageButton(label, (ImTextureID)(intptr_t)pieceTextures[pieceChar], ImVec2(iconSize, iconSize))) {
                    uint16_t nextMove = encodeMove(promotionFrom, promotionTo, promoCode);
                    if (move(nextMove, game.state.isWhiteTurn)) {
                        lastMoveFrom = promotionFrom;
                        lastMoveTo = promotionTo;
                        game.state.isWhiteTurn = !game.state.isWhiteTurn;
                    }
                    awaitingPromotion = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopID();
                ImGui::SameLine(0.0f, 12.0f);  // Adds spacing between buttons
            }
        };

        promoteWith("Queen", promotionIsWhite ? 'Q' : 'q', 4);
        promoteWith("Rook",  promotionIsWhite ? 'R' : 'r', 3);
        promoteWith("Bishop",promotionIsWhite ? 'B' : 'b', 2);
        promoteWith("Knight",promotionIsWhite ? 'N' : 'n', 1);

        ImGui::NewLine();
        ImGui::EndPopup();
    }



}