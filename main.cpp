// #include "game.h"
//
// int main() {
//     runInConsole();
// }

#include <bitset>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "game.h"
#include <iostream>
#include <unordered_map>
#include "textures.cpp"

const int BOARD_SIZE = 8;
const float CELL_SIZE = 60.0f;

int selectedX = -1;
int selectedY = -1;
int selectedSquare = -1;
int clickedSquare = -1;
int lastMoveFrom = -1;
int lastMoveTo = -1;

void handleBoardClicks(int x, int y) {
    int square = (7 - y) * 8 + x;

    if (selectedSquare == -1) {
        selectedSquare = square;
    }
    else {
        clickedSquare = square;
        uint16_t nextMove = encodeMove(selectedSquare, clickedSquare, 0);
        std::cout << "Encoded move: 0x" << std::hex << std::bitset<16>{nextMove} << std::endl;

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
}

int main() {
    setup();
    loadPieceTextures();

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Create a windowed mode window and OpenGL context
    GLFWwindow* window = glfwCreateWindow(640, 640, "Chess GUI", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Load OpenGL functions using glad
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Chess Board");
        drawChessboard();
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
