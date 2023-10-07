#pragma once
#include "mr_parser/mr_parser.h"
#include "geometry.h"
#include "colors.h"
#include <exception>
#include <string>
#include "graphics-structs.h"
using namespace std;

string textureNameFromShader(const MapFaceWithExtraInfo& face);
class invalid_coordinates_error : public exception {};
uint8_t getPixelIndex(const MapTexture& texture, int x, int y, int minu, int minv, uint16_t texpageX, uint16_t texpageY, CLUT_MODE clutMode);
void saveImage(vector<vector<RGBA>> pixels, string outputFilename);
void outputTextureError(string text);
vector<Rect> findIslands(const vector<vector<RGBA>> &mask);
vector<vector<RGBA>> cutTexture(const vector<vector<RGBA>>& image, const Rect& rect);
int getBelongingIslandIndex(const vector<Rect>& rects, const Point& point);
PointF relativeCoordinates(const Rect& rect, const Point& point);

template<typename T>
void setFaceUVs(T& face, const MapTextureHeader& textureHeader,
  int outputTextureWidth, int outputTextureHeight, const vector<Rect>& islands
) {
  CLUT_MODE clutMode = face.shader.getClutMode();
  int mulX = (clutMode == CLUT_4_BIT) ? 4 : 2;
  int offY = (clutMode == CLUT_4_BIT) ? 0 : textureHeader.height;
  Rect island;
  Point p;
  int &x = p.x;
  int &y = p.y;

  float pixelW = 1.0f / (float)outputTextureWidth;
  float pixelH = 1.0f / (float)outputTextureHeight;

  p = face.getAtlasUV(textureHeader, 0);
  face.belongingIslandIndex = getBelongingIslandIndex(islands, p);
  island = islands[face.belongingIslandIndex];

  p = face.getAtlasUV(textureHeader, 0);
  face.vc[0].setUV(relativeCoordinates(island, p));
  p = face.getAtlasUV(textureHeader, 1);
  face.vc[1].setUV(relativeCoordinates(island, p));
  p = face.getAtlasUV(textureHeader, 2);
  face.vc[2].setUV(relativeCoordinates(island, p));
  if (face.vc.size() > 3) {
    p = face.getAtlasUV(textureHeader, 3);
    face.vc[3].setUV(relativeCoordinates(island, p));
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