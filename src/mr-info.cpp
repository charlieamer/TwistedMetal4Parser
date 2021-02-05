#include "mr_parser/mr_parser.h"
#include <iostream>
#include <fstream>
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

void Extract(const Node* node) {
    // mkdir name
    mkd(node->name.c_str());
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
    
    string folderName = replaceFileExtension(argv[1], "extracted");
    mkd(folderName.c_str());
    cd(folderName.c_str());
    Extract(root);
}