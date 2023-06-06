#include "mr_parser/mr_parser.h"
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <lodepng.h>
#include <math.h>
#include <algorithm>
using namespace std;

#define VERT_OFFSET(offset) (startingVertexIndex + (uint16_t)offset)

bool exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return true;
    }
    return false;
}

struct VertexWithUV {
  Pos3D pos;
  float u, v;
};

struct VertexWithColorAndUV : VertexWithUV {
  Color col;
  bool hasColor;
  bool operator==(const VertexWithColorAndUV& other) const {
    return other.col == col && other.pos == pos && other.hasColor == hasColor;
  }
};

struct VertexWithNormalAndUV : VertexWithUV {
  Pos3D normal;
  bool operator==(const VertexWithNormalAndUV& other) const {
    return other.pos == pos && other.normal == normal;
  }
};

struct UvRect {
  uint8_t minu, maxu, minv, maxv;
  UvRect(const ShaderInfo& shader, size_t numVertices) {
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
  int getTexCoordIndex(uint8_t u, uint8_t v) const {
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
  int getTexCoordIndex(const ShaderInfo& shader, size_t vertexIndex) const {
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
};

template<typename FaceType, typename VertexType>
struct FaceWithExtraInfo {
  FaceType face;
  ShaderInfo shader;
  vector<VertexType> vc;
};

#define GENERIC_FACE FaceWithExtraInfo<typename FaceType, typename VertexType>

struct CarFaceWithExtraInfo : FaceWithExtraInfo<CarFace, VertexWithNormalAndUV> {};

struct MapFaceWithExtraInfo : FaceWithExtraInfo<MapFaceInfo, VertexWithColorAndUV> {
  string belongingDestroyableName;
};

string textureNameFromShader(const MapFaceWithExtraInfo& face) {
  UvRect uv(face.shader, face.vc.size());
  char tmp[256];
  snprintf(tmp, 256, "%dx%d_%dx%d_%x_%x",
    uv.minu, uv.minv, uv.maxu, uv.maxv, face.shader.pallete, face.shader.texpage
  );
  return tmp;
}

uint8_t highest(uint8_t r, uint8_t g, uint8_t b) {
  return max(max(r, g), b);
}

RGBA getColor(uint16_t color, SEMI_TRANSPARENCY_MODE mode, bool isFaceTransparent) {
  if (color == 0) {
    return {0,0,0,0};
  }
  uint8_t r = color & 0b00000000'00011111;
  uint8_t g =(color & 0b00000011'11100000) >> 5;
  uint8_t b =(color & 0b01111100'00000000) >> 10;
  uint8_t s =(color & 0b10000000'00000000) >> 15;
  r = (uint8_t)(r << 3);
  g = (uint8_t)(g << 3);
  b = (uint8_t)(b << 3);
  if (!s || !isFaceTransparent) {
    return { r, g, b, 255 };
  } else {
    switch(mode) {
      case TRANSPARENCY_MODE_ALPHA:
        return { r, g, b, 127 };
      case TRANSPARENCY_MODE_FULL:
        return { r, g, b, (uint8_t)min(highest(r,g,b) * highest(r,g,b) / 256 * 4, 255) };
      case TRANSPARENCY_MODE_FULL_INVERTED:
        return { r, g, b, (uint8_t)(255 - highest(r,g,b)) };
      case TRANSPARENCY_MODE_QUARTER:
        return { r, g, b, (uint8_t)(highest(r,g,b) * highest(r,g,b) / 256) };
    }
  }
  return { 255, 0, 255, 255 };
}

class invalid_coordinates_error : public exception {};

uint8_t getPixelIndex(const MapTexture& texture, int x, int y, int minu, int minv, uint16_t texpageX, uint16_t texpageY, CLUT_MODE clutMode) {
  if (clutMode == CLUT_4_BIT) {
    int xx = texpageX * 2 + x / 2 + minu / 2 - texture.header.offsetX * 2;
    int yy = texpageY + minv + y - texture.header.offsetY;
    if (xx < 0 || xx >= texture.header.halfWidth * 2 || yy < 0 || yy > texture.header.height) {
      throw invalid_coordinates_error();
    }
    uint8_t value = texture.data[xx][yy];
    if (x % 2 == 1) {
      return (uint8_t)(value >> 4);
    } else {
      return value & 0xF;
    }
  } else {
    int xx = texpageX * 2 + x + minu - texture.header.offsetX;
    int yy = texpageY + minv + y - texture.header.offsetY;
    if (xx < 0 || xx >= texture.header.halfWidth * 2 || yy < 0 || yy > texture.header.height) {
      throw invalid_coordinates_error();
    }
    uint8_t value = texture.data[xx][yy];
    return value;
  }
}

int numberOfLowerValues(int cmp, int a, int b, int c) {
  int ret = 0;
  ret += cmp > a;
  ret += cmp > b;
  ret += cmp > c;
  return ret;
}

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
    cerr << "Unsupported CLUT mode" << endl;
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
        cerr << "Invalid texture coordinates\n";
        return;
      }
    }
  }
}

void saveImage(vector<vector<RGBA>> pixels, string outputFilename) {
  int width = pixels.size();
  int height = (width > 0) ? pixels[0].size() : 0;
  vector<RGBA> outputArray(width * height);
  int pixelIndex = 0;
  for (int y=0; y<height; y++) {
    for (int x=0; x<width; x++) {
      outputArray[pixelIndex++] = pixels[x][y];
    }
  }
  lodepng::encode(outputFilename, (unsigned char*)(void*)outputArray.data(), width, height);
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

void outputVerticesWithColorAndUVs(ostream& out, const vector<VertexWithColorAndUV>& vertices) {
  for (const auto& vertex : vertices) {
    out << "v " << -vertex.pos.x / 1000.0f << " " << -vertex.pos.y / 1000.0f << " " << vertex.pos.z / 1000.0f;
    if (vertex.hasColor) {
      out << " " << vertex.col.red() << " " << vertex.col.green() << " " << vertex.col.blue();
    } else {
      out << " 0 0 0";
    }
    out << endl;
  }
}

void outputVerticesWithNormalAndUVs(ostream& out, const vector<VertexWithNormalAndUV>& vertices) {
  for (const auto& vertex : vertices) {
    out << "v " << -vertex.pos.x / 10.0f << " " << -vertex.pos.y / 10.0f << " " << vertex.pos.z / 10.0f;
    out << endl;
  }
}

void outputMaterial(string mrFilePath) {
  ofstream outMtl(replaceFileExtension(mrFilePath, "mtl"), ios::binary);
  outMtl  << "newmtl atlas\n"
          << "   Ka 1.000 1.000 1.000\n"
          << "   Kd 1.000 1.000 1.000\n"
          << "   Ks 0.000 0.000 0.000\n"
          << "   map_Kd " << getFileName(mrFilePath) << ".png\n"
          << "\n";
}

template<typename T>
void outputUVs(ostream& out, const vector<T> facesExtra) {
  for (const auto& face : facesExtra) {
    UvRect uv(face.shader, face.vc.size());
    if (face.vc.size() == 3) {
      out << "vt " << face.vc[2].u << " " << 1.0 - face.vc[2].v << endl;
      out << "vt " << face.vc[1].u << " " << 1.0 - face.vc[1].v << endl;
      out << "vt " << face.vc[0].u << " " << 1.0 - face.vc[0].v << endl;
    }
    if (face.vc.size() == 4) {
      out << "vt " << face.vc[0].u << " " << 1.0 - face.vc[0].v << endl;
      out << "vt " << face.vc[2].u << " " << 1.0 - face.vc[2].v << endl;
      out << "vt " << face.vc[3].u << " " << 1.0 - face.vc[3].v << endl;
      out << "vt " << face.vc[1].u << " " << 1.0 - face.vc[1].v << endl;
    }
  }
}

void outputNormals(ostream& out, const vector<CarFaceWithExtraInfo> facesExtra) {
  for (const auto& face : facesExtra) {
    UvRect uv(face.shader, face.vc.size());
    if (face.vc.size() == 3) {
      out << "vn " << -face.vc[2].normal.x << " " << -face.vc[2].normal.y << " " << face.vc[2].normal.z << endl;
      out << "vn " << -face.vc[1].normal.x << " " << -face.vc[1].normal.y << " " << face.vc[1].normal.z << endl;
      out << "vn " << -face.vc[0].normal.x << " " << -face.vc[0].normal.y << " " << face.vc[0].normal.z << endl;
    }
    if (face.vc.size() == 4) {
      out << "vn " << -face.vc[0].normal.x << " " << -face.vc[0].normal.y << " " << face.vc[0].normal.z << endl;
      out << "vn " << -face.vc[2].normal.x << " " << -face.vc[2].normal.y << " " << face.vc[2].normal.z << endl;
      out << "vn " << -face.vc[3].normal.x << " " << -face.vc[3].normal.y << " " << face.vc[3].normal.z << endl;
      out << "vn " << -face.vc[1].normal.x << " " << -face.vc[1].normal.y << " " << face.vc[1].normal.z << endl;
    }
  }
}

void outputFaces(ostream& out, const vector<MapFaceWithExtraInfo> facesExtra, const vector<VertexWithColorAndUV> allVertices) {
  int uvCount = 0;
  string lastDestroyableName = "NO GROUP BY DEFAULT";
  for (const auto& face : facesExtra) {
    UvRect uv(face.shader, face.vc.size());
    if (face.belongingDestroyableName != lastDestroyableName) {
      string groupForOutput = (face.belongingDestroyableName.length() == 0) ? "default" : face.belongingDestroyableName;
      out << "g " << groupForOutput << endl;
    }
    lastDestroyableName = face.belongingDestroyableName;

    if (face.face.vertexIndicesInMapSquare.size() == 3) {
      out << "f " << (find(allVertices.begin(), allVertices.end(), face.vc[2]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 1 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[1]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 2 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[0]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 3 << "\n";
      uvCount += 3;
    }
    if (face.face.vertexIndicesInMapSquare.size() == 4) {
      out << "f " << (find(allVertices.begin(), allVertices.end(), face.vc[0]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 1 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[2]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 2 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[3]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 3 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[1]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 4 << "\n";
      uvCount += 4;
    }
  }
}

void outputFaces(ostream& out, const vector<CarFaceWithExtraInfo> facesExtra, const vector<VertexWithNormalAndUV> allVertices, int faceOffset, int numFaces) {
  int uvCount = 0;
  for (int i=0; i<faceOffset; i++) {
    uvCount += facesExtra[i].vc.size();
  }
  for (int i=faceOffset; i < faceOffset + numFaces; i++) {
    const auto& face = facesExtra[i];
    UvRect uv(face.shader, face.vc.size());
    if (face.vc.size() == 3) {
      out << "f " << (find(allVertices.begin(), allVertices.end(), face.vc[2]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 1 << "/" << uvCount + 1 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[1]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 2 << "/" << uvCount + 2 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[0]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 3 << "/" << uvCount + 3 << "\n";
      uvCount += 3;
    }
    if (face.vc.size() == 4) {
      out << "f " << (find(allVertices.begin(), allVertices.end(), face.vc[0]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 1 << "/" << uvCount + 1 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[2]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 2 << "/" << uvCount + 2 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[3]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 3 << "/" << uvCount + 3 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[1]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 4 << "/" << uvCount + 4 << "\n";
      uvCount += 4;
    }
  }
}

void outputWeaponList(ofstream& out, Node* weaponRoot) {
  for (Node& child : weaponRoot->children) {
    const auto pos = child.getAttributeByName("pos")->getDataAsVector<int16_t>();
    out << child.name << " " <<
           child.getAttributeByName("type")->getDataAsString() << " " <<
           -pos[0]/1000.0 << " " << -pos[1]/1000.0 << " " << pos[2]/1000.0 << endl;
  }
}

void tryConvertMapFile(string mrPath) {
  Node *root = LoadFromFile(mrPath.c_str());
  
  Node *texturesRoot = LoadFromFile(replaceFileExtension(mrPath, "IMG").c_str());
  MapTexture texture = getMapTexture(texturesRoot);
  MapTexture clut = getMapClut(texturesRoot);
  cout << "Map texture: " << texture.header.halfWidth * 2 << " x " << texture.header.height << endl;

  vector<Pos3D> vertices = getListOfVerticesForMap(root);
  cout << vertices.size() << " vertices\n";
  vector<MapFaceInfo> faces = getListOfFacesForMap(root);
  cout << faces.size() << " faces\n";
  vector<SquareInfo> squares = getListOfSquaresForMap(root);
  cout << squares.size() << " squares\n";
  vector<ShaderInfo> shaders = getListOfShaderInfoForMap(root);
  cout << shaders.size() << " shader infos\n";

  map<uint32_t, string> destroyableFaceIndices = getDestroyableFaceIndices(root->getChildByPath("objects/destroyable/instances/startup"));
  vector<MapFaceWithExtraInfo> facesExtra = buildFacesExtra(squares, faces, vertices, shaders, destroyableFaceIndices);

  string mapFileName = getFileName(mrPath);
  convertTexture(facesExtra, texture, clut, mapFileName);

  outputMaterial(mrPath);

  ofstream out(replaceFileExtension(mrPath, "obj"), ios::binary);
  out << "mtllib " << mapFileName << ".mtl\n";

  // extract vertices
  vector<VertexWithColorAndUV> allVertices = extractVertices<MapFaceWithExtraInfo, VertexWithColorAndUV>(facesExtra);
  // output vertices
  outputVerticesWithColorAndUVs(out, allVertices);

  out << "usemtl atlas\n";

  outputUVs(out, facesExtra);
  outputFaces(out, facesExtra, allVertices);

  ofstream weaponsOut(replaceFileExtension(mrPath, "txt"));
  outputWeaponList(weaponsOut, root->getChildByPath("objects/powerup/instances/startup"));
}

bool shouldGroupBeTranslated(string groupName) {
  vector<string> translatedStarts {"rr_wheel", "rl_wheel", "fr_wheel", "fl_wheel"};
  for (string start : translatedStarts) {
    if (groupName.rfind(start, 0) == 0) {
      return true;
    }
  }
  return false;
}

map<string, vector<CarFaceWithExtraInfo>> buildCarFacesExtra(
  const vector<Pos3D>& vertices,
  const vector<CarFace>& faces,
  const vector<VertexNormal>& normals,
  Node* root,
  bool extractHighHP
) {
  map<string, vector<CarFaceWithExtraInfo>> ret;
  for (const auto& groupComponent : root->getChildByPath("display/resolution/r0/Groups")->components) {
    CarVertexGroup group = groupComponent.getDataAs<CarVertexGroup>();
    CarVertexGroupInfo groupInfo = extractHighHP ? group.HighHPInfo : group.LowHPInfo;
    vector<CarFaceWithExtraInfo> vec;
    for (int i=0; i<groupInfo.faceCount; i++) {
      CarFaceWithExtraInfo face;
      face.face = faces[groupInfo.startFaceIndex + i];
      face.shader = face.face.shader;
      for (int j=0; j<face.face.getNumVertices(); j++) {
        VertexWithNormalAndUV v;
        v.pos = vertices[face.face.vertexOffsets[j] + groupInfo.startVertexIndex];
        int offsetMul = shouldGroupBeTranslated(groupComponent.name);
        v.pos.x += group.offset3D.x * offsetMul;
        v.pos.y += group.offset3D.y * offsetMul;
        v.pos.z += group.offset3D.z * offsetMul;
        const VertexNormal& normal = normals[face.face.normalOffsets[j] + groupInfo.vertexNormalIndex];
        v.normal.x = normal.x;
        v.normal.y = normal.y;
        v.normal.z = normal.z;
        face.vc.push_back(v);
      }
      vec.push_back(face);
    }
    ret[groupComponent.name] = vec;
  }
  return ret;
}

void tryConvertCarFile(string mrPath, bool extractHighHP) {
  Node *root = LoadFromFile(mrPath.c_str());

  if (!extractHighHP) {
    mrPath = removeFileExtension(mrPath) + "-lowHP.MR";
  }

  vector<Pos3D> vertices = getListOfVerticesForCar(root);
  vector<CarFace> faces = getListOfFacesForCar(root);
  vector<VertexNormal> normals = getCarVertexNormals(root);

  cout << vertices.size() << " vertices\n";
  cout << faces.size() << " faces\n";
  
  ofstream out(replaceFileExtension(mrPath, "obj"), ios::binary);
  auto facesExtra = buildCarFacesExtra(vertices, faces, normals, root, extractHighHP);
  vector<CarFaceWithExtraInfo> allFaces;
  for (const auto& it : facesExtra) {
    allFaces.insert(allFaces.end(), it.second.begin(), it.second.end());
  }
  MapTexture texture = getCarTexture(root);

  string mapFileName = getFileName(mrPath);
  convertTexture(allFaces, texture, texture, removeFileExtension(mrPath));
  out << "mtllib " << mapFileName << ".mtl\n";
  out << "usemtl atlas\n";

  outputMaterial(mrPath);

  // extract vertices
  vector<VertexWithNormalAndUV> allExtractedVertices = extractVertices<CarFaceWithExtraInfo, VertexWithNormalAndUV>(allFaces);
  // output vertices
  outputVerticesWithNormalAndUVs(out, allExtractedVertices);

  outputNormals(out, allFaces);
  outputUVs(out, allFaces);
  int currentFaceIndex = 0;
  for (const auto& it : facesExtra) {
    out << "o " << it.first << endl;
    outputFaces(out, allFaces, allExtractedVertices, currentFaceIndex, it.second.size());
    currentFaceIndex += it.second.size();
  }
}

int main(int argc, const char *argv[]) {
  if (argc < 2)
  {
    cout << "Usage: " << argv[0] << " file.mr\n";
    return 1;
  }

  try {
    cout << "Trying to open as map.\n";
    tryConvertMapFile(argv[1]);
    cout << "Successfuly converted file as map.\n";
    return 0;
  } catch(const exception &ex) {
    cerr << "Error opening the file as map: " << ex.what() << endl;
    try {
      cout << "Trying to open as car.\n";
      cout << "High HP\n";
      tryConvertCarFile(argv[1], true);
      cout << "Low HP\n";
      tryConvertCarFile(argv[1], false);
      cout << "Successfuly converted file as car.\n";
    } catch(const exception& ex) {
      cerr << "Error opening file as car: " << ex.what() << endl;
    } 
    return 0;
  }
  return 1;
}