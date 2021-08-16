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

    startupfiles.asts.emplace_back("files/source_array_import.txt");
    startupfiles.codes.emplace_back("files/source_array_import.pro");
    startupfiles.asts.emplace_back("files/source_list_expand.txt");
    startupfiles.codes.emplace_back("files/source_list_expand.pro");
    startupfiles.asts.emplace_back("files/source_dft_model.txt");
    startupfiles.codes.emplace_back("files/source_dft_model.pro");
    startupfiles.asts.emplace_back("files/source_dft_multi.txt");
    startupfiles.codes.emplace_back("files/source_dft_multi.pro");
    return startupfiles;
}

int main(int argc, char **argv)
{
    files sfiles = loadfiles();
    sourcetrail::SourcetrailDBWriter *writer = createdb();
    codedata output;
    output = parseast(sfiles.asts.at(0), output);
    output = parseast(sfiles.asts.at(1), output);
    output = parseast(sfiles.asts.at(2), output);
    output = parseast(sfiles.asts.at(3), output);
    parse(output, writer, sfiles);
    closedb(writer);
}

