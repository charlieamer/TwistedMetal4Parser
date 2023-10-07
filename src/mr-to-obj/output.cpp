#include "output.h"
#include <fstream>
#include <algorithm>

void outputVerticesWithColorAndUVs(ostream& out, const vector<VertexWithColorAndUV>& vertices) {
  for (const auto& vertex : vertices) {
    out << "v " << -vertex.pos.x / 1000.0f << " " << -vertex.pos.y / 1000.0f << " " << vertex.pos.z / 1000.0f;
    if (vertex.hasColor) {
      out << " " << vertex.col.red() << " " << vertex.col.green() << " " << vertex.col.blue();
    } else {
      out << " 0 0 0";
    }
    out << endl;
  }
}

void outputVerticesWithNormalAndUVs(ostream& out, const vector<VertexWithNormalAndUV>& vertices) {
  for (const auto& vertex : vertices) {
    out << "v " << -vertex.pos.x / 10.0f << " " << -vertex.pos.y / 10.0f << " " << vertex.pos.z / 10.0f;
    out << endl;
  }
}

void outputMaterial(path outputFolder, string mapName, int numIslands) {
  ofstream outMtl(outputFolder / (mapName + ".mtl"), ios::binary);
  for (int i=0; i<numIslands; i++) {
    outMtl  << "newmtl island-" << i << "\n"
            << "   Ka 1.000 1.000 1.000\n"
            << "   Kd 1.000 1.000 1.000\n"
            << "   Ks 0.000 0.000 0.000\n"
            << "   map_Kd island-" << i << ".png\n"
            << "\n";
  }
}

void outputNormals(ostream& out, const vector<CarFaceWithExtraInfo> facesExtra) {
  for (const auto& face : facesExtra) {
    UvRect uv(face.shader, face.vc.size());
    if (face.vc.size() == 3) {
      out << "vn " << -face.vc[2].normal.x << " " << -face.vc[2].normal.y << " " << face.vc[2].normal.z << endl;
      out << "vn " << -face.vc[1].normal.x << " " << -face.vc[1].normal.y << " " << face.vc[1].normal.z << endl;
      out << "vn " << -face.vc[0].normal.x << " " << -face.vc[0].normal.y << " " << face.vc[0].normal.z << endl;
    }
    if (face.vc.size() == 4) {
      out << "vn " << -face.vc[0].normal.x << " " << -face.vc[0].normal.y << " " << face.vc[0].normal.z << endl;
      out << "vn " << -face.vc[2].normal.x << " " << -face.vc[2].normal.y << " " << face.vc[2].normal.z << endl;
      out << "vn " << -face.vc[3].normal.x << " " << -face.vc[3].normal.y << " " << face.vc[3].normal.z << endl;
      out << "vn " << -face.vc[1].normal.x << " " << -face.vc[1].normal.y << " " << face.vc[1].normal.z << endl;
    }
  }
}

void outputFaces(ostream& out, const vector<MapFaceWithExtraInfo> facesExtra, const vector<VertexWithColorAndUV> allVertices) {
  int uvCount = 0;
  string lastDestroyableName = "NO GROUP BY DEFAULT";
  size_t lastUsedIsland = -1;
  for (const auto& face : facesExtra) {
    if (face.belongingIslandIndex != lastUsedIsland) {
      out << "usemtl island-" << face.belongingIslandIndex << endl;
      lastUsedIsland = face.belongingIslandIndex;
    }
    UvRect uv(face.shader, face.vc.size());
    if (face.belongingDestroyableName != lastDestroyableName) {
      string groupForOutput = (face.belongingDestroyableName.length() == 0) ? "default" : face.belongingDestroyableName;
      out << "g " << groupForOutput << endl;
    }
    lastDestroyableName = face.belongingDestroyableName;

    if (face.face.vertexIndicesInMapSquare.size() == 3) {
      out << "f " << (find(allVertices.begin(), allVertices.end(), face.vc[2]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 1 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[1]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 2 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[0]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 3 << "\n";
      uvCount += 3;
    }
    if (face.face.vertexIndicesInMapSquare.size() == 4) {
      out << "f " << (find(allVertices.begin(), allVertices.end(), face.vc[0]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 1 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[2]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 2 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[3]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 3 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[1]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 4 << "\n";
      uvCount += 4;
    }
  }
}

void outputFaces(ostream& out, const vector<CarFaceWithExtraInfo> facesExtra, const vector<VertexWithNormalAndUV> allVertices, int faceOffset, int numFaces) {
  int uvCount = 0;
  for (int i=0; i<faceOffset; i++) {
    uvCount += facesExtra[i].vc.size();
  }
  size_t lastUsedIsland = -1;
  for (int i=faceOffset; i < faceOffset + numFaces; i++) {
    const auto& face = facesExtra[i];
    if (face.belongingIslandIndex != lastUsedIsland) {
      out << "usemtl island-" << face.belongingIslandIndex << endl;
      lastUsedIsland = face.belongingIslandIndex;
    }
    UvRect uv(face.shader, face.vc.size());
    if (face.vc.size() == 3) {
      out << "f " << (find(allVertices.begin(), allVertices.end(), face.vc[2]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 1 << "/" << uvCount + 1 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[1]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 2 << "/" << uvCount + 2 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[0]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 3 << "/" << uvCount + 3 << "\n";
      uvCount += 3;
    }
    if (face.vc.size() == 4) {
      out << "f " << (find(allVertices.begin(), allVertices.end(), face.vc[0]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 1 << "/" << uvCount + 1 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[2]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 2 << "/" << uvCount + 2 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[3]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 3 << "/" << uvCount + 3 << " "
                  << (find(allVertices.begin(), allVertices.end(), face.vc[1]) - allVertices.begin()) + 1 << "/"
                    << uvCount + 4 << "/" << uvCount + 4 << "\n";
      uvCount += 4;
    }
  }
}

void outputWeaponList(ofstream& out, Node* weaponRoot) {
  for (Node& child : weaponRoot->children) {
    const auto pos = child.getAttributeByName("pos")->getDataAsVector<int16_t>();
    out << child.name << " " <<
           child.getAttributeByName("type")->getDataAsString() << " " <<
           -pos[0]/1000.0 << " " << -pos[1]/1000.0 << " " << pos[2]/1000.0 << endl;
  }
}