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

//returns either "function" for false or "procedure" for true
//used to turn is_procedure into a string for recordSymbol()
std::string pro_to_str(bool ispro)
{
    if(ispro)
    {
        return "procedure";
    }
    else
    {
        return "function";
    }
}

//from sourcetraildb examples
sourcetrail::NameHierarchy to_name_hierarchy(sourcetrail::SourcetrailDBWriter *writer, const std::string parent, const std::string str, bool ispro)
{
    sourcetrail::NameHierarchy name = { "::", {} };
    name.nameElements.emplace_back( sourcetrail::NameElement({ pro_to_str(ispro), parent, ""}) );
    int pid = writer->recordSymbol(name);
    writer->recordSymbolKind(pid, sourcetrail::SymbolKind::FUNCTION);

    name.nameElements.emplace_back( sourcetrail::NameElement({ "keyword", str, "" }) );

    return name;
}

sourcetrail::SourcetrailDBWriter *createdb(const char * name)
{
    sourcetrail::SourcetrailDBWriter *writer = new sourcetrail::SourcetrailDBWriter{};
    writer->open(name);
    return writer;
}

//returns true if function exists in codedata AND the int i is one of the implicit parameters
bool getfunctionimplicit(codedata data, std::string name, int index)
{
    for(abstract_function i : data.functions)
    {
        if(i.name.find(name) != std::string::npos)
        {
            bool found = false;
            for(abstract_implicit j : i.implicit_keywords)
            {
                if(j.argnum = index)
                {
                    found = true;
                    break;
                }
            }
            if(!found)
            {
                return false;
            }
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
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
        int fileId = writer->recordFile(getname(data.functions.at(i).file, file));
        std::string fnname = data.functions.at(i).name;
        int loc = data.functions.at(i).loc; //line of function definition
        int loc_in = data.functions.at(i).loc_in_line; //location in line of function definition
        //construct list of parameters (keywords)
        std::string paramstring = ": ";
        for(uint j = 0; j < data.functions.at(i).params.size(); ++j)
        {
            paramstring.append(data.functions.at(i).params.at(j)).append(", ");
        }

        int fid = writer->recordSymbol({ "::", { { pro_to_str(data.functions.at(i).is_procedure), fnname, "" } } });

        writer->recordSymbolDefinitionKind(fid, sourcetrail::DefinitionKind::EXPLICIT);
        writer->recordSymbolKind(fid, sourcetrail::SymbolKind::FUNCTION);
        writer->recordSymbolLocation(fid, {fileId, loc, loc_in, loc, loc_in + static_cast<int>(data.functions.at(i).name.size()) - 1});
        ids.push_back(fid);

        for(uint j = 0; j < data.functions.at(i).params.size(); ++j)
        {
            int pid = writer->recordSymbol(to_name_hierarchy(writer, fnname, data.functions.at(i).params.at(j), data.functions.at(i).is_procedure) );
            writer->recordSymbolDefinitionKind(pid, sourcetrail::DefinitionKind::EXPLICIT);
            writer->recordSymbolKind(pid, sourcetrail::SymbolKind::FIELD);
            writer->recordSymbolLocation(pid, {fileId, loc, loc_in, loc, loc_in + static_cast<int>(data.functions.at(i).name.size()) - 1});
        }
        //non-explicit keywords
        for(uint j = 0; j < data.functions.at(i).implicit_keywords.size(); ++j)
        {
            writer->recordSymbol(to_name_hierarchy(writer, fnname, data.functions.at(i).implicit_keywords.at(j).name,  data.functions.at(i).is_procedure) );
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
            std::string refname = data.functions.at(i).fn_references.at(j).fn_reference;
            if(std::find(data.functions.begin(), data.functions.end(), refname) != data.functions.end())
            {
                int dist = std::distance(data.functions.begin(), std::find(data.functions.begin(), data.functions.end(), refname)),
                    refid = writer->recordReference(ids[i], ids[dist], sourcetrail::ReferenceKind::CALL);
                writer->recordReferenceLocation(refid, {fileId,
                                                        data.functions.at(i).fn_references.at(j).ref_loc,
                                                        data.functions.at(i).fn_references.at(j).ref_loc_in_line,
                                                        data.functions.at(i).fn_references.at(j).ref_loc,
                                                        data.functions.at(i).fn_references.at(j).ref_loc_in_line + static_cast<int>(data.functions.at(i).fn_references.at(j).fn_reference.size()) - 1
                                                       });
            }
            else
            {
                int id = writer->recordSymbol({ "::", { { pro_to_str(data.functions.at(i).fn_references.at(j).is_procedure), refname, ""} } });
                writer->recordSymbolKind(id, sourcetrail::SymbolKind::FUNCTION);
                int refid = writer->recordReference(ids[i], id, sourcetrail::ReferenceKind::CALL);
                writer->recordReferenceLocation(refid, {fileId,
                                                        data.functions.at(i).fn_references.at(j).ref_loc,
                                                        data.functions.at(i).fn_references.at(j).ref_loc_in_line,
                                                        data.functions.at(i).fn_references.at(j).ref_loc,
                                                        data.functions.at(i).fn_references.at(j).ref_loc_in_line + static_cast<int>(data.functions.at(i).fn_references.at(j).fn_reference.size()) - 1
                                                       });
            }
            for(uint k = 0; k < data.functions.at(i).fn_references.at(j).fn_called_keywords.size(); ++k)
            {
                if(getfunctionimplicit(data, data.functions.at(i).fn_references.at(j).fn_reference, k))
                {
                    std::string calledname = data.functions.at(i).fn_references.at(j).fn_called_keywords.at(k);
                    int id = writer->recordSymbol(to_name_hierarchy(writer,
                                                                    data.functions.at(i).fn_references.at(j).fn_reference,
                                                                    data.functions.at(i).fn_references.at(j).fn_called_keywords.at(k),
                                                                    data.functions.at(i).fn_references.at(j).is_procedure)
                                                  );

                    writer->recordSymbolKind(id, sourcetrail::SymbolKind::FIELD);
                    writer->recordReference(ids[i], id, sourcetrail::ReferenceKind::USAGE);
                }
                else
                {
                    std::cout << "rejected implicit keyword" << data.functions.at(i).fn_references.at(j).fn_called_keywords.at(k) << "\n";
                }
            }
        }
        //referenced system vars
        for(uint j = 0; j < data.functions.at(i).var_references.size(); ++j)
        {
            std::string refname = data.functions.at(i).var_references.at(j);

            int id = writer->recordSymbol({ "::", { { "system variable", refname, ""} } });
            writer->recordSymbolKind(id, sourcetrail::SymbolKind::GLOBAL_VARIABLE);
            int refid = writer->recordReference(ids[i], id, sourcetrail::ReferenceKind::USAGE);
            writer->recordReferenceLocation(refid, {fileId, data.functions.at(i).fn_references.at(j).ref_loc, 1, data.functions.at(i).fn_references.at(j).ref_loc, 2});
        }
    }
    writer->commitTransaction();
}

void closedb(sourcetrail::SourcetrailDBWriter *writer)
{
    writer->close();
    delete writer;
}
