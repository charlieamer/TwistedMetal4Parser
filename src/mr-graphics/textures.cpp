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

vector<vector<bool>> visited;
vector<Rect> islands;

vector<Point> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}; // Up, Down, Left, Right

void dfs(int x, int y, int &minX, int &minY, int &maxX, int &maxY, const vector<vector<RGBA>> &mask) {
  if (x < 0 || x >= mask.size() || y < 0 || y >= mask[0].size() || visited[x][y] || mask[x][y].R == 0)
    return;
  
  visited[x][y] = true;

  // Updating rectangle boundaries
  minX = min(minX, x);
  minY = min(minY, y);
  maxX = max(maxX, x);
  maxY = max(maxY, y);

  for (auto &dir : directions)
    dfs(x + dir.x, y + dir.y, minX, minY, maxX, maxY, mask);
}

vector<Rect> findIslands(const vector<vector<RGBA>> &mask) {
  int n = mask.size();
  int m = mask[0].size();

  visited.resize(n, vector<bool>(m, false));

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      if (!visited[i][j] && mask[i][j].R == 255) {
        int minX = i, minY = j, maxX = i, maxY = j;
        dfs(i, j, minX, minY, maxX, maxY, mask);
        islands.push_back({{minX, minY}, {maxX, maxY}});
      }
    }
  }

  return islands;
}

bool Rect::includes(const Point& point) const {
  return point.x >= topLeft.x && point.x <= bottomRight.x && 
          point.y >= topLeft.y && point.y <= bottomRight.y;
}

vector<vector<RGBA>> cutTexture(const vector<vector<RGBA>>& image, const Rect& rect) {
    vector<vector<RGBA>> cutImage;

    // Check if rectangle coordinates are within image bounds
    if (rect.topLeft.x < 0 || rect.topLeft.y < 0 || 
        rect.bottomRight.x >= image.size() || 
        rect.bottomRight.y >= image[0].size()) {
        throw out_of_range("Rectangle coordinates are out of image bounds.");
    }

    // Iterate over the pixels in the rectangle and copy them to the new image
    for (int i = rect.topLeft.x; i <= rect.bottomRight.x; i++) {
        vector<RGBA> column;
        for (int j = rect.topLeft.y; j <= rect.bottomRight.y; j++) {
            column.push_back(image[i][j]);
        }
        cutImage.push_back(column);
    }

    return cutImage;
}

int getBelongingIslandIndex(const vector<Rect>& rects, const Point& point) {
  for (int i=0; i<rects.size(); i++) {
    if (rects[i].includes(point)) return i;
  }
  throw runtime_error("Island with that position not found");
}

PointF relativeCoordinates(const Rect& rect, const Point& point) {
  if (!rect.includes(point)) {
    throw out_of_range("Point is not inside the rectangle.");
  }

  float width = rect.bottomRight.x - rect.topLeft.x;
  float height = rect.bottomRight.y - rect.topLeft.y;

  PointF relativePoint;
  relativePoint.x = (point.x - rect.topLeft.x) / width;
  relativePoint.y = (point.y - rect.topLeft.y) / height;

  return relativePoint;
}
