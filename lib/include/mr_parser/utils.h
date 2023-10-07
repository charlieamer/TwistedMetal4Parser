#pragma once
#include <vector>
#include <string>
#include <string.h>
#include <algorithm>
#include <filesystem>
#include "types.h"
#include "node.h"

#if defined(_WIN32)
#include <direct.h>
// MSDN recommends against using getcwd & chdir names
#define cwd _getcwd
#define cd _chdir
#define mkd _mkdir
#else
#include "unistd.h"
#include <sys/stat.h>
#define cwd getcwd
#define cd chdir
#define mkd(path) mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif

using namespace std;
using namespace std::filesystem;

string removeFileExtension(string filename);
string replaceFileExtension(string fileName, string newExtension);
string getFileName(string fullPath);

void writeBytesToFile(ostream& out, vector<byte_t> bytes);

template <typename T>
vector<vector<T>> make2Dvector(int width, int height) {
  vector<vector<T>> ret;
  ret.resize(width);
  for (auto& column : ret) {
    column.resize(height);
  }
  return ret;
}


template <typename T>
inline bool inVector(const vector<T>& vec, const T& v) {
  return find(vec.begin(), vec.end(), v) != vec.end();
}

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
vector<byte_t> loadFileToBuffer(const char* path);

int convertStringToInt(const string& str, path forOutput, int defaultValue, int vmin, int vmax);

struct Pos3D {
  int16_t x, y, z, unused;

  bool operator==(const Pos3D& other) const {
    return x == other.x && y == other.y && z == other.z;
  }
};
vector<Pos3D> getListOfVerticesForMap(Node* root);
vector<Pos3D> getListOfVerticesForCar(Node* root);

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
  bool isTransparent() const;
  bool isShaded() const;
  byte_t specialByte;
  vector<int> vertexIndicesInMapSquare;
  uint32_t shaderOffset;
  uint32_t offsetInBuffer;
  vector<Color> colors;
};
vector<MapFaceInfo> getListOfFacesForMap(Node* root);

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

struct CarFace {
  byte_t faceSpecialByte;
  byte_t unused0[3];
  ShaderInfo shader;
  byte_t vertexOffsets[4];
  byte_t normalOffsets[4];
  int getNumVertices() const;
  bool isTransparent() const;
};
vector<CarFace> getListOfFacesForCar(Node* root);
int getIndexOfFaceByOffset(const vector<MapFaceInfo>& faces, uint32_t desiredOffsetInBuffer);

struct MapTextureHeader {
  uint16_t offsetX;
  uint16_t offsetY;
  uint16_t halfWidth;
  uint16_t height;
};

struct RGBA {
  uint8_t R = 0;
  uint8_t G = 0;
  uint8_t B = 0;
  uint8_t A = 0;
};

struct MapTexture {
  MapTextureHeader header;
  vector<vector<byte_t>> data;
  vector<byte_t> convertToBytes();
};

MapTexture genericGetMapTexture(const vector<byte_t>& bytes);
MapTexture getMapTexture(Node* textureRoot);
// http://www.psxdev.net/forum/viewtopic.php?t=953
MapTexture getMapClut(Node* textureRoot);
MapTexture getCarTexture(Node* root);

struct CarVertexGroupInfo {
  uint16_t startFaceIndex;
  uint16_t faceCount;
  uint16_t startVertexIndex;
  uint16_t vertexCount;
  uint16_t vertexNormalIndex;
  uint16_t vertexNormalCount;
};

struct CarVertexGroup {
  CarVertexGroupInfo HighHPInfo;
  CarVertexGroupInfo LowHPInfo;
  Pos3D offset3D;
};

struct VertexNormal {
  int16_t x, y, z;
  uint16_t vertexOffset;
};
vector<VertexNormal> getCarVertexNormals(Node* root);