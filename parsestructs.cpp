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

//from sourcetraildb examples
sourcetrail::NameHierarchy to_name_hierarchy(sourcetrail::SourcetrailDBWriter *writer, const std::string parent, const std::string str, const std::string paramstring)
{
    sourcetrail::NameHierarchy name = { "::", {} };
    name.nameElements.emplace_back( sourcetrail::NameElement({ "function", parent, paramstring}) );
    int pid = writer->recordSymbol(name);
    writer->recordSymbolDefinitionKind(pid, sourcetrail::DefinitionKind::EXPLICIT);
    writer->recordSymbolKind(pid, sourcetrail::SymbolKind::FUNCTION);

    name.nameElements.emplace_back( sourcetrail::NameElement({ "keyword", str, }) );

    return name;
}

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
    writer->beginTransaction();
    std::vector<int> ids;
    //define directly referenced functions first to ensure linking works
    for(uint i = 0; i < data.functions.size(); ++i)
    {
        std::string fnname = data.functions.at(i).name;

        //construct list of parameters (keywords)
        std::string paramstring = ": ";
        for(uint j = 0; j < data.functions.at(i).params.size(); ++j)
        {
            paramstring.append(data.functions.at(i).params.at(j)).append(", ");
        }

        int fid = writer->recordSymbol({ "::", { { "function", fnname, paramstring } } });

        writer->recordSymbolDefinitionKind(fid, sourcetrail::DefinitionKind::EXPLICIT);
        writer->recordSymbolKind(fid, sourcetrail::SymbolKind::FUNCTION);
        ids.push_back(fid);

        for(uint j = 0; j < data.functions.at(i).params.size(); ++j)
        {
            int pid = writer->recordSymbol(to_name_hierarchy(writer, fnname, data.functions.at(i).params.at(j), paramstring) );
            writer->recordSymbolDefinitionKind(pid, sourcetrail::DefinitionKind::EXPLICIT);
            writer->recordSymbolKind(pid, sourcetrail::SymbolKind::FIELD);
        }
    }
    //directly referenced common blocks
    for(abstract_common i : data.commons)
    {
        int fileId = writer->recordFile(getname(i.file, file));
        std::string comname = i.name;
        std::string file = i.file;
        int loc = i.loc;
        int id = writer->recordSymbol({ "::", { { "common block", comname, "" } } });
        writer->recordSymbolKind(id, sourcetrail::SymbolKind::STRUCT);
        writer->recordSymbolDefinitionKind(id, sourcetrail::DefinitionKind::EXPLICIT);
        writer->recordSymbolLocation(id, {fileId, loc, 1, loc, 2});
    }
    //function linking
    for(uint i = 0; i < data.functions.size(); ++i)
    {
        int fileId = writer->recordFile(getname(data.functions.at(i).file, file));
        //referenced functions
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
                writer->recordSymbolKind(id, sourcetrail::SymbolKind::FUNCTION);
                int refid = writer->recordReference(ids[i], id, sourcetrail::ReferenceKind::CALL);
                writer->recordReferenceLocation(refid, {fileId, data.functions.at(i).fn_refs_loc.at(j), 1, data.functions.at(i).fn_refs_loc.at(j), 10});
            }
        }
        //referenced system vars
        for(uint j = 0; j < data.functions.at(i).var_references.size(); ++j)
        {
            std::string refname = data.functions.at(i).var_references.at(j);

            int id = writer->recordSymbol({ "::", { { "system variable", refname, ""} } });
            writer->recordSymbolKind(id, sourcetrail::SymbolKind::GLOBAL_VARIABLE);
            int refid = writer->recordReference(ids[i], id, sourcetrail::ReferenceKind::USAGE);
            writer->recordReferenceLocation(refid, {fileId, data.functions.at(i).var_refs_loc.at(j), 1, data.functions.at(i).var_refs_loc.at(j), 2});
        }
        sourcetrail::SourceRange range = { fileId, data.functions.at(i).loc, 1, data.functions.at(i).loc, 9};
        writer->recordReferenceLocation(ids[i], range);
    }
    writer->commitTransaction();
}

void closedb(sourcetrail::SourcetrailDBWriter *writer)
{
    writer->close();
    delete writer;
}
