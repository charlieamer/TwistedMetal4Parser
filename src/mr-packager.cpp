#include <iostream>
#include <filesystem>
#include <fstream>
#include <string.h>
#include "mr_parser/mr_parser.h"

using namespace std;
using namespace std::filesystem;

void recurseFolder(Node& node, path folder) {
  for (directory_iterator i(folder), end; i != end; ++i) {
    if (!is_directory(i->path())) {
      cout << i->path().relative_path().c_str() << endl;
      auto buffer = loadFileToBuffer(i->path().relative_path().c_str());
      Component component(i->path().filename());
      component.data = buffer;
      node.components.push_back(component);
    } else if (i->path().filename().string().substr(0, 5) == "NODE_") {
      Node childNode(i->path().filename().string().substr(5));
      recurseFolder(childNode, i->path());
      node.children.push_back(childNode);
    } else {
      cout << "WARNING: Folder names must start with NODE_. Found: " << i->path().string() << endl;
    }
  }
}

int main(int argc, const char *argv[])
{
  if (argc < 2)
  {
    cout << "Usage: " << argv[0] << " folder.extracted\n";
    return 1;
  }
  string fileOutput = replaceFileExtension(argv[1], "MR");
  Node rootNode("Root");
  recurseFolder(rootNode, path(argv[1]) / "NODE_Root");
  SaveToFile(&rootNode, fileOutput.c_str());
  cout << "Saved to " << fileOutput << endl;
  return 0;
} 