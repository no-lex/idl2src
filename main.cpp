#include "SourcetrailDBWriter.h"
#include "NameHierarchy.h"
#include "SourceRange.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include "parsegdl.h"
#include "parsestructs.h"

static const std::string name = "myfile.txt";

int main(int argc, char **argv)
{
    sourcetrail::SourcetrailDBWriter *writer = createdb();
    codedata data = parseast(name);
    parse(data, writer);
    closedb(writer);
}

