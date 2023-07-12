#include "geometry.h"
#include <math.h>
#include <iostream>
using namespace std;

bool VertexWithColorAndUV::operator==(const VertexWithColorAndUV& other) const {
  return other.col == col && other.pos == pos && other.hasColor == hasColor;
}


bool VertexWithNormalAndUV::operator==(const VertexWithNormalAndUV& other) const {
  return other.pos == pos && other.normal == normal;
}

UvRect::UvRect(const ShaderInfo& shader, size_t numVertices) {
  minu = 255, maxu = 0, minv = 255, maxv = 0;
  minu = min(minu, shader.u1);
  minu = min(minu, shader.u2);
  minu = min(minu, shader.u3);
  if (numVertices == 4)
    minu = min(minu, shader.u4);
  minv = min(minv, shader.v1);
  minv = min(minv, shader.v2);
  minv = min(minv, shader.v3);
  if (numVertices == 4)
    minv = min(minv, shader.v4);
  maxu = max(maxu, shader.u1);
  maxu = max(maxu, shader.u2);
  maxu = max(maxu, shader.u3);
  if (numVertices == 4)
    maxu = max(maxu, shader.u4);
  maxv = max(maxv, shader.v1);
  maxv = max(maxv, shader.v2);
  maxv = max(maxv, shader.v3);
  if (numVertices == 4)
    maxv = max(maxv, shader.v4);
}
int UvRect::getTexCoordIndex(uint8_t u, uint8_t v) const {
  int bitX = (int)round((float)(u - minu) / (float)(maxu - minu));
  if (minu == maxu) {
    bitX = 0;
  }
  int bitY = (int)round((float)(v - minv) / (float)(maxv - minv));
  if (minv == maxv) {
    bitY = 0;
  }
  return ((bitX << 1) | (1 - bitY)) & 0b11;
}
int UvRect::getTexCoordIndex(const ShaderInfo& shader, size_t vertexIndex) const {
  switch(vertexIndex) {
    case 0:
    return getTexCoordIndex(shader.u1, shader.v1);
    case 1:
    return getTexCoordIndex(shader.u2, shader.v2);
    case 2:
    return getTexCoordIndex(shader.u3, shader.v3);
    case 3:
    return getTexCoordIndex(shader.u4, shader.v4);
  }
  return 0;
}



#define VERT_OFFSET(offset) (startingVertexIndex + (uint16_t)offset)

map<uint32_t, string> getDestroyableFaceIndices(Node* destroyableRoot) {
  map<uint32_t, string> ret;
  for (Node child : destroyableRoot->children) {
    if (child.getAttributeByName("displayEmbedded")->getDataAs<uint32_t>()) {
      auto values = child.getAttributeByName("displayFaceInfo")->getDataAsVector<uint32_t>();
      // first 5 numbers are some meta data about sizes
      for (size_t i=5; i<values.size(); i++) {
        ret[values[i]] = child.name;
      }
    }
  }
  return ret;
}

vector<MapFaceWithExtraInfo> buildFacesExtra(
  const vector<SquareInfo>& squares,
  const vector<MapFaceInfo>& faces,
  const vector<Pos3D>& vertices,
  const vector<ShaderInfo>& shaders,
  const map<uint32_t, string>& destroyableFaceIndices
) {
  vector<MapFaceWithExtraInfo> ret;
  for (const SquareInfo& square : squares) {
    const SquareDrawInfo& drawInfo = square.highLOD;
    int startingFaceIndex = getIndexOfFaceByOffset(faces, drawInfo.faceDataOffset);
    if (startingFaceIndex == -1) {
      //throw runtime_error("Starting face not found");
      cerr << "Starting face not found\n";
      continue;
    }
    uint16_t startingVertexIndex = drawInfo.vertexStartIndex;
    for (int i=0; i<drawInfo.numFaces; i++) {
      const MapFaceInfo& face = faces[i + startingFaceIndex];
      MapFaceWithExtraInfo faceExtra;
      faceExtra.face = face;
      faceExtra.shader = shaders[face.shaderOffset / sizeof(ShaderInfo)];
      for (size_t j=0; j<face.vertexIndicesInMapSquare.size(); j++) {
        Pos3D pos = vertices[VERT_OFFSET(face.vertexIndicesInMapSquare[j])];
        bool hasColor = j < face.colors.size();
        VertexWithColorAndUV vc;
        vc.pos = pos;
        vc.col = hasColor ? face.colors[j] : Color();
        vc.hasColor = hasColor;
        faceExtra.vc.push_back(vc);
      }
      const auto destroyableFaceIndex = destroyableFaceIndices.find(faceExtra.face.offsetInBuffer);
      if (destroyableFaceIndex != destroyableFaceIndices.end()) {
        faceExtra.belongingDestroyableName = destroyableFaceIndex->second;
      }
      ret.push_back(faceExtra);
    }
  }
  return ret;
}
