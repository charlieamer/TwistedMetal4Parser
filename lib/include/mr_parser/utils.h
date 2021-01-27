#pragma once
#include <vector>
#include <string>
using namespace std;
#include "types.h"
#include "node.h"

string replaceFileExtension(string fileName, string newExtension);
string getFileName(string fullPath);

template <typename T>
void AppendToBuffer(vector<byte_t> &data, T value)
{
  byte_t buffer[sizeof(value)];
  memcpy(buffer, &value, sizeof(value));
  for (int i = 0; i < sizeof(value); i++)
  {
    data.push_back(buffer[i]);
  }
}

template <typename T>
T GetIntFromBuffer(byte_t* data, size_t offset = 0)
{
  T ret;
  memcpy(&ret, data + offset, sizeof(T));
  return ret;
}

template <typename T>
T GetIntFromBuffer(const vector<byte_t> &data, size_t offset = 0)
{
  return GetIntFromBuffer<T>(data.data(), offset);
}

vector<string> splitString(string str, string sep);

struct Pos3D {
  int16_t x, y, z, unused;

  bool operator==(const Pos3D& other) const {
    return x == other.x && y == other.y && z == other.z;
  }
};
vector<Pos3D> getListOfVerticesForMap(Node* root);

struct Color {
  uint8_t r,g,b;
  bool operator==(const Color& other) const {
    return r == other.r && g == other.g && b == other.b;
  }
  float red() const {
    return r / 255.0f;
  }
  float green() const {
    return g / 255.0f;
  }
  float blue() const {
    return b / 255.0f;
  }
};

struct MapFaceInfo {
  bool isTransparent;
  bool isShaded;
  vector<int> vertexIndicesInMapSquare;
  uint32_t shaderOffset;
  uint32_t offsetInBuffer;
  vector<Color> colors;
};
vector<MapFaceInfo> getListOfFacesForMap(Node* root);
int getIndexOfFaceByOffset(const vector<MapFaceInfo>& faces, uint32_t desiredOffsetInBuffer);

struct SquareDrawInfo {
  byte_t numVertices;
  byte_t numFaces;
  uint16_t vertexStartIndex;
  uint32_t faceDataOffset;
};
struct SquareInfo {
  byte_t unused[0x28];
  // high quality faces
  SquareDrawInfo highLOD;
  // low quality faces
  SquareDrawInfo lowLOD;
};
vector<SquareInfo> getListOfSquaresForMap(Node* root);

enum CLUT_MODE : uint8_t {
  CLUT_4_BIT,
  CLUT_8_BIT,
  CLUT_DIRECT
};
enum SEMI_TRANSPARENCY_MODE : uint8_t {
  TRANSPARENCY_MODE_ALPHA,
  TRANSPARENCY_MODE_FULL,
  TRANSPARENCY_MODE_FULL_INVERTED,
  TRANSPARENCY_MODE_QUARTER
};
struct ShaderInfo {
  uint8_t u1, v1;
  uint16_t pallete;
  uint8_t u2, v2;
  uint16_t texpage;
  uint8_t u3, v3;
  uint8_t u4, v4;

  uint16_t getClutX() const;
  uint16_t getClutY() const;

  CLUT_MODE getClutMode() const;
  SEMI_TRANSPARENCY_MODE getTransparencyMode() const;
  uint16_t getTexturePageX() const;
  uint16_t getTexturePageY() const;
};
vector<ShaderInfo> getListOfShaderInfoForMap(Node* root);

struct MapTextureHeader {
  uint16_t offsetX;
  uint16_t offsetY;
  uint16_t halfWidth;
  uint16_t height;
};

struct RGBA {
  uint8_t R;
  uint8_t G;
  uint8_t B;
  uint8_t A;
};

struct MapTexture {
  MapTextureHeader header;
  vector<vector<byte_t>> data;
};

MapTexture getMapTexture(Node* textureRoot);
MapTexture getMapClut(Node* textureRoot);

// http://www.psxdev.net/forum/viewtopic.php?t=953