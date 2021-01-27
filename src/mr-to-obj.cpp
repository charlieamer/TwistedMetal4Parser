#include "mr_parser/mr_parser.h"
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <lodepng.h>
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

struct VertexWithColor {
  Pos3D pos;
  Color col;
  bool hasColor;
  bool operator==(const VertexWithColor& other) const {
    return other.col == col && other.pos == pos && other.hasColor == hasColor;
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

struct FaceWithExtraInfo {
  MapFaceInfo face;
  ShaderInfo shader;
  vector<VertexWithColor> vc;
};

string textureNameFromShader(const FaceWithExtraInfo& face) {
  UvRect uv(face.shader, face.vc.size());
  char tmp[256];
  snprintf(tmp, 256, "%dx%d_%dx%d_%x_%x",
    uv.minu, uv.minv, uv.maxu, uv.maxv, face.shader.pallete, face.shader.texpage
  );
  return tmp;
}

struct TextureForExport {
  int width, height;
  bool ok;
  vector<RGBA> data;
};

uint8_t highest(uint8_t r, uint8_t g, uint8_t b) {
  return max(max(r, g), b);
}

RGBA getColor(uint16_t color, SEMI_TRANSPARENCY_MODE mode) {
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
  if (!s) {
    return { r, g, b, 255 };
  } else {
    switch(mode) {
      case TRANSPARENCY_MODE_ALPHA:
        return { r, g, b, 127 };
      case TRANSPARENCY_MODE_FULL:
        return { r, g, b, highest(r,g,b) };
      case TRANSPARENCY_MODE_FULL_INVERTED:
        return { r, g, b, (uint8_t)(255 - highest(r,g,b)) };
      case TRANSPARENCY_MODE_QUARTER:
        return { r, g, b, (uint8_t)(highest(r,g,b) >> 2) };
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

TextureForExport textureFromFace(const FaceWithExtraInfo& face, const MapTexture& texture, const MapTexture& clut) {
  TextureForExport ret;
  ret.ok = false;
  CLUT_MODE clutMode = face.shader.getClutMode();
  if (clutMode == CLUT_DIRECT) {
    cerr << "Unsupported CLUT mode" << endl;
    return ret;
  }
  UvRect uv(face.shader, face.vc.size());
  
  ret.width = uv.maxu - uv.minu + 1;
  ret.height = uv.maxv - uv.minv + 1;
  uint16_t lutx = face.shader.getClutX();
  uint16_t luty = face.shader.getClutY();
  vector<uint16_t> lut;
  for (uint16_t i=0; i < ((clutMode == CLUT_4_BIT) ? 16 : 256); i++) {
    uint8_t pixel2 = clut.data[(lutx + i - clut.header.offsetX) * 2][luty - clut.header.offsetY];
    uint8_t pixel1 = clut.data[(lutx + i - clut.header.offsetX) * 2 + 1][luty - clut.header.offsetY];
    lut.push_back(((uint16_t)(pixel1 << 8)) | (uint16_t)pixel2);
  }
  for (int v = 0; v < ret.height; v++) {
    for (int u = 0; u < ret.width; u++) {
      try {
        uint8_t index = getPixelIndex(
          texture, u, v, uv.minu, uv.minv,
          face.shader.getTexturePageX(), face.shader.getTexturePageY(), clutMode
        );
        ret.data.push_back(getColor(lut[index], face.shader.getTransparencyMode()));
      } catch(invalid_coordinates_error) {
        cerr << "Invalid texture coordinates\n";
        return ret;
      }
    }
  }
  if (ret.width * ret.height != ret.data.size()) {
    throw runtime_error("Something went wrong when converting the texture");
  }
  ret.ok = true;
  return ret;
}

int main(int argc, const char *argv[]) {
  if (argc < 2)
  {
    cout << "Usage: " << argv[0] << " file.mr\n";
    return 1;
  }
  Node *root = LoadFromFile(argv[1]);
  
  Node *texturesRoot = LoadFromFile(replaceFileExtension(argv[1], "IMG").c_str());
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

  vector<FaceWithExtraInfo> facesExtra;

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
      FaceWithExtraInfo faceExtra { face, shaders[face.shaderOffset / sizeof(ShaderInfo)] };
      for (size_t j=0; j<face.vertexIndicesInMapSquare.size(); j++) {
        Pos3D pos = vertices[VERT_OFFSET(face.vertexIndicesInMapSquare[j])];
        bool hasColor = j < face.colors.size();
        VertexWithColor vc { pos, hasColor ? face.colors[j] : Color(), hasColor };
        faceExtra.vc.push_back(vc);
      }
      facesExtra.push_back(faceExtra);
    }
  }

  string mapFileName = getFileName(argv[1]);
  string textureOutputFolder = "textures-" + mapFileName;
  system(("mkdir " + textureOutputFolder).c_str());
  for (const auto& face : facesExtra) {
    string fname = textureOutputFolder + "/" + textureNameFromShader(face) + ".png";
    if (!exists(fname.c_str())) {
      TextureForExport exp = textureFromFace(face, texture, clut);
      if (exp.ok) {
        lodepng::encode(fname, (unsigned char*)(void*)exp.data.data(), exp.width, exp.height);
      }
    }
  }
  ofstream out(replaceFileExtension(argv[1], "obj"), ios::binary);
  out << "mtllib " << mapFileName << ".mtl\n";
  out << "vt 0 0\n"
      << "vt 0 1\n"
      << "vt 1 0\n"
      << "vt 1 1\n";
  ofstream outMtl(replaceFileExtension(argv[1], "mtl"), ios::binary);

  // extract vertices
  vector<VertexWithColor> allVertices;
  for (const auto& face : facesExtra) {
    for (const auto& vertex : face.vc) {
      if (find(allVertices.begin(), allVertices.end(), vertex) == allVertices.end()) {
        allVertices.push_back(vertex);
      }
    }
  }
  // output vertices
  for (const auto& vertex : allVertices) {
    out << "v " << -vertex.pos.x / 1000.0f << " " << -vertex.pos.y / 1000.0f << " " << vertex.pos.z / 1000.0f;
    if (vertex.hasColor) {
      out << " " << vertex.col.red() << " " << vertex.col.green() << " " << vertex.col.blue();
    } else {
      out << " 0 0 0";
    }
    out << endl;
  }

  // extract materials
  set<string> createdMaterials;
  for (const auto& face : facesExtra) {
    string materialName = textureNameFromShader(face);
    if (createdMaterials.find(materialName) == createdMaterials.end()) {
      outMtl << "newmtl " << materialName << endl;
      outMtl << "   Ka 1.000 1.000 1.000\n"
             << "   Kd 1.000 1.000 1.000\n"
             << "   Ks 0.000 0.000 0.000\n"
            //  << "   map_Ka " << textureOutputFolder << "/" << materialName << ".png\n"
             << "   map_Kd " << textureOutputFolder << "/" << materialName << ".png\n"
             << "\n";
      createdMaterials.insert(materialName);
    }
  }

  // output faces
  for (const auto& face : facesExtra) {
    out << "usemtl " << textureNameFromShader(face) << endl;
    UvRect uv(face.shader, face.vc.size());
    if (face.face.vertexIndicesInMapSquare.size() == 3) {
      out << "f " << (find(allVertices.begin(), allVertices.end(), face.vc[2]) - allVertices.begin()) + 1 << "/"
                    << uv.getTexCoordIndex(face.shader, 2) + 1 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[1]) - allVertices.begin()) + 1 << "/"
                    << uv.getTexCoordIndex(face.shader, 1) + 1 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[0]) - allVertices.begin()) + 1 << "/"
                    << uv.getTexCoordIndex(face.shader, 0) + 1 << "\n";
    }
    if (face.face.vertexIndicesInMapSquare.size() == 4) {
      // This arrangement is compatible with with most of obj viewers
      // 1 3 2 0
      // 2 0 3 1
      out << "f " << (find(allVertices.begin(), allVertices.end(), face.vc[0]) - allVertices.begin()) + 1 << "/"
                    << uv.getTexCoordIndex(face.shader, 0) + 1 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[2]) - allVertices.begin()) + 1 << "/"
                    << uv.getTexCoordIndex(face.shader, 2) + 1 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[3]) - allVertices.begin()) + 1 << "/"
                    << uv.getTexCoordIndex(face.shader, 3) + 1 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[1]) - allVertices.begin()) + 1 << "/"
                    << uv.getTexCoordIndex(face.shader, 1) + 1 << "\n";
    }
  }
}