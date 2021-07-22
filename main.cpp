#include "SourcetrailDBWriter.h"
#include "NameHierarchy.h"
#include "SourceRange.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include "parsegdl.h"
#include "parsestructs.h"

int main(int argc, char **argv)
{
    sourcetrail::SourcetrailDBWriter *writer = createdb();
    codedata data = parseast();
    parse(data, writer);
    closedb(writer);
}

