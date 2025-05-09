#pragma once

#include <string>
#include <glad/gl.h>
#include <unordered_map>

extern std::unordered_map<char, GLuint> pieceTextures;
extern std::unordered_map<char, std::string> pieceToFile;

GLuint loadTextureFromFile(const char* filename);

void loadPieceTextures();
