#include "mr_parser/mr_parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <errno.h>
using namespace std;

void safecd(string dir) {
    if (cd(dir.c_str()) != 0) {
        throw runtime_error(string("Unable to change directory to folder ") + dir + ": " + strerror(errno));
    };
}

void safemkd(string dir) {
    if (mkd(dir.c_str()) != 0) {
        throw runtime_error(string("Unable to create folder ") + dir + ": " + strerror(errno));
    };
}

string encode(const string& path, bool fullEncode) {
    ostringstream os;
    for (char c : path) {
        if ((isalnum(c) && !fullEncode) || c == '-' || c == '_' || c == '.' || c == '/') {
            os << c;
        } else {
            os << '%' << uppercase << setw(2) << setfill('0') << hex << ((unsigned int)(unsigned char)c);
            os << nouppercase << setw(1) << dec;
        }
    }
    return os.str();
}

string getComponentFilename(const Component& component, bool fullEncode) {
    stringstream outputFname;
    outputFname << encode(component.name, fullEncode) << "--" << (int)component.componentType;
    if (component.typeParameter != 0xffff)
        outputFname << "--" << (int)component.typeParameter;
    return outputFname.str();
}

void Extract(Node* node) {
    // mkdir name
    try {
        safemkd("NODE_" + node->name);
    } catch(const exception& ex) {
        cout << "WARNING: Skipping node " << node->name << " because: " << ex.what() << endl;
        return;
    }
    // cd name
    safecd("NODE_" + node->name);
    // attribute order
    ofstream orderComponents("order_components.txt");
    // attributes
    for (const Component& component : node->components) {
        bool fullEncode = false;
        string intendedFilename = getComponentFilename(component, fullEncode);
        if (exists(intendedFilename)) {
            fullEncode = true;
            intendedFilename = getComponentFilename(component, fullEncode);
            cout << "NOTICE: Component file already exists: " << component.name;
            cout << ". This is usually because on Windows names are case in-sensitive ";
            cout << "and there exists another component with same name but different";
            cout << " casing. Don't worry! The script will fully url-encode this";
            cout << " component's name\n";
        }
        ofstream out(intendedFilename, ios::binary);
        out.write((const char*)component.data.data(), component.data.size());
        orderComponents << encode(component.name, fullEncode) << endl;
    }
    orderComponents.close();
    // node order
    ofstream orderNodes("order_nodes.txt");
    for (Node& child : node->children) {
        #if defined(_WIN32)
            if (child.name.rfind('.') == child.name.length()-1) {
                cout << "WARNING: In Windows folders cannot end with dot (.). Because of that the node '" << child.name;
                cout << "' will have to be renamed to '";
                child.name = child.name.substr(0, child.name.length() - 1);
                cout << child.name << "'" << endl;
            }
        #endif
        orderNodes << child.name << endl;
        Extract(&child);
    }
    orderNodes.close();
    // cd ..
    safecd("..");
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " file.mr\n";
        return 1;
    }
    Node *root = LoadFromFile(argv[1]);
    cout << "Root name is " << root->name << endl;
    
    string folderName = string(argv[1]) + ".extracted";
    try {
        safemkd(folderName.c_str());
        safecd(folderName.c_str());
        if (root->name != "Root") {
            string renameFile = "root_name.txt";
            ofstream out(renameFile);
            out << root->name;
            out.close();
            root->name = "Root";
        }
        Extract(root);
    } catch(const exception& ex) {
        cout << "Error extracting: " << ex.what() << endl;
    }
}