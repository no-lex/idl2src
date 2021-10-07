#include "SourcetrailDBWriter.h"
#include "NameHierarchy.h"
#include "SourceRange.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include "parsegdl.h"
#include "parsestructs.h"

files loadfiles()
{
    files startupfiles;

    std::vector<std::string> ast = startupfiles.loadfile("files.txt");
    std::vector<std::string> code = startupfiles.loadfile("files2.txt");

    for(std::string i : ast)
    {
        startupfiles.asts.emplace_back(i);
    }
    for(std::string i : code)
    {
        startupfiles.codes.emplace_back(i);
    }
    return startupfiles;
}

int main(int argc, char **argv)
{
    //handle input arguments
    const char * name = "default.srctrldb";
    codedata output;

    bool nolink = false;
    bool serialize = false; //toggles writing the codedata object to file at the end

    std::string path1;
    std::string path2;

    std::string outfile; //name of file for serializer to write out

    for(int i = 1; i < argc; i++)
    {
        if(argv[i][0]=='-')
        {
            switch(argv[i][1])
            {
                case 'v':
                {
                    std::cout << "idl2src alpha built " << __DATE__ << "\n";
                    exit(0);
                }
                case 'h':
                {
                    std::cout << "usage: idl2src [OPTIONS]\n\n";
                    std::cout << "  -h    print this usage and exit\n";
                    std::cout << "  -v    print out version information and exit\n";
                    std::cout << "  -f    name of file to index\n";
                    std::cout << "  -s    serialize the database to this file\n";
                    std::cout << "  -d    deserialize the database into program memory\n";
                    std::cout << "  -n    name of database to output\n";
                    std::cout << "  -l    disables linking functions\n";
                    exit(0);
                }
                case 'f':
                {
                    std::cout << argv[i + 1] << "\n";
                    path1 = path2 = argv[i+1];
                    path1.append(".pro");
                    path2.append(".txt");
                    std::cout << path1 << "\n";
                    std::cout << path2 << "\n";
                    break;
                }
                case 'd':
                {
                    output.deserialize(argv[i + 1]);
                    break;
                }
                case 's':
                {
                    serialize = true;
                    outfile = argv[i + 1];
                    break;
                }
                case 'n':
                {
                    name = argv[i + 1];
                    break;
                }
                case 'l':
                {
                    nolink = true;
                    std::cout << "Skipping linking step\n";
                }
            }
        }
    }

    files sfiles = { {path2}, {path1} };

    sourcetrail::SourcetrailDBWriter *writer = createdb(name);
    for(std::string i : sfiles.asts)
    {
        output.parseast(i);
    }
    for(std::string i : sfiles.asts)
    {
        std::cout << i << "\n";
    }
    parse(output, writer, sfiles, nolink);
    if(serialize)
    {
        output.serialize(outfile);
    }
    closedb(writer);
}

