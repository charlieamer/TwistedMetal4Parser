#pragma once
#include <iostream>
#include "geometry.h"
using namespace std;

void outputVerticesWithColorAndUVs(ostream& out, const vector<VertexWithColorAndUV>& vertices);

void outputVerticesWithNormalAndUVs(ostream& out, const vector<VertexWithNormalAndUV>& vertices);

void outputMaterial(string mrFilePath);

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