/*parsestructs: use the data structures generated in parsegdl to write
 * data to the sourcetrail database
 */

#include "SourcetrailDBWriter.h"
#include "NameHierarchy.h"
#include "SourceRange.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include "parsegdl.h"
#include "parsestructs.h"

const char * filename = "testfile.srctrldb";

sourcetrail::SourcetrailDBWriter *createdb()
{
    sourcetrail::SourcetrailDBWriter *writer = new sourcetrail::SourcetrailDBWriter{};
    writer->open(filename);
    return writer;
}

void parse(codedata data, sourcetrail::SourcetrailDBWriter *writer)
{
    int fileId = writer->recordFile("source_array_import.pro");
    for(uint i = 0; i < data.functions.size(); ++i)
    {
        std::string paramstring = ": ";
        for(uint j = 0; j < data.functions.at(i).params.size(); ++j)
        {
            paramstring.append(data.functions.at(i).params.at(j)).append(", ");
        }
        std::string fnname = data.functions.at(i).name;
        int fid = writer->recordSymbol({ "::", { { "function", fnname, paramstring } } });
        writer->recordSymbolDefinitionKind(fid, sourcetrail::DefinitionKind::EXPLICIT);
        writer->recordSymbolKind(fid, sourcetrail::SymbolKind::METHOD);

        for(uint j = 0; j < data.functions.at(i).fn_references.size(); ++j)
        {
            std::string refname = data.functions.at(i).fn_references.at(j);
            int id = writer->recordSymbol({ "::", { { "function", refname, ""} } });
            writer->recordSymbolKind(id, sourcetrail::SymbolKind::METHOD);
            writer->recordReference(fid, id, sourcetrail::ReferenceKind::CALL);
        }
        sourcetrail::SourceRange range = { fileId, data.functions.at(i).loc, 1, data.functions.at(i).loc, 9+fnname.size()};
        writer->recordReferenceLocation(fid, range);
    }
}

void closedb(sourcetrail::SourcetrailDBWriter *writer)
{
    writer->close();
    delete writer;
}
