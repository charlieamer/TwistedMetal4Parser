#pragma once
#include "mr_parser/mr_parser.h"
#include <vector>
#include <map>
#include <algorithm>
using namespace std;

struct VertexWithUV {
  Pos3D pos;
  float u, v;
};

struct VertexWithColorAndUV : VertexWithUV {
  Color col;
  bool hasColor;
  bool operator==(const VertexWithColorAndUV& other) const;
};

struct VertexWithNormalAndUV : VertexWithUV {
  Pos3D normal;
  bool operator==(const VertexWithNormalAndUV& other) const;
};

struct UvRect {
  uint8_t minu, maxu, minv, maxv;
  UvRect(const ShaderInfo& shader, size_t numVertices);
  int getTexCoordIndex(uint8_t u, uint8_t v) const;
  int getTexCoordIndex(const ShaderInfo& shader, size_t vertexIndex) const;
};

template<typename FaceType, typename VertexType>
struct FaceWithExtraInfo {
  FaceType face;
  ShaderInfo shader;
  vector<VertexType> vc;
};

struct CarFaceWithExtraInfo : FaceWithExtraInfo<CarFace, VertexWithNormalAndUV> {};

struct MapFaceWithExtraInfo : FaceWithExtraInfo<MapFaceInfo, VertexWithColorAndUV> {
  string belongingDestroyableName;
};

map<uint32_t, string> getDestroyableFaceIndices(Node* destroyableRoot);
vector<MapFaceWithExtraInfo> buildFacesExtra(
  const vector<SquareInfo>& squares,
  const vector<MapFaceInfo>& faces,
  const vector<Pos3D>& vertices,
  const vector<ShaderInfo>& shaders,
  const map<uint32_t, string>& destroyableFaceIndices
);

template<typename InType, typename OutType>
vector<OutType> extractVertices(const vector<InType>& facesExtra) {
  vector<OutType> ret;
  for (const auto& face : facesExtra) {
    for (const auto& vertex : face.vc) {
      if (find(ret.begin(), ret.end(), vertex) == ret.end()) {
        ret.push_back(vertex);
      }
    }
  }
  return ret;
}