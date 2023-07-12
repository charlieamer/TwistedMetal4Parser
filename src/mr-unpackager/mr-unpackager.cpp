#include "mr_parser/mr_parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#if defined(_WIN32)
#include <direct.h>
// MSDN recommends against using getcwd & chdir names
#define cwd _getcwd
#define cd _chdir
#define mkd _mkdir
#else
#include "unistd.h"
#include <sys/stat.h>
#define cwd getcwd
#define cd chdir
#define mkd(path) mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif
using namespace std;

void Extract(const Node* node) {
    // mkdir name
    mkd(("NODE_" + node->name).c_str());
    // cd name
    cd(("NODE_" + node->name).c_str());
    // attribute order
    ofstream orderComponents("order_components.txt");
    // attributes
    for (const Component& attribute : node->components) {
        stringstream outputFname;
        outputFname << attribute.name
            << "--" << (int)attribute.componentType;
        if (attribute.typeParameter != 0xffff)
            outputFname << "--" << (int)attribute.typeParameter;
        ofstream out(outputFname.str(), ios::binary);
        out.write((const char*)attribute.data.data(), attribute.data.size());
        orderComponents << attribute.name << endl;
    }
    orderComponents.close();
    // node order
    ofstream orderNodes("order_nodes.txt");
    for (const Node& child : node->children) {
        orderNodes << child.name << endl;
        Extract(&child);
    }
    orderNodes.close();
    // cd ..
    cd("..");
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " file.mr\n";
        return 1;
    }
    Node *root = LoadFromFile(argv[1]);
    root->printRecursively();
    cout << "Root name is " << root->name << endl;
    
    string folderName = string(argv[1]) + ".extracted";
    mkd(folderName.c_str());
    cd(folderName.c_str());
    Extract(root);
}