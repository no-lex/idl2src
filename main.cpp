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

    std::vector<std::string> ast = loadfile("files2.txt");
    std::vector<std::string> code = loadfile("files.txt");

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
    files sfiles = loadfiles();
    sourcetrail::SourcetrailDBWriter *writer = createdb();
    codedata output;
    for(uint i = 0; i < sfiles.asts.size(); ++i)
    {
        output = parseast(sfiles.asts.at(i), output);
    }
    parse(output, writer, sfiles);
    closedb(writer);
}

