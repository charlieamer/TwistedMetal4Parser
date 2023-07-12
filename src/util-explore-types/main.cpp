#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <mr_parser/mr_parser.h>
#include <map>
#include <set>
using namespace std;

bool validate_arguments(int argc, char* argv[], filesystem::path& path) {
  if(argc < 2) {
    cout << "Usage: " << argv[0] << " [path]\n";
    return false;
  }
    
  path = argv[1];

  if (!filesystem::exists(path)) {
    cout << "Path does not exist: " << path << '\n';
    return false;
  }

  if (!filesystem::exists(path / "MENU")) {
    cout << "Path does not exist: " << path / "MENU" << '\n';
    return false;
  }

  if (!filesystem::is_directory(path)) {
    cout << "Path is not a directory: " << path << '\n';
    return false;
  }
    
  return true;
}

vector<string> get_file_list(const filesystem::path& path) {
  vector<string> file_list;
  for (const auto& entry : filesystem::directory_iterator(path)) {
    file_list.push_back(filesystem::absolute(entry.path()).string());
  }
  return file_list;
}

void print_paths(const vector<string>& file_list) {
  for (const auto& path : file_list) {
    cout << path << '\n';
  }
}

map<uint8_t, bool> typeHasParam;
map<uint8_t, set<string>> typeComponents;
map<string, uint8_t> nameToType;

void checkComponentType(const Component& component) {
  bool hasParam = component.typeParameter != 0xffff;
  if (typeHasParam.count(component.componentType)) {
    if (typeHasParam[component.componentType] != hasParam) {
      cout << "WARNING: mixed hasParam for type " << (int)component.componentType << " component: " << component.name << endl;
    }
  } else {
    typeHasParam[component.componentType] = hasParam;
  }
}

void fillTypeInfo(const Component& component, const Node* node) {
  typeComponents[component.componentType].insert(component.name);
  if (nameToType.count(component.name)) {
    if (nameToType[component.name] != component.componentType) {
      cout << "WARNING: mixed component type for " << component.name
           << " type: " << (int)component.componentType
           << " vs " << (int)nameToType[component.name]
           << " (node " << node->name << ")" << endl;
    }
  } else {
    nameToType[component.name] = component.componentType;
  }
}

void do_node(const Node* node) {
  for (const auto& component : node->components) {
    checkComponentType(component);
    fillTypeInfo(component, node);
    if (component.name == "maxDistanceToDisplay") {
      cout << "HERE\n";
    }
  }
  for (const auto& child : node->children) {
    do_node(&child);
  }
}

int main(int argc, char* argv[]) {
  filesystem::path path;
  
  if (!validate_arguments(argc, argv, path)) {
    return 1;
  }

  vector<string> all_files = get_file_list(path);
  vector<string> menu_files = get_file_list(path / "MENU");
  all_files.insert(all_files.begin(), menu_files.begin(), menu_files.end());

  vector<Node*> all_nodes;
  map<Node*, string> fnames;
  for (auto fname : all_files) {
    try {
      auto node = LoadFromFile(fname.c_str(), false);
      all_nodes.push_back(node);
      fnames[node] = fname;
    } catch(runtime_error err) {
    }
  }

  cout << "Loaded " << all_nodes.size() << " files\n";

  cout << hex;
  for (auto node : all_nodes) {
    cout << fnames[node] << endl;
    do_node(node);
  }

  cout << endl << endl << hex;

  // for (auto& it : typeComponents) {
  //   cout << "0x" << (int)it.first << ": ";
  //   for (auto& name : it.second) {
  //     cout << name << "   ";
  //   }
  //   cout << endl << endl;
  // }

  // cout << "map<string, uint8_t> nameBasedType = {\n";
  // for (auto& it : nameToType) {
  //   cout << "  {\"" << it.first << "\", 0x" << (int)it.second << "},\n";
  // }
  // cout << "}\n\n";

  cout << "map<uint8_t, bool> typeHasParams = {\n";
  for (auto& it : typeHasParam) {
    cout << "  {0x" << (int)it.first << ", " << (it.second ? "true" : "false") << "},\n";
  }
  cout << "}\n\n";

  return 0;
}
