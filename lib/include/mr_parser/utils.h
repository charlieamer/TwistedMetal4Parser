#pragma once
#include <vector>
#include <string>
using namespace std;
#include "types.h"
#include "node.h"

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
};
vector<Pos3D> getListOfVerticesForMap(Node* root);

struct MapFaceInfo {
  bool isTransparent;
  bool isShaded;
  vector<int> vertexIndicesInMapSquare;
  uint32_t shaderOffset;
  uint32_t offsetInBuffer;
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
  SquareDrawInfo drawInfos[2];
};
vector<SquareInfo> getListOfSquaresForMap(Node* root);