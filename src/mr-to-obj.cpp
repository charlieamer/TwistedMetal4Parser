#include "mr_parser/mr_parser.h"
#include <iostream>
#include <fstream>
#include <map>
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
  cout << vertices.size() << " vertices\n";
  vector<MapFaceInfo> faces = getListOfFacesForMap(root);
  cout << faces.size() << " faces\n";
  vector<SquareInfo> squares = getListOfSquaresForMap(root);
  cout << squares.size() << " squares\n";

  map<size_t, Color> vertexIndexToColor;

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
      for (size_t j=0; j<face.colors.size(); j++) {
        vertexIndexToColor[VERT_OFFSET(face.vertexIndicesInMapSquare[j])] = face.colors[j];
      }
    }
  }
  for (size_t i=0; i<vertices.size(); i++) {
    out << "v " << vertices[i].x / 1000.0f << " " << -vertices[i].y / 1000.0f << " " << vertices[i].z / 1000.0f;
    auto it = vertexIndexToColor.find(i+1);
    if (it != vertexIndexToColor.end()) {
      out << " " << it->second.red() << " " << it->second.green() << " " << it->second.blue();
    } else {
      out << " 1 1 1";
    }
    out << endl;
  }
  for (const SquareInfo& square : squares) {
    const SquareDrawInfo& drawInfo = square.highLOD;
    int startingFaceIndex = getIndexOfFaceByOffset(faces, drawInfo.faceDataOffset);
    uint16_t startingVertexIndex = drawInfo.vertexStartIndex;
    for (int i=0; i<drawInfo.numFaces; i++) {
      const MapFaceInfo& face = faces[i + startingFaceIndex];
      if (face.vertexIndicesInMapSquare.size() == 3) {
        out << "f " << VERT_OFFSET(face.vertexIndicesInMapSquare[0]) << " " <<
                        VERT_OFFSET(face.vertexIndicesInMapSquare[1]) << " " <<
                        VERT_OFFSET(face.vertexIndicesInMapSquare[2]) << endl;
      }
      if (face.vertexIndicesInMapSquare.size() == 4) {
        // This arrangement is compatible with with most of obj viewers
        out << "f " << VERT_OFFSET(face.vertexIndicesInMapSquare[1]) << " " <<
                        VERT_OFFSET(face.vertexIndicesInMapSquare[3]) << " " <<
                        VERT_OFFSET(face.vertexIndicesInMapSquare[2]) << " " <<
                        VERT_OFFSET(face.vertexIndicesInMapSquare[0]) << endl;
      }
    }
    // break;
  }
}