#pragma once
#include "mr_parser/mr_parser.h"
#include "geometry.h"
#include "colors.h"
#include <exception>
#include <string>
using namespace std;

string textureNameFromShader(const MapFaceWithExtraInfo& face);
class invalid_coordinates_error : public exception {};
uint8_t getPixelIndex(const MapTexture& texture, int x, int y, int minu, int minv, uint16_t texpageX, uint16_t texpageY, CLUT_MODE clutMode);
void saveImage(vector<vector<RGBA>> pixels, string outputFilename);
void outputTextureError(string text);

template<typename T>
void setFaceUVs(T& face, const MapTextureHeader& textureHeader, int outputTextureWidth, int outputTextureHeight) {
  CLUT_MODE clutMode = face.shader.getClutMode();
  int mulX = (clutMode == CLUT_4_BIT) ? 4 : 2;
  int offY = (clutMode == CLUT_4_BIT) ? 0 : textureHeader.height;
  int x;
  int y;
  float halfPixelW = 1.0f / (float)outputTextureWidth / 2.0f;
  float halfPixelH = 1.0f / (float)outputTextureHeight / 2.0f;
  x = face.shader.u1 + (face.shader.getTexturePageX() - textureHeader.offsetX) * mulX;
  y = face.shader.v1 + (face.shader.getTexturePageY() - textureHeader.offsetY) + offY;
  face.vc[0].u = (float)x / (float)outputTextureWidth + halfPixelW;
  face.vc[0].v = (float)y / (float)outputTextureHeight + halfPixelH;

  x = face.shader.u2 + (face.shader.getTexturePageX() - textureHeader.offsetX) * mulX;
  y = face.shader.v2 + (face.shader.getTexturePageY() - textureHeader.offsetY) + offY;
  face.vc[1].u = (float)x / (float)outputTextureWidth + halfPixelW;
  face.vc[1].v = (float)y / (float)outputTextureHeight + halfPixelH;
  x = face.shader.u3 + (face.shader.getTexturePageX() - textureHeader.offsetX) * mulX;
  y = face.shader.v3 + (face.shader.getTexturePageY() - textureHeader.offsetY) + offY;
  face.vc[2].u = (float)x / (float)outputTextureWidth + halfPixelW;
  face.vc[2].v = (float)y / (float)outputTextureHeight + halfPixelH;
  if (face.vc.size() > 3) {
    x = face.shader.u4 + (face.shader.getTexturePageX() - textureHeader.offsetX) * mulX;
    y = face.shader.v4 + (face.shader.getTexturePageY() - textureHeader.offsetY) + offY;
    face.vc[3].u = (float)x / (float)outputTextureWidth + halfPixelW;
    face.vc[3].v = (float)y / (float)outputTextureHeight + halfPixelH;
  }
}

template<typename T>
void fillTextureFace(const T& face, const MapTexture& texture, const MapTexture& clut, vector<vector<RGBA>>& output, vector<vector<RGBA>>& outputMask) {
  CLUT_MODE clutMode = face.shader.getClutMode();
  if (clutMode == CLUT_DIRECT) {
    outputTextureError("Unsupported CLUT mode");
    return;
  }
  UvRect uv(face.shader, face.vc.size());

  int width = uv.maxu - uv.minu + 1;
  int height = uv.maxv - uv.minv + 1;
  uint16_t lutx = face.shader.getClutX();
  uint16_t luty = face.shader.getClutY();
  vector<uint16_t> lut;
  for (uint16_t i=0; i < ((clutMode == CLUT_4_BIT) ? 16 : 256); i++) {
    uint8_t pixel2 = clut.data[(lutx + i - clut.header.offsetX) * 2][luty - clut.header.offsetY];
    uint8_t pixel1 = clut.data[(lutx + i - clut.header.offsetX) * 2 + 1][luty - clut.header.offsetY];
    lut.push_back(((uint16_t)(pixel1 << 8)) | (uint16_t)pixel2);
  }
  for (int v = 0; v < height; v++) {
    for (int u = 0; u < width; u++) {
      try {
        uint8_t index = getPixelIndex(
          texture, u, v, uv.minu, uv.minv,
          face.shader.getTexturePageX(), face.shader.getTexturePageY(), clutMode
        );
        RGBA color = getColor(lut[index], face.shader.getTransparencyMode(), face.face.isTransparent());
        int mulX = (clutMode == CLUT_4_BIT) ? 4 : 2;
        int offY = (clutMode == CLUT_4_BIT) ? 0 : texture.header.height;
        int x = u + (face.shader.getTexturePageX() - texture.header.offsetX) * mulX + uv.minu;
        int y = v + (face.shader.getTexturePageY() - texture.header.offsetY) + offY + uv.minv;
        output[x][y] = color;
        outputMask[x][y] = {255, 255, 255, 255};
      } catch(invalid_coordinates_error) {
        outputTextureError("Invalid texture coordinates");
        return;
      }
    }
  }
}

template<typename T>
void convertTexture(vector<T>& facesExtra, const MapTexture& texture, const MapTexture& clut, string mapFileName) {
  int width = texture.header.halfWidth * 4;
  int height = texture.header.height * 2;
  vector<vector<RGBA>> atlas(width);
  vector<vector<RGBA>> mask(width);
  for (auto& column : atlas) {
    column.resize(height);
  }
  for (auto& column : mask) {
    column.resize(height);
  }
  for (auto& face : facesExtra) {
    fillTextureFace(face, texture, clut, atlas, mask);
    setFaceUVs(face, texture.header, width, height);
  }
  saveImage(atlas, mapFileName + ".png");
  // saveImage(mask, mapFileName + "-mask.png");
}
