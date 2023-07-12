#include <iostream>
#include <map>
#include <algorithm>
#include <filesystem>
#include <assert.h>
#include "mr_parser/mr_parser.h"

using namespace filesystem;

template<typename T>
bool compareNamable(const T& a, const T& b) {
  return a.name < b.name;
}

template<typename T>
bool compareNamableVector(vector<T>& a, vector<T>& b, const char* comparingThing) {
  if (a.size() != b.size()) {
    cout << "Different count of " << comparingThing << ". "
         << a.size() << " vs " << b.size() << endl;
    return false;
  }
  for (int i=0; i<a.size(); i++) {
    if (a[i].name != b[i].name) {
      cout << comparingThing << "[" << i << "] have different names: "
        << a[i].name << " vs " << b[i].name << endl;
      return false;
    }
  }
  return true;
}

void printDataHex(const vector<byte_t>& data) {
  for (int i=0; i<data.size(); i++) {
    printf("0x%02x ", (unsigned int)data[i]);
  }
  printf("\n");
}

bool compare(const Component& c1, const Component& c2) {
  if (c1.name != c2.name) {
    cout << "Components have different names: "
      << c1.name << " vs " << c2.name << endl;
    return false;
  }
  if (c1.componentType != c2.componentType) {
    cout << c1.name << " components have different types: "
      << (int)c1.componentType << " vs " << (int)c2.componentType << endl;
    return false;
  }
  if (c1.typeParameter != c2.typeParameter) {
    cout << c1.name << " components have different type parameters: "
      << (int)c1.typeParameter << " vs " << (int)c2.typeParameter << endl;
    return false;
  }
  if (c1.data != c2.data) {
    cout << c1.name << " components have different content\n";
    return false;
  }
  return true;
}

// true if they are same
bool compare(Node& node1, Node& node2, path p1 = "Root", path p2 = "Root") {
  // cout << p1.string() << " - " << p2.string() << endl;
  if (node1.name != node2.name) {
    cout << "Nodes have different names: "
      << node1.name << " vs " << node2.name << endl;
    return false;
  }
  if (!compareNamableVector(node1.children, node2.children, "children")) {
    return false;
  }
  if (!compareNamableVector(node1.components, node2.components, "components")) {
    return false;
  }
  for (int i=0; i<node1.components.size(); i++) {
    assert(node1.components[i].name == node2.components[i].name);
    if (!compare(node1.components[i], node2.components[i])) {
      return false;
    }
  }
  for (int i=0; i<node1.children.size(); i++) {
    assert(node1.children[i].name == node2.children[i].name);
    if (!compare(node1.children[i], node2.children[i], p1 / node1.children[i].name, p2 / node2.children[i].name)) {
      return false;
    }
  }
  return true;
}

int main(int argc, const char *argv[])
{
  if (argc < 3)
  {
    cout << "Usage: " << argv[0] << " file1.mr file2.mr\n";
    return 1;
  }
  Node* file1 = LoadFromFile(argv[1]);
  Node* file2 = LoadFromFile(argv[2]);
  if (compare(*file1, *file2)) {
    cout << "Files are the same\n";
  } else {
    cout << "Files are different\n";
  }
}