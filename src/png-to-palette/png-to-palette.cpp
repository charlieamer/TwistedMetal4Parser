#include <iostream>
#include <mr_parser/utils.h>
#include <colors.h>
#include <textures.h>
#include <lodepng.h>
#include <fstream>
using namespace std;

int main(int argc, char** argv) {
  if (argc != 2) {
    cout << "Usage: " << argv[0] << " PATH_TO_PALETTE.png";
    return 1;
  }

  MapTexture texture;
  unsigned w, h;
  vector<byte_t> pngBytes;
  path inputPath = argv[1];
  auto splits = splitString(inputPath.filename().stem().string(), "__");
  if (splits.size() != 4) {
    for (auto s : splits) {
      cout << s << " " << endl;
    }
    cout << "ERROR: File is not named correctly. Please name it like this: palette--original_filename--offsetX--offsetY.png\n";
    return 1;
  }
  path output = path(argv[1]).parent_path() / splits[1];
  auto image = lodepng::decode(pngBytes, w, h, string(argv[1]));
  int bytesPerRow = w * 4;
  texture.header.halfWidth = w;
  texture.header.height = h;
  texture.header.offsetX = convertStringToInt(splits[2], argv[1], 0, 0, 65535);
  texture.header.offsetY = convertStringToInt(splits[3], argv[1], 0, 0, 65535);

  texture.data = make2Dvector<byte_t>(w * 2, h);

  for (int y=0; y<h; y++) {
    for (int x=0; x<w; x++) {
      RGBA color = {
        pngBytes[y * bytesPerRow + x * 4 + 0],
        pngBytes[y * bytesPerRow + x * 4 + 1],
        pngBytes[y * bytesPerRow + x * 4 + 2],
        pngBytes[y * bytesPerRow + x * 4 + 3],
      };
      uint16_t r = color.R;
      uint16_t g = color.G;
      uint16_t b = color.B;
      uint16_t a = color.A;
      uint16_t mask = 0b00000000'00011111;
      uint16_t convertedColor =
        (((r >> 3) & mask) << 0) |
        (((g >> 3) & mask) << 5) |
        (((b >> 3) & mask) << 10) |
        ((a < 200) << 15);
      texture.data[x * 2 + 1][y] = (convertedColor >> 8) & 0xff;
      texture.data[x * 2][y] = convertedColor & 0xff;
      if (x == 0 && y == 0) {
        cout << r << " " << g << " " << b << " " << a << " " << convertedColor << endl;
      }
    }
  }

  ofstream out(output, ios::binary);
  writeBytesToFile(out, texture.convertToBytes());
  cout << "Saved to " << output << endl;
}