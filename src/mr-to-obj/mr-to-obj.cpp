#include "mr_parser/mr_parser.h"
#include "geometry.h"
#include "textures.h"
#include "output.h"
#include <iostream>
#include <fstream>
using namespace std;

void tryConvertMapFile(string mrPath) {
  Node *root = LoadFromFile(mrPath.c_str());
  
  Node *texturesRoot = LoadFromFile(replaceFileExtension(mrPath, "IMG").c_str());
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

  map<uint32_t, string> destroyableFaceIndices = getDestroyableFaceIndices(root->getChildByPath("objects/destroyable/instances/startup"));
  vector<MapFaceWithExtraInfo> facesExtra = buildFacesExtra(squares, faces, vertices, shaders, destroyableFaceIndices);

  string mapFileName = getFileName(mrPath);
  convertTexture(facesExtra, texture, clut, removeFileExtension(mrPath));

  outputMaterial(mrPath);

  ofstream out(replaceFileExtension(mrPath, "obj"), ios::binary);
  out << "mtllib " << mapFileName << ".mtl\n";

  // extract vertices
  vector<VertexWithColorAndUV> allVertices = extractVertices<MapFaceWithExtraInfo, VertexWithColorAndUV>(facesExtra);
  // output vertices
  outputVerticesWithColorAndUVs(out, allVertices);

  out << "usemtl atlas\n";

  outputUVs(out, facesExtra);
  outputFaces(out, facesExtra, allVertices);

  ofstream weaponsOut(replaceFileExtension(mrPath, "txt"));
  outputWeaponList(weaponsOut, root->getChildByPath("objects/powerup/instances/startup"));
}

bool shouldGroupBeTranslated(string groupName) {
  vector<string> translatedStarts {"rr_wheel", "rl_wheel", "fr_wheel", "fl_wheel"};
  for (string start : translatedStarts) {
    if (groupName.rfind(start, 0) == 0) {
      return true;
    }
  }
  return false;
}

map<string, vector<CarFaceWithExtraInfo>> buildCarFacesExtra(
  const vector<Pos3D>& vertices,
  const vector<CarFace>& faces,
  const vector<VertexNormal>& normals,
  Node* root,
  bool extractHighHP
) {
  map<string, vector<CarFaceWithExtraInfo>> ret;
  for (const auto& groupComponent : root->getChildByPath("display/resolution/r0/Groups")->components) {
    CarVertexGroup group = groupComponent.getDataAs<CarVertexGroup>();
    CarVertexGroupInfo groupInfo = extractHighHP ? group.HighHPInfo : group.LowHPInfo;
    vector<CarFaceWithExtraInfo> vec;
    for (int i=0; i<groupInfo.faceCount; i++) {
      CarFaceWithExtraInfo face;
      face.face = faces[groupInfo.startFaceIndex + i];
      face.shader = face.face.shader;
      for (int j=0; j<face.face.getNumVertices(); j++) {
        VertexWithNormalAndUV v;
        v.pos = vertices[face.face.vertexOffsets[j] + groupInfo.startVertexIndex];
        int offsetMul = shouldGroupBeTranslated(groupComponent.name);
        v.pos.x += group.offset3D.x * offsetMul;
        v.pos.y += group.offset3D.y * offsetMul;
        v.pos.z += group.offset3D.z * offsetMul;
        const VertexNormal& normal = normals[face.face.normalOffsets[j] + groupInfo.vertexNormalIndex];
        v.normal.x = normal.x;
        v.normal.y = normal.y;
        v.normal.z = normal.z;
        face.vc.push_back(v);
      }
      vec.push_back(face);
    }
    ret[groupComponent.name] = vec;
  }
  return ret;
}

void tryConvertCarFile(string mrPath, bool extractHighHP) {
  Node *root = LoadFromFile(mrPath.c_str());

  if (!extractHighHP) {
    mrPath = removeFileExtension(mrPath) + "-lowHP.MR";
  }

  vector<Pos3D> vertices = getListOfVerticesForCar(root);
  vector<CarFace> faces = getListOfFacesForCar(root);
  vector<VertexNormal> normals = getCarVertexNormals(root);

  cout << vertices.size() << " vertices\n";
  cout << faces.size() << " faces\n";
  
  ofstream out(replaceFileExtension(mrPath, "obj"), ios::binary);
  auto facesExtra = buildCarFacesExtra(vertices, faces, normals, root, extractHighHP);
  vector<CarFaceWithExtraInfo> allFaces;
  for (const auto& it : facesExtra) {
    allFaces.insert(allFaces.end(), it.second.begin(), it.second.end());
  }
  MapTexture texture = getCarTexture(root);

  string mapFileName = getFileName(mrPath);
  convertTexture(allFaces, texture, texture, removeFileExtension(mrPath));
  out << "mtllib " << mapFileName << ".mtl\n";
  out << "usemtl atlas\n";

  outputMaterial(mrPath);

  // extract vertices
  vector<VertexWithNormalAndUV> allExtractedVertices = extractVertices<CarFaceWithExtraInfo, VertexWithNormalAndUV>(allFaces);
  // output vertices
  outputVerticesWithNormalAndUVs(out, allExtractedVertices);

  outputNormals(out, allFaces);
  outputUVs(out, allFaces);
  int currentFaceIndex = 0;
  for (const auto& it : facesExtra) {
    out << "o " << it.first << endl;
    outputFaces(out, allFaces, allExtractedVertices, currentFaceIndex, it.second.size());
    currentFaceIndex += it.second.size();
  }
}

int main(int argc, const char *argv[]) {
  if (argc < 2)
  {
    cout << "Usage: " << argv[0] << " file.mr\n";
    return 1;
  }

  try {
    cout << "Trying to open as map.\n";
    tryConvertMapFile(argv[1]);
    cout << "Successfuly converted file as map.\n";
    return 0;
  } catch(const exception &ex) {
    cerr << "Error opening the file as map: " << ex.what() << endl;
    try {
      cout << "Trying to open as car.\n";
      cout << "High HP\n";
      tryConvertCarFile(argv[1], true);
      cout << "Low HP\n";
      tryConvertCarFile(argv[1], false);
      cout << "Successfuly converted file as car.\n";
    } catch(const exception& ex) {
      cerr << "Error opening file as car: " << ex.what() << endl;
    } 
    return 0;
  }
  return 1;
}