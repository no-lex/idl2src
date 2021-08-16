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
#include <algorithm>

#include "parsegdl.h"
#include "parsestructs.h"

const char * filename = "testfile.srctrldb";

sourcetrail::SourcetrailDBWriter *createdb()
{
    sourcetrail::SourcetrailDBWriter *writer = new sourcetrail::SourcetrailDBWriter{};
    writer->open(filename);
    return writer;
}

//returns the string for a code file name given an ast file name
std::string getname(std::string ast, files file)
{
    int loc = std::distance(file.asts.begin(), std::find(file.asts.begin(), file.asts.end(), ast));
    return file.codes.at(loc);
}


void parse(codedata data, sourcetrail::SourcetrailDBWriter *writer, files file)
{
    std::vector<int> ids;
    for(uint i = 0; i < data.functions.size(); ++i)
    {
        int fileId = writer->recordFile(getname(data.functions.at(i).file, file));
        std::string paramstring = ": ";
        for(uint j = 0; j < data.functions.at(i).params.size(); ++j)
        {
            paramstring.append(data.functions.at(i).params.at(j)).append(", ");
        }
        std::string fnname = data.functions.at(i).name;

        int fid = writer->recordSymbol({ "::", { { "function", fnname, paramstring } } });

        writer->recordSymbolDefinitionKind(fid, sourcetrail::DefinitionKind::EXPLICIT);
        writer->recordSymbolKind(fid, sourcetrail::SymbolKind::METHOD);
        ids.push_back(fid);
    }
    //indirectly referenced functions
    for(uint i = 0; i < data.functions.size(); ++i)
    {
        int fileId = writer->recordFile(getname(data.functions.at(i).file, file));
        for(uint j = 0; j < data.functions.at(i).fn_references.size(); ++j)
        {
            std::string refname = data.functions.at(i).fn_references.at(j);
            if(std::find(data.functions.begin(), data.functions.end(), refname) != data.functions.end())
            {
                int dist = std::distance(data.functions.begin(), std::find(data.functions.begin(), data.functions.end(), refname));
                int refid = writer->recordReference(ids[i], ids[dist], sourcetrail::ReferenceKind::CALL);
                writer->recordReferenceLocation(refid, {fileId, data.functions.at(i).fn_refs_loc.at(j), 1, data.functions.at(i).fn_refs_loc.at(j), 10});
            }
            else
            {
                int id = writer->recordSymbol({ "::", { { "function", refname, ""} } });
                writer->recordSymbolKind(id, sourcetrail::SymbolKind::METHOD);
                int refid = writer->recordReference(ids[i], id, sourcetrail::ReferenceKind::CALL);
                writer->recordReferenceLocation(refid, {fileId, data.functions.at(i).fn_refs_loc.at(j), 1, data.functions.at(i).fn_refs_loc.at(j), 10});
            }
        }
        sourcetrail::SourceRange range = { fileId, data.functions.at(i).loc, 1, data.functions.at(i).loc, 9};
        writer->recordReferenceLocation(ids[i], range);
    }
}

void closedb(sourcetrail::SourcetrailDBWriter *writer)
{
    writer->close();
    delete writer;
}
