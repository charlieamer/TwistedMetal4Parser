#include "textures.h"
#include <lodepng.h>
#include <string>
#include <iostream>
using namespace std;

string textureNameFromShader(const MapFaceWithExtraInfo& face) {
  UvRect uv(face.shader, face.vc.size());
  char tmp[256];
  snprintf(tmp, 256, "%dx%d_%dx%d_%x_%x",
    uv.minu, uv.minv, uv.maxu, uv.maxv, face.shader.pallete, face.shader.texpage
  );
  return tmp;
}

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
  cout << "Saving image " << outputFilename << endl;
  lodepng::encode(outputFilename, (unsigned char*)(void*)outputArray.data(), width, height);
}

void outputTextureError(string text) {
  cerr << text << endl;
}
