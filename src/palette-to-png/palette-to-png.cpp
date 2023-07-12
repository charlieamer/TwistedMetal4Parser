#include <iostream>
#include <mr_parser/utils.h>
#include <colors.h>
#include <textures.h>
#include <sstream>
using namespace std;

int main(int argc, char** argv) {
  if (argc != 2) {
    cout << "Usage: " << argv[0] << " PATH_TO_PALETTE.clt--12";
    return 1;
  }
  
  auto contents = loadFileToBuffer(argv[1]);
  auto texture = genericGetMapTexture(contents);

  stringstream s;
  path inputPath(argv[1]);
  string folderPath = inputPath.parent_path().string();
  string filename = inputPath.filename().string();
  s << folderPath << "/palette__" << filename << "__" << texture.header.offsetX << "__" << texture.header.offsetY << ".png";
  string output = s.str();

  vector<vector<RGBA>> pixels = make2Dvector<RGBA>(texture.header.halfWidth, texture.header.height);

  for (int y=0; y<texture.header.height; y++) {
    for (int x=0; x<texture.header.halfWidth; x++) {
      auto pixel1 = texture.data.data()[x * 2][y];
      auto pixel2 = texture.data.data()[x * 2 + 1][y];
      uint16_t color = (((uint16_t) pixel1) << 8) | ((uint16_t)pixel2);
      pixels[x][y] = getColor(color, TRANSPARENCY_MODE_ALPHA, true);
      if (color == 0) pixels[x][y] = {0, 0, 0, 255};
    }
  }

  saveImage(pixels, output);
}