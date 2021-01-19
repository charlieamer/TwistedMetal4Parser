#include "mr_parser/mr_parser.h"
#include <iostream>
#include <fstream>
#include <map>
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

struct FaceWithExtraInfo {
  MapFaceInfo face;
  ShaderInfo shader;
  vector<VertexWithColor> vc;
};

string textureNameFromShader(const FaceWithExtraInfo& face) {
  uint8_t minu = 255, maxu = 0, minv = 255, maxv = 0;
  minu = min(minu, face.shader.u1);
  minu = min(minu, face.shader.u2);
  minu = min(minu, face.shader.u3);
  if (face.vc.size() == 4)
    minu = min(minu, face.shader.u4);
  minv = min(minv, face.shader.v1);
  minv = min(minv, face.shader.v2);
  minv = min(minv, face.shader.v3);
  if (face.vc.size() == 4)
    minv = min(minv, face.shader.v4);
  maxu = max(maxu, face.shader.u1);
  maxu = max(maxu, face.shader.u2);
  maxu = max(maxu, face.shader.u3);
  if (face.vc.size() == 4)
    maxu = max(maxu, face.shader.u4);
  maxv = max(maxv, face.shader.v1);
  maxv = max(maxv, face.shader.v2);
  maxv = max(maxv, face.shader.v3);
  if (face.vc.size() == 4)
    maxv = max(maxv, face.shader.v4);
  char tmp[256];
  snprintf(tmp, 256, "textures/%dx%d_%dx%d_%x_%x.png",
    minu, minv, maxu, maxv, face.shader.pallete, face.shader.texpage
  );
  if (strcmp(tmp, "textures/128x128_190x190_3c55_13.png") == 0) {
    return tmp;
  }
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

TextureForExport textureFromFace(FaceWithExtraInfo face, MapTexture texture, MapTexture clut) {
  TextureForExport ret;
  ret.ok = false;
  if (face.face.shaderOffset == 0x2AA38) {
    ret.ok = false;
  }
  CLUT_MODE clutMode = face.shader.getClutMode();
  if (clutMode == CLUT_DIRECT) {
    cerr << "Unsupported CLUT mode" << endl;
    return ret;
  }
  uint8_t minu = 255, maxu = 0, minv = 255, maxv = 0;
  minu = min(minu, face.shader.u1);
  minu = min(minu, face.shader.u2);
  minu = min(minu, face.shader.u3);
  if (face.vc.size() == 4)
    minu = min(minu, face.shader.u4);
  minv = min(minv, face.shader.v1);
  minv = min(minv, face.shader.v2);
  minv = min(minv, face.shader.v3);
  if (face.vc.size() == 4)
    minv = min(minv, face.shader.v4);
  maxu = max(maxu, face.shader.u1);
  maxu = max(maxu, face.shader.u2);
  maxu = max(maxu, face.shader.u3);
  if (face.vc.size() == 4)
    maxu = max(maxu, face.shader.u4);
  maxv = max(maxv, face.shader.v1);
  maxv = max(maxv, face.shader.v2);
  maxv = max(maxv, face.shader.v3);
  if (face.vc.size() == 4)
    maxv = max(maxv, face.shader.v4);
  
  ret.width = maxu - minu + 1;
  int actualWidth = ret.width;
  if (clutMode == CLUT_4_BIT) {
    actualWidth /= 4;
    actualWidth++;
    ret.width = actualWidth * 4;
  }
  if (clutMode == CLUT_8_BIT) {
    actualWidth /= 2;
    actualWidth++;
    ret.width = actualWidth * 2;
  }
  ret.height = maxv - minv + 1;
  uint16_t lutx = face.shader.getClutX();
  uint16_t luty = face.shader.getClutY();
  vector<uint16_t> lut;
  for (uint16_t i=0; i < ((clutMode == CLUT_4_BIT) ? 16 : 256); i++) {
    uint8_t pixel2 = clut.data[(lutx + i - clut.header.offsetX) * 2][luty - clut.header.offsetY];
    uint8_t pixel1 = clut.data[(lutx + i - clut.header.offsetX) * 2 + 1][luty - clut.header.offsetY];
    lut.push_back(((uint16_t)(pixel1 << 8)) | (uint16_t)pixel2);
  }
  for (int v = 0; v < ret.height; v++) {
    for (int u = 0; u < actualWidth; u++) {
      int x = (u + (int)face.shader.getTexturePageX() + minu) * 2 - texture.header.offsetX;
      int y = v + (int)face.shader.getTexturePageY() + minv - texture.header.offsetY;
      if (x < 0 || x >= texture.header.halfWidth * 2 || y < 0 || y > texture.header.height) {
        cerr << "Invalid texture coordinates: " << x << "x" << y << endl;
        return ret;
      }
      uint8_t pixel1 = texture.data[x][y];
      uint8_t pixel2 = texture.data[x+1][y];
      if (clutMode == CLUT_4_BIT) {
        uint8_t lut1 = pixel1 & 0x0F;
        uint8_t lut2 = (pixel1 & 0xF0) >> 4;
        uint8_t lut3 = pixel2 & 0x0F;
        uint8_t lut4 = (pixel2 & 0xF0) >> 4;
        ret.data.push_back(getColor(lut[lut1], face.shader.getTransparencyMode()));
        ret.data.push_back(getColor(lut[lut2], face.shader.getTransparencyMode()));
        ret.data.push_back(getColor(lut[lut3], face.shader.getTransparencyMode()));
        ret.data.push_back(getColor(lut[lut4], face.shader.getTransparencyMode()));
      }
      if (clutMode == CLUT_8_BIT) {
        ret.data.push_back(getColor(lut[pixel1], face.shader.getTransparencyMode()));
        ret.data.push_back(getColor(lut[pixel2], face.shader.getTransparencyMode()));
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
  // ofstream out(replaceFileExtension(argv[1], "obj"), ios::binary);
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
    } // 00 80 A1 3C 3E 80 15 00 00 BE 3E BE
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

  // for (const SquareInfo& square : squares) {
  //   const SquareDrawInfo& drawInfo = square.highLOD;
  //   int startingFaceIndex = getIndexOfFaceByOffset(faces, drawInfo.faceDataOffset);
  //   uint16_t startingVertexIndex = drawInfo.vertexStartIndex;
  //   for (int i=0; i<drawInfo.numFaces; i++) {
  //     const MapFaceInfo& face = faces[i + startingFaceIndex];
  //     if (face.vertexIndicesInMapSquare.size() == 3) {
  //       out << "f " << VERT_OFFSET(face.vertexIndicesInMapSquare[0]) << " " <<
  //                       VERT_OFFSET(face.vertexIndicesInMapSquare[1]) << " " <<
  //                       VERT_OFFSET(face.vertexIndicesInMapSquare[2]) << endl;
  //     }
  //     if (face.vertexIndicesInMapSquare.size() == 4) {
  //       // This arrangement is compatible with with most of obj viewers
  //       out << "f " << VERT_OFFSET(face.vertexIndicesInMapSquare[1]) << " " <<
  //                       VERT_OFFSET(face.vertexIndicesInMapSquare[3]) << " " <<
  //                       VERT_OFFSET(face.vertexIndicesInMapSquare[2]) << " " <<
  //                       VERT_OFFSET(face.vertexIndicesInMapSquare[0]) << endl;
  //     }
  //   }
  // }

  system("mkdir textures");
  for (const auto& face : facesExtra) {
    string fname = textureNameFromShader(face);
    if (!exists(fname.c_str())) {
      TextureForExport exp = textureFromFace(face, texture, clut);
      if (exp.ok) {
        lodepng::encode(fname, (unsigned char*)(void*)exp.data.data(), exp.width, exp.height);
      }
    }
  }
}