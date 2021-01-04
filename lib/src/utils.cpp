#include <stdexcept>
#include <iostream>
#include "mr_parser/utils.h"

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

vector<Pos3D> getListOfVerticesForMap(Node* root) {
  Node* displayPolys = root->getChildByPath("world/displayPolys");
  if (displayPolys == nullptr) {
    throw runtime_error("world/displayPolys not found. Maybe this is not a map file?");
  }
  Component* vertexData = displayPolys->getAttributeByName("vertexData");
  if (vertexData == nullptr) {
    throw runtime_error("world/displayPolys doesn't contain vertexData component.");
  }
  vector<Pos3D> ret;
  ret.resize(vertexData->data.size() / sizeof(Pos3D));
  memcpy(ret.data(), vertexData->data.data(), vertexData->data.size());
  return ret;
}


#define TEXTURED_TRIANGLE__OPAQUE 0x24
#define TEXTURED_QUAD__OPAQUE 0x2c
#define TEXTURED_QUAD__SEMI_TRANSPARENT 0x2e
#define SHADED_TEXTURED_TRIANGLE__OPAQUE 0x34
#define SHADED_TEXTURED_QUAD__OPAQUE 0x3c
#define SHADED_TEXTURED_QUAD__SEMI_TRANSPARENT 0x3e

vector<MapFaceInfo> getListOfFacesForMap(Node* root) {
  vector<MapFaceInfo> ret;
  Node* displayPolys = root->getChildByPath("world/displayPolys");
  if (displayPolys == nullptr) {
    throw runtime_error("world/displayPolys not found. Maybe this is not a map file?");
  }
  Component* faceData = displayPolys->getAttributeByName("faceData");
  if (faceData == nullptr) {
    throw runtime_error("world/displayPolys doesn't contain faceData component.");
  }
  for (byte_t* dataPtr = faceData->data.data(); dataPtr < faceData->data.data() + faceData->data.size();) {
    MapFaceInfo faceInfo;
    byte_t specialByte = dataPtr[3];
    if (specialByte != TEXTURED_TRIANGLE__OPAQUE &&
        specialByte != TEXTURED_QUAD__OPAQUE &&
        specialByte != TEXTURED_QUAD__SEMI_TRANSPARENT &&
        specialByte != SHADED_TEXTURED_TRIANGLE__OPAQUE &&
        specialByte != SHADED_TEXTURED_QUAD__OPAQUE &&
        specialByte != SHADED_TEXTURED_QUAD__SEMI_TRANSPARENT
    ) {
      throw runtime_error("Unknown special byte in face");
    }

    byte_t numVertices = (specialByte == TEXTURED_TRIANGLE__OPAQUE ||
                          specialByte == SHADED_TEXTURED_TRIANGLE__OPAQUE) ? 3 : 4;
    faceInfo.isShaded = (specialByte == SHADED_TEXTURED_TRIANGLE__OPAQUE ||
                         specialByte == SHADED_TEXTURED_QUAD__OPAQUE ||
                         specialByte == SHADED_TEXTURED_QUAD__SEMI_TRANSPARENT);
    faceInfo.isTransparent = (specialByte == SHADED_TEXTURED_QUAD__SEMI_TRANSPARENT ||
                              specialByte == TEXTURED_QUAD__SEMI_TRANSPARENT);

    byte_t specialByteForSize = specialByte & 0xfc;
    byte_t sizeInBuffer = 0xff;
    if (specialByteForSize == 0x34 || specialByteForSize == 0x3c) {
      sizeInBuffer = 24;
    } else if (specialByteForSize == 0x24 || specialByteForSize == 0x2c) {
      sizeInBuffer = 16;
    }
    if (sizeInBuffer == 0xff) {
      throw runtime_error("Couldn't determine size of face");
    }
    for (byte_t i=0; i<numVertices; i++) {
      faceInfo.vertexIndicesInMapSquare.push_back(dataPtr[4 + i]);
    }
    faceInfo.shaderOffset = GetIntFromBuffer<uint32_t>(dataPtr, sizeInBuffer - 4);
    faceInfo.offsetInBuffer = dataPtr - faceData->data.data();
    dataPtr += sizeInBuffer;
    ret.push_back(faceInfo);
  }
  return ret;
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