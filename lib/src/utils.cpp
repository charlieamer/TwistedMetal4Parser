#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "mr_parser/utils.h"
using namespace std::filesystem;

string replaceFileExtension(string fileName, string newExtension) {
  auto lastDot = fileName.find_last_of('.');
  if (lastDot != string::npos) {
    fileName = fileName.substr(0, lastDot);
  }
  return fileName + "." + newExtension;
}

string removeFileExtension(string filename) {
  const size_t period_idx = filename.rfind('.');
  if (std::string::npos != period_idx)
  {
      filename.erase(period_idx);
  }
  return filename;
}

string getFileName(string filename) {
  // Remove directory if present.
  // Do this before extension removal incase directory has a period character.
  const size_t last_slash_idx = filename.find_last_of("\\/");
  if (std::string::npos != last_slash_idx)
  {
      filename.erase(0, last_slash_idx + 1);
  }

  // Remove extension if present.
  const size_t period_idx = filename.rfind('.');
  if (std::string::npos != period_idx)
  {
      filename.erase(period_idx);
  }
  return filename;
}

vector<string> splitString(string target, string delim)
{
  vector<string> v;
  if (!target.empty())
  {
    string::size_type start = 0;
    do
    {
      size_t x = target.find(delim, start);
      if (x == string::npos)
        break;

      string strToAdd = target.substr(start, x - start);
      v.push_back(strToAdd);
      start += delim.size() + strToAdd.length();
    } while (true);

    v.push_back(target.substr(start));
  }
  return v;
}

vector<byte_t> loadFileToBuffer(const char* path) {
  ifstream infile(path, ios::binary);

  if (infile.bad()) {
    throw runtime_error("Unable to open file");
  }

  infile.seekg(0, std::ios::end);
  size_t length = (size_t)infile.tellg();
  infile.seekg(0, std::ios::beg);

  vector<byte_t> buffer;
  buffer.insert(buffer.begin(), istreambuf_iterator<char>(infile), istreambuf_iterator<char>());
  return buffer;
}

int convertStringToInt(const string& str, path forOutput, int defaultValue, int vmin, int vmax) {
  std::istringstream iss(str);
  int number;
  if (iss >> number) {
    if (number < vmin || number > vmax) {
      cout << "WARNING: Number " << str << " is outside of allowed range.\n";
      cout << "  Allowed numbers are (inclusive) " << vmin << " to " << vmax << "\n";
      cout << "  This happened for file name: " << forOutput << endl;
      return defaultValue;
    } else {

    }
    return number;
  } else {
    cout << "WARNING: Unable to convert " << str << " to valid number.\n";
    cout << "  This happened for file name: " << forOutput << endl;
    return defaultValue;
  }
}

void writeBytesToFile(ostream& out, vector<byte_t> bytes) {
  for (const char c : bytes)
  {
    out << c;
  }
}

vector<Pos3D> getListOfVertices(Node* root, string path, string attribute) {
  Node* displayPolys = root->getChildByPath(path);
  if (displayPolys == nullptr) {
    throw runtime_error("Node path not found.");
  }
  Component* vertexData = displayPolys->getAttributeByName(attribute);
  if (vertexData == nullptr) {
    throw runtime_error("Node path doesn't contain given attribute component.");
  }
  vector<Pos3D> ret;
  ret.resize(vertexData->data.size() / sizeof(Pos3D));
  memcpy(ret.data(), vertexData->data.data(), vertexData->data.size());
  return ret;
}

vector<Pos3D> getListOfVerticesForMap(Node* root) {
  return getListOfVertices(root, "world/displayPolys", "vertexData");
}

vector<Pos3D> getListOfVerticesForCar(Node* root) {
  return getListOfVertices(root, "display/resolution", "VertexList");
}

#define TEXTURED_TRIANGLE__OPAQUE 0x24
#define TEXTURED_TRIANGLE__SEMI_TRANSPARENT 0x26
#define TEXTURED_QUAD__OPAQUE 0x2c
#define TEXTURED_QUAD__SEMI_TRANSPARENT 0x2e
#define SHADED_TEXTURED_TRIANGLE__OPAQUE 0x34
#define SHADED_TEXTURED_TRIANGLE__SEMI_TRANSPARENT 0x36
#define SHADED_TEXTURED_QUAD__OPAQUE 0x3c
#define SHADED_TEXTURED_QUAD__SEMI_TRANSPARENT 0x3e

int numPolyVertices(byte_t specialByte) {
    return (specialByte == TEXTURED_TRIANGLE__OPAQUE ||
            specialByte == SHADED_TEXTURED_TRIANGLE__OPAQUE ||
            specialByte == SHADED_TEXTURED_TRIANGLE__SEMI_TRANSPARENT ||
            specialByte == TEXTURED_TRIANGLE__SEMI_TRANSPARENT) ? 3 : 4;
}

bool isPolyShaded(byte_t specialByte) {
  return (specialByte == SHADED_TEXTURED_TRIANGLE__OPAQUE ||
          specialByte == SHADED_TEXTURED_QUAD__OPAQUE ||
          specialByte == SHADED_TEXTURED_QUAD__SEMI_TRANSPARENT ||
          specialByte == SHADED_TEXTURED_TRIANGLE__SEMI_TRANSPARENT);
}

bool isPolyTransparent(byte_t specialByte) {
    return (specialByte == SHADED_TEXTURED_QUAD__SEMI_TRANSPARENT ||
            specialByte == TEXTURED_QUAD__SEMI_TRANSPARENT ||
            specialByte == SHADED_TEXTURED_TRIANGLE__SEMI_TRANSPARENT ||
            specialByte == TEXTURED_TRIANGLE__SEMI_TRANSPARENT);
}

bool MapFaceInfo::isShaded() const {
  return isPolyShaded(specialByte);
}

bool MapFaceInfo::isTransparent() const {
  return isPolyTransparent(specialByte);
}

bool CarFace::isTransparent() const {
  return isPolyTransparent(faceSpecialByte);
}

vector<MapFaceInfo> getListOfFaces(Node* root, string path, string attribute) {
  vector<MapFaceInfo> ret;
  Node* displayPolys = root->getChildByPath(path);
  if (displayPolys == nullptr) {
    throw runtime_error("Path to node not found.");
  }
  Component* faceData = displayPolys->getAttributeByName(attribute);
  if (faceData == nullptr) {
    throw runtime_error("Path to node doesn't contain face data component.");
  }
  for (byte_t* dataPtr = faceData->data.data(); dataPtr < faceData->data.data() + faceData->data.size();) {
    MapFaceInfo faceInfo;
    byte_t specialByte = dataPtr[3];
    if (specialByte != TEXTURED_TRIANGLE__OPAQUE &&
        specialByte != TEXTURED_QUAD__OPAQUE &&
        specialByte != TEXTURED_QUAD__SEMI_TRANSPARENT &&
        specialByte != TEXTURED_TRIANGLE__SEMI_TRANSPARENT &&
        specialByte != SHADED_TEXTURED_TRIANGLE__OPAQUE &&
        specialByte != SHADED_TEXTURED_QUAD__OPAQUE &&
        specialByte != SHADED_TEXTURED_QUAD__SEMI_TRANSPARENT &&
        specialByte != SHADED_TEXTURED_TRIANGLE__SEMI_TRANSPARENT
    ) {
      throw runtime_error("Unknown special byte in face");
    }

    byte_t numVertices = numPolyVertices(specialByte);
    faceInfo.specialByte = specialByte;

    byte_t sizeInBuffer = faceInfo.isShaded() ? 24 : 16;
    const static byte_t colorOffsets[] = {0, 8, 12, 16};
    for (byte_t i=0; i<numVertices; i++) {
      faceInfo.vertexIndicesInMapSquare.push_back(dataPtr[4 + i]);
      if (faceInfo.isShaded()) {
        Color color;
        memcpy(&color, dataPtr + colorOffsets[i], sizeof(Color));
        faceInfo.colors.push_back(color);
      }
    }
    faceInfo.shaderOffset = GetIntFromBuffer<uint32_t>(dataPtr, sizeInBuffer - 4);
    faceInfo.offsetInBuffer = dataPtr - faceData->data.data();
    dataPtr += sizeInBuffer;
    ret.push_back(faceInfo);
  }
  return ret;
}

vector<MapFaceInfo> getListOfFacesForMap(Node* root) {
  return getListOfFaces(root, "world/displayPolys", "faceData");
}

vector<CarFace> getListOfFacesForCar(Node* root) {
  Node* displayPolys = root->getChildByPath("display/resolution");
  if (displayPolys == nullptr) {
    throw runtime_error("Node path not found.");
  }
  Component* faceData = displayPolys->getAttributeByName("FaceList");
  if (faceData == nullptr) {
    throw runtime_error("Node path doesn't contain given attribute component.");
  }
  vector<CarFace> ret;
  ret.resize(faceData->data.size() / sizeof(CarFace));
  memcpy(ret.data(), faceData->data.data(), faceData->data.size());
  return ret;
}

int CarFace::getNumVertices() const {
  return numPolyVertices(faceSpecialByte);
}

int getIndexOfFaceByOffset(const vector<MapFaceInfo>& faces, uint32_t desiredOffsetInBuffer) {
  int ret = 0;
  for (const MapFaceInfo& face : faces) {
    if (face.offsetInBuffer == desiredOffsetInBuffer) {
      return ret;
    }
    ret++;
  }
  return -1;
}

uint16_t ShaderInfo::getClutX() const {
  return (pallete & 0b00000000'00111111) * 16;
}


uint16_t ShaderInfo::getClutY() const {
  return (pallete & 0b01111111'11000000) >> 6;
}

CLUT_MODE ShaderInfo::getClutMode() const {
  return (CLUT_MODE)((texpage & 0b00000001'10000000) >> 7);
}

SEMI_TRANSPARENCY_MODE ShaderInfo::getTransparencyMode() const {
  return (SEMI_TRANSPARENCY_MODE)((texpage & 0b00000000'01100000) >> 5);
}

uint16_t ShaderInfo::getTexturePageX() const {
  return (texpage & 0b00000000'00001111) * 64;
}

uint16_t ShaderInfo::getTexturePageY() const {
  return ((texpage & 0b00000000'00010000) >> 4) * 256;
}

vector<SquareInfo> getListOfSquaresForMap(Node* root) {
  Node* squares = root->getChildByPath("world/squares");
  if (squares == nullptr) {
    throw runtime_error("world/squares not found. Maybe this is not a map file?");
  }
  Component* squareData = squares->getAttributeByName("squareData");
  if (squareData == nullptr) {
    throw runtime_error("world/squares doesn't contain squareData component.");
  }
  vector<SquareInfo> ret;
  ret.resize(squareData->data.size() / sizeof(SquareInfo));
  memcpy(ret.data(), squareData->data.data(), squareData->data.size());
  return ret;
}

vector<ShaderInfo> getListOfShaderInfoForMap(Node* root) {
  Node* displayPolys = root->getChildByPath("world/displayPolys");
  if (displayPolys == nullptr) {
    throw runtime_error("world/displayPolys not found. Maybe this is not a map file?");
  }
  Component* shaderInfo = displayPolys->getAttributeByName("shaderData");
  if (shaderInfo == nullptr) {
    throw runtime_error("world/displayPolys doesn't contain shaderData component.");
  }
  vector<ShaderInfo> ret;
  ret.resize(shaderInfo->data.size() / sizeof(ShaderInfo));
  memcpy(ret.data(), shaderInfo->data.data(), shaderInfo->data.size());
  return ret;
}


vector<byte_t> MapTexture::convertToBytes() {
  vector<byte_t> ret;
  ret.push_back(header.offsetX & 0xff);
  ret.push_back((header.offsetX >> 8) & 0xff);
  ret.push_back(header.offsetY & 0xff);
  ret.push_back((header.offsetY >> 8) & 0xff);
  ret.push_back(header.halfWidth & 0xff);
  ret.push_back((header.halfWidth >> 8) & 0xff);
  ret.push_back(header.height & 0xff);
  ret.push_back((header.height >> 8) & 0xff);
  for (int y=0; y<header.height; y++) {
    for (int x=0; x<header.halfWidth * 2; x++) {
      ret.push_back(data[x][y]);
    }
  }
  return ret;
}

MapTexture genericGetMapTexture(const vector<byte_t>& bytes) {
  MapTexture ret;
  memcpy(&ret.header, bytes.data(), sizeof(MapTextureHeader));
  if (bytes.size() - sizeof(MapTextureHeader) != ret.header.halfWidth * 2 * ret.header.height ) {
    throw runtime_error("invalide texture size");
  }
  ret.data = make2Dvector<byte_t>(ret.header.halfWidth * 2, ret.header.height);
  size_t index = 0;
  for (uint16_t y = 0; y < ret.header.height; y++) {
    for (uint16_t x = 0; x < ret.header.halfWidth * 2; x++) {
      ret.data[x][y] = bytes[sizeof(MapTextureHeader) + index++];
    }
  }
  return ret;
}

MapTexture genericGetMapTexture(Node* textureRoot, string childPath, string attributeName) {
  Node* world = textureRoot->getChildByPath(childPath);
  if (world == nullptr) {
    throw runtime_error("node in texture not found.");
  }
  Component* texture = world->getAttributeByName(attributeName);
  if (texture == nullptr) {
    throw runtime_error("node in texture doesn't contain texture data");
  }
  return genericGetMapTexture(texture->data);
}

MapTexture getMapTexture(Node* textureRoot) {
  return genericGetMapTexture(textureRoot, "world", "world.bit");
}

MapTexture getMapClut(Node* textureRoot) {
  return genericGetMapTexture(textureRoot, "world", "world.clt");
}

MapTexture getCarTexture(Node* root) {
  return genericGetMapTexture(root, "display", "car.bit");
}

vector<VertexNormal> getCarVertexNormals(Node* root) {
  return root->getChildByPath("display/resolution")->getAttributeByName("VertexNormalList")->getDataAsVector<VertexNormal>();
}