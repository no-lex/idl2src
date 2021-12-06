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

//creates a sourcetrail db that writes to the file name
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
    }
    return false;
}

//gets the parameter name at position index for the given function
std::string getimplicitname(codedata data, std::string name, int index)
{
    for(abstract_function i : data.functions)
    {
        if(i.name.find(name) != std::string::npos)
        {
            for(abstract_implicit j : i.implicit_keywords)
            {
                if(j.argnum = index)
                {
                    return j.name;
                }
            }
            return "";
        }
    }
    return "";
}
//returns the string for a code file name given an ast file name
std::string getname(std::string ast, files file)
{
    if(ast == "dummy")
    {
        return "";
    }
    unsigned int loc = std::distance(file.asts.begin(), std::find(file.asts.begin(), file.asts.end(), ast));
    if(loc >= file.codes.size())
    {
        return "";
    }
    return file.codes.at(loc);
}

//given a codedata object to read from, a sourcetrailwriter to write to, and file list,
//writes the codedata created in parsegdl to a sourcetrial writer
void parse(codedata data, sourcetrail::SourcetrailDBWriter *writer, files file, bool nolink)
{
    writer->beginTransaction();
    std::vector<int> ids;
    //define directly referenced functions first to ensure linking works
    //for(unsigned int i = 0; i < data.functions.size(); ++i)
    for(abstract_function i : data.functions)
    {
        int fileId = writer->recordFile(getname(i.file, file));
        std::string fnname = i.name;
        int loc = i.loc; //line of function definition
        int loc_in = i.loc_in_line; //location in line of function definition

        //construct list of parameters (keywords)
        std::string paramstring = ": ";
        for(unsigned int j = 0; j < i.params.size(); ++j)
        {
            paramstring.append(i.params.at(j)).append(", ");
        }
        int fid = writer->recordSymbol({ "::", { { pro_to_str(i.is_procedure), fnname, "" } } });

        writer->recordSymbolDefinitionKind(fid, sourcetrail::DefinitionKind::EXPLICIT);
        writer->recordSymbolKind(fid, sourcetrail::SymbolKind::FUNCTION);
        writer->recordSymbolLocation(fid, {fileId, loc, loc_in, loc, loc_in + static_cast<int>(i.name.size()) - 1});
        ids.push_back(fid);

        for(unsigned int j = 0; j < i.params.size(); ++j)
        {
            int pid = writer->recordSymbol(to_name_hierarchy(writer, fnname, i.params.at(j), i.is_procedure) );
            writer->recordSymbolDefinitionKind(pid, sourcetrail::DefinitionKind::EXPLICIT);
            writer->recordSymbolKind(pid, sourcetrail::SymbolKind::FIELD);
            writer->recordSymbolLocation(pid, {fileId, loc, loc_in, loc, loc_in + static_cast<int>(i.name.size()) - 1});
        }
        //non-explicit keywords
        for(unsigned int j = 0; j < i.implicit_keywords.size(); ++j)
        {
            int pid = writer->recordSymbol(to_name_hierarchy(writer, fnname, i.implicit_keywords.at(j).name, i.is_procedure) );
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
    //skip the rest of the writing if we don't want to link in the database
    if(nolink)
    {
        writer->commitTransaction();
        return;
    }
    //function linking
    for(unsigned int i = 0; i < data.functions.size(); ++i)
    {
        int fileId = writer->recordFile(getname(data.functions.at(i).file, file));
        //referenced functions
        for(unsigned int j = 0; j < data.functions.at(i).fn_references.size(); ++j)
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
            //loop through all keywords within the function references
            for(unsigned int k = 0; k < data.functions.at(i).fn_references.at(j).fn_called_keywords.size(); ++k)
            {
                //gets whether the parameter in position k has an implicit (non-explicit) name
                if(getfunctionimplicit(data, data.functions.at(i).fn_references.at(j).fn_reference, k))
                {
                    int id = writer->recordSymbol(to_name_hierarchy(writer,
                                                data.functions.at(i).fn_references.at(j).fn_reference,
                                                getimplicitname(data, data.functions.at(i).fn_references.at(j).fn_reference, k),
                                                data.functions.at(i).fn_references.at(j).is_procedure)
                              );
                    writer->recordSymbolKind(id, sourcetrail::SymbolKind::FIELD);
                    //if the function has a keyword of the same name, connect the two
                    std::string kwname = data.functions.at(i).fn_references.at(j).fn_called_keywords.at(k);
                    bool useparam = std::find(data.functions.at(i).params.begin(), data.functions.at(i).params.end(), kwname) != data.functions.at(i).params.end();
                    int paramindex = std::distance(data.functions.at(i).params.begin(), std::find(data.functions.at(i).params.begin(), data.functions.at(i).params.end(), kwname));
                    if(useparam)
                    {
                        /* ids[i] is the function id
                         * id is the keyword id
                         * USAGE is the type of link
                         */
                        std::string fnname = data.functions.at(i).name;
                        int pid = writer->recordSymbol(to_name_hierarchy(writer, fnname, data.functions.at(i).params.at(paramindex), data.functions.at(i).is_procedure) );
                        writer->recordReference(pid, id, sourcetrail::ReferenceKind::USAGE);
                    }
                    else
                    {
                        writer->recordReference(ids[i], id, sourcetrail::ReferenceKind::USAGE);
                    }
                    //std::cout << "implicit argument found called " << getimplicitname(data, data.functions.at(i).fn_references.at(j).fn_reference, k) << "\n";
                }
                //if the parameter is explicitly defined (name is defined in parameter list)
                else
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
            }
        }
        //referenced system vars
        for(unsigned int j = 0; j < data.functions.at(i).var_references.size(); ++j)
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

//closes and deletes the pointer to the database passed
void closedb(sourcetrail::SourcetrailDBWriter *writer)
{
    writer->close();
    delete writer;
}
