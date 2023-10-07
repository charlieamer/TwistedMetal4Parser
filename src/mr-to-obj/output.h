#pragma once
#include <iostream>
#include "geometry.h"
#include "textures.h"
using namespace std;

void outputVerticesWithColorAndUVs(ostream& out, const vector<VertexWithColorAndUV>& vertices);

void outputVerticesWithNormalAndUVs(ostream& out, const vector<VertexWithNormalAndUV>& vertices);

void outputMaterial(path outputFolder, string mapName, int numIslands);

template<typename T>
void outputUVs(ostream& out, const vector<T> facesExtra) {
  for (const auto& face : facesExtra) {
    UvRect uv(face.shader, face.vc.size());
    if (face.vc.size() == 3) {
      out << "vt " << face.vc[2].u << " " << 1.0 - face.vc[2].v << endl;
      out << "vt " << face.vc[1].u << " " << 1.0 - face.vc[1].v << endl;
      out << "vt " << face.vc[0].u << " " << 1.0 - face.vc[0].v << endl;
    }
    if (face.vc.size() == 4) {
      out << "vt " << face.vc[0].u << " " << 1.0 - face.vc[0].v << endl;
      out << "vt " << face.vc[2].u << " " << 1.0 - face.vc[2].v << endl;
      out << "vt " << face.vc[3].u << " " << 1.0 - face.vc[3].v << endl;
      out << "vt " << face.vc[1].u << " " << 1.0 - face.vc[1].v << endl;
    }
  }
}

void outputNormals(ostream& out, const vector<CarFaceWithExtraInfo> facesExtra);

void outputFaces(ostream& out, const vector<MapFaceWithExtraInfo> facesExtra, const vector<VertexWithColorAndUV> allVertices);

void outputFaces(ostream& out, const vector<CarFaceWithExtraInfo> facesExtra, const vector<VertexWithNormalAndUV> allVertices, int faceOffset, int numFaces);

void outputWeaponList(ofstream& out, Node* weaponRoot);

// returns array of islands in atlas
template<typename T>
vector<Rect> convertTexture(
  vector<T>& facesExtra, const MapTexture& texture,
  const MapTexture& clut, path outputFolder, string mapName
) {
  int width = texture.header.halfWidth * 4;
  int height = texture.header.height * 2;
  vector<vector<RGBA>> atlas(width);
  vector<vector<RGBA>> mask(width);
  for (auto& column : atlas) {
    column.resize(height);
  }
  for (auto& column : mask) {
    column.resize(height);
  }
  for (auto& face : facesExtra) {
    fillTextureFace(face, texture, clut, atlas, mask);
  }
  auto islands = findIslands(mask);

  for (int i=0; i<islands.size(); i++) {
    auto theCut = cutTexture(atlas, islands[i]);
    stringstream s;
    s << outputFolder.string() << "/" << "island-" << i << ".png";
    saveImage(theCut, s.str());
  }
  for (auto& face : facesExtra) {
    setFaceUVs(face, texture.header, width, height, islands);
  }
  saveImage(atlas, (outputFolder / (mapName + ".png")).string());
  // saveImage(mask, mapFileName + "-mask.png");
  return islands;
}