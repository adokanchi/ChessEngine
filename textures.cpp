#include "textures.h"
#include <iostream>

std::unordered_map<char, GLuint> pieceTextures;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::unordered_map<char, std::string> pieceToFile = {
    {'P', "wp.png"},
    {'N', "wn.png"},
    {'B', "wb.png"},
    {'R', "wr.png"},
    {'Q', "wq.png"},
    {'K', "wk.png"},
    {'p', "bp.png"},
    {'n', "bn.png"},
    {'b', "bb.png"},
    {'r', "br.png"},
    {'q', "bq.png"},
    {'k', "bk.png"}
};

GLuint loadTextureFromFile(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
    if (!data) {
        std::cerr << "Failed to load image: " << filename << std::endl;
        return 0;
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    return texID;
}

void loadPieceTextures() {
    pieceTextures['P'] = loadTextureFromFile("assets/wp.png");
    pieceTextures['N'] = loadTextureFromFile("assets/wn.png");
    pieceTextures['B'] = loadTextureFromFile("assets/wb.png");
    pieceTextures['R'] = loadTextureFromFile("assets/wr.png");
    pieceTextures['Q'] = loadTextureFromFile("assets/wq.png");
    pieceTextures['K'] = loadTextureFromFile("assets/wk.png");
    pieceTextures['p'] = loadTextureFromFile("assets/bp.png");
    pieceTextures['n'] = loadTextureFromFile("assets/bn.png");
    pieceTextures['b'] = loadTextureFromFile("assets/bb.png");
    pieceTextures['r'] = loadTextureFromFile("assets/br.png");
    pieceTextures['q'] = loadTextureFromFile("assets/bq.png");
    pieceTextures['k'] = loadTextureFromFile("assets/bk.png");
}
