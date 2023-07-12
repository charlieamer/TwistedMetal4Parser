#include <iostream>
#include <filesystem>
#include <fstream>
#include <string.h>
#include "mr_parser/utils.h"
#include "mr_parser/mr_parser.h"

using namespace std;
using namespace std::filesystem;

vector<string> readOrder(path pth, string filename) {
  ifstream order(pth / filename);
  vector<string> ret;
  if (!order.good()) {
    cout << "WARNING: order file " << filename << " doesn't exist in folder " << pth.string() << endl;
    return ret;
  }
  while(order.good()) {
    string line;
    getline(order, line);
    if (line.length() > 0) {
      ret.push_back(line);
    }
  }
  return ret;
}

#include <filesystem>
#include <iostream>

bool findFileWithSamePrefix(path pathToFile, path& result) {
  // The parent directory of the given path
  path parentDir = pathToFile.parent_path();

  // The name of the file without the extension
  string baseName = pathToFile.stem().string();

  // Iterate over all files in the directory
  for (const auto& entry : directory_iterator(parentDir)) {
    // If the current file starts with the same name
    if (entry.path().stem().string().substr(0, baseName.size()) == baseName) {
      result = entry.path();
      return true;
    }
  }
  return false;
}

int convertStringToInt(const string& str, path forOutput, int defaultValue, int vmin, int vmax) {
    std::istringstream iss(str);
    int number;
    if (iss >> number) {
      if (number < vmin || number > vmax) {
        cout << "WARNING: Number " << str << " is outside of allowed range.\n";
        cout << "  Allowed numbers are (inclusive) " << vmin << " to " << vmax << "\n";
        cout << "  This happened for file name: " << forOutput << endl;
        return defaultValue;
      } else {

      }
      return number;
    } else {
      cout << "WARNING: Unable to convert " << str << " to valid number.\n";
      cout << "  This happened for file name: " << forOutput << endl;
      return defaultValue;
    }
}

// returns true if added
bool appendComponent(path expectedPath, Node& node) {
  // expected path is path that COULD be without any suffixes
  // (like adding component type, etc)
  path pathToFile;
  if (!findFileWithSamePrefix(expectedPath, pathToFile)) {
    return false;
  }
  auto filePath = pathToFile.string();
  auto splits = splitString(pathToFile.filename().string(), "--");
  // cout << filePath << " ";
  // for (auto split : splits) {
  //   cout << split << " ";
  // }
  // cout << endl;
  auto componentName = splits[0];
  if (splits.size() == 1) {
    cout << "WARNING: Couldn't deduce component type for file " << filePath << endl;
    cout << "  File is missing '--TYPE' at end.\n";
    cout << "  If you know the type append it. For example, if type is 3, the file";
    cout << " should be called " << filePath << "--3\n\n";
  }

  if (node.getAttributeByName(componentName) != nullptr) {
    return false;
  }
  // cout << filePath << endl;
  auto buffer = loadFileToBuffer(filePath.c_str());
  if (buffer.size() == 0) {
    cout << "WARNING: " << filePath << " has 0 bytes after loading.\n";
  }
  Component component(componentName);
  component.data = buffer;
  if (splits.size() > 1 && splits[1].length()) {
    component.componentType = convertStringToInt(splits[1], pathToFile, 0xff, 0, 0xff);
    // cout << component.componentType << "\n";
  }
  if (splits.size() > 2 && splits[2].length()) {
    component.typeParameter = convertStringToInt(splits[2], pathToFile, 0xffff, 0, 0xffff);
  }
  node.components.push_back(component);
  return true;
}

void recurseFolder(Node& node, path folder);

// returns true if added
bool appendNode(path pathToFolder, Node& node) {
  auto filename = pathToFolder.filename().string();
  if (!exists(pathToFolder)) {
    return false;
  }
  auto nodename = filename.substr(5);
  if (node.getChildByPath(nodename) != nullptr) {
    return false;
  }
  // cout << "DIR: " << pathToFolder.relative_path().string() << endl;
  Node childNode(nodename);
  recurseFolder(childNode, pathToFolder);
  node.children.push_back(childNode);
  return true;
}

void recurseFolder(Node& node, path folder) {
  vector<string> componentOrder = readOrder(folder, "order_components.txt");
  vector<string> nodeOrder = readOrder(folder, "order_nodes.txt");

  for (const auto& componentInOrder : componentOrder) {
    if (!appendComponent(folder / componentInOrder, node)) {
      cout << "WARNING: Component " << componentInOrder << " mentioned in "
           << folder.string() << "/order_components.txt but the file for it doesn't exist.\n";
    }
  }
  for (const auto& nodeInOrder : nodeOrder) {
    auto fullFolderPath = folder / ("NODE_" + nodeInOrder);
    if (!appendNode(fullFolderPath, node)) {
      cout << "WARNING: Node " << nodeInOrder << " mentioned in "
           << folder.string() << "/order_nodes.txt but the folder for it doesn't exist.\n";
      cout << "Full folder for it should be" << fullFolderPath.string() << endl;
    }
  }

  for (directory_iterator i(folder), end; i != end; ++i) {
    if (!is_directory(i->path())) {
      if (i->path().filename().string() == "order_components.txt" ||
          i->path().filename().string() == "order_nodes.txt") {
        continue;
      }
      if (appendComponent(i->path(), node)) {
        cout << "WARNING: File " << i->path() << " exists but it is not mentioned "
             << "in " << folder.string() << "/order_components.txt. It is added at end of current node.\n";
      };
    } else if (i->path().filename().string().substr(0, 5) == "NODE_") {
      if (appendNode(i->path(), node)) {
        cout << "WARNING: Folder " << i->path() << " exists but it is not mentioned "
             << "in " << folder.string() << "/order_node.txt. It is added at end of current node.\n";
      };
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