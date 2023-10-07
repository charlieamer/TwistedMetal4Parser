#pragma once
#include "mr_parser/mr_parser.h"
#include <vector>
#include <map>
#include <algorithm>
#include "graphics-structs.h"
using namespace std;

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