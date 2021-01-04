#include "mr_parser/mr_parser.h"
#include <iostream>
#include <fstream>
using namespace std;

#define VERT_OFFSET(offset) (startingVertexIndex + (uint16_t)offset + (uint16_t)1)

int main(int argc, const char *argv[]) {
  if (argc < 3)
  {
    cout << "Usage: " << argv[0] << " file.mr output.obj\n";
    return 1;
  }
  Node *root = LoadFromFile(argv[1]);
  vector<Pos3D> vertices = getListOfVerticesForMap(root);
  ofstream out(argv[2], ios::binary);
  for (const Pos3D& pos : vertices) {
    out << "v " << pos.x << " " << -pos.y << " " << pos.z << endl;
  }
  cout << vertices.size() << " vertices\n";
  vector<MapFaceInfo> faces = getListOfFacesForMap(root);
  cout << faces.size() << " faces\n";
  vector<SquareInfo> squares = getListOfSquaresForMap(root);
  cout << squares.size() << " squares\n";
  for (const SquareInfo& square : squares) {
    for (int squareDrawInfoIndex = 0; squareDrawInfoIndex < 1; squareDrawInfoIndex++) {
      const SquareDrawInfo& drawInfo = square.drawInfos[squareDrawInfoIndex];
      int startingFaceIndex = getIndexOfFaceByOffset(faces, drawInfo.faceDataOffset);
      if (startingFaceIndex == -1) {
        throw runtime_error("Starting face not found");
      }
      uint16_t startingVertexIndex = drawInfo.vertexStartIndex;
      for (int i=0; i<drawInfo.numFaces; i++) {
        const MapFaceInfo& face = faces[i + startingFaceIndex];
        if (face.vertexIndicesInMapSquare.size() == 3) {
        out << "f " << VERT_OFFSET(face.vertexIndicesInMapSquare[0]) << " " <<
                       VERT_OFFSET(face.vertexIndicesInMapSquare[1]) << " " <<
                       VERT_OFFSET(face.vertexIndicesInMapSquare[2]) << endl;
        }
        if (face.vertexIndicesInMapSquare.size() == 4) {
          out << "f " << VERT_OFFSET(face.vertexIndicesInMapSquare[1]) << " " <<
                         VERT_OFFSET(face.vertexIndicesInMapSquare[3]) << " " <<
                         VERT_OFFSET(face.vertexIndicesInMapSquare[2]) << " " <<
                         VERT_OFFSET(face.vertexIndicesInMapSquare[0]) << endl;
          // 0 1 2 3
          // x x   x
          //   x x x
          // 0 1 3 2
          // 1 0 2 3
        }
      }
    }
    // break;
  }
}