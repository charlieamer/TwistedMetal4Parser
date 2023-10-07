#pragma once
#include "mr_parser/utils.h"

struct PointF {
  float x, y;
};

struct VertexWithUV {
  Pos3D pos;
  float u, v;
  inline void setUV(const PointF& p) {
    u = p.x;
    v = p.y;
  }
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

struct Point {
  int x, y;
};

struct Rect {
  Point topLeft;
  Point bottomRight;
  bool includes(const Point& point) const;
};

template<typename FaceType, typename VertexType>
struct FaceWithExtraInfo {
  FaceType face;
  ShaderInfo shader;
  vector<VertexType> vc;
  size_t belongingIslandIndex;

  Point getAtlasUV(MapTextureHeader textureHeader, int pointIndex) {
    CLUT_MODE clutMode = shader.getClutMode();
    int mulX = (clutMode == CLUT_4_BIT) ? 4 : 2;
    int offY = (clutMode == CLUT_4_BIT) ? 0 : textureHeader.height;
    int x, y;
    switch (pointIndex) {
      case 0:
        x = shader.u1;
        y = shader.v1;
        break;
      case 1:
        x = shader.u2;
        y = shader.v2;
        break;
      case 2:
        x = shader.u3;
        y = shader.v3;
        break;
      case 3:
        x = shader.u4;
        y = shader.v4;
        break;
      default:
        throw invalid_argument("point index must either be 0, 1, 2 or 3");
    }
    x += (shader.getTexturePageX() - textureHeader.offsetX) * mulX;
    y += (shader.getTexturePageY() - textureHeader.offsetY) + offY;
    return {x, y};
  }
};

struct CarFaceWithExtraInfo : FaceWithExtraInfo<CarFace, VertexWithNormalAndUV> {};

struct MapFaceWithExtraInfo : FaceWithExtraInfo<MapFaceInfo, VertexWithColorAndUV> {
  string belongingDestroyableName;
};