#include "mr_parser/mr_parser.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#ifdef _WIN32
#include <direct.h>
// MSDN recommends against using getcwd & chdir names
#define cwd _getcwd
#define cd _chdir
#define mkd _mkdir
#else
#include "unistd.h"
#define cwd getcwd
#define cd chdir
#define mkd(path) mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif
using namespace std;

bool exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return true;
    }
    return false;
}

void Extract(const Node* node) {
    // mkdir name
    if (!exists(node->name.c_str())) {
        mkd(node->name.c_str());
    }
    // cd name
    cd(node->name.c_str());
    // attributes
    for (const Component& attribute : node->components) {
        ofstream out(attribute.name, ios::binary);
        out.write((const char*)attribute.data.data(), attribute.data.size());
    }
    for (const Node& child : node->children) {
        Extract(&child);
    }
    // cd ..
    cd("..");
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " file.mr [--extract]\n";
        return 1;
    }
    Node *root = LoadFromFile(argv[1]);
    root->printRecursively();
    for (int i=2; i<argc; i++) {
        if (strcmp(argv[i], "--extract") == 0) {
            Extract(root);
        }
    }
    // Node* os = root->getChildByPath("os");
    // ofstream functionNames("names.txt");
    // functionNames << "{";
    // if (os != nullptr) {
    //     for (const Component& attribute : os->components) {
    //         functionNames << "'" << attribute.name << "': " << attribute.getDataAs<uint32_t>() << ",";
    //     }
    // }
    // functionNames << "}";
}