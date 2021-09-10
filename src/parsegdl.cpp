/* parsegdl.cpp: parsing of the gdl abstract output files
 *
 * This file parses a GDL debug file and stores its information as a `codedata`
 * object which is used by `parsestructs` in order to create a sourcetrail database.
 */
#include "SourcetrailDBWriter.h"
#include "NameHierarchy.h"
#include "SourceRange.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include "parsegdl.h"

/* loadfile: loads a file to a std::vector
 *
 * This function returns a std::vector of std::strings, with each string
 * corresponding to a single line from the file read in. As a result, the
 * resulting std::vector is of length {number of lines in file}.
 *
 * Parameters:
 *  - std::string name: path to the file to load
 * Returns:
 *  - std::vector<std::string> of file
 */
std::vector<std::string> files::loadfile(std::string name)
{
    std::vector<std::string> output;
    std::string line;
    std::ifstream myfile;
    myfile.open(name);

    if(!myfile.is_open())
    {
      perror("Error opening file");
      exit(EXIT_FAILURE);
    }
    while(getline(myfile, line))
    {
       output.emplace_back(line);
    }
    myfile.close();
    return output;
}

/*==============================================================================

                            Text Processing Functions

==============================================================================*/

/* getfunctionname: returns name of function from FUNCTION call
 *
 *  <[39:0]fcall(22) [92:0]N_ELEMENTS(22)
 *   [92:0]N_ELEMENTS(22)
 *  < [92:0]N_ELEMENTS
 *  N_ELEMENTS
 *
 * <[64:0]pcall(21) [92:0]PRINT(21) [27:0x7f210c631670]WARNING: obs structure not present in source_dft_multi, but flatten_spectrum was set(21) >
 *  [92:0]PRINT(21) [27:0x7f210c631670]WARNING: obs structure not present in source_dft_multi, but flatten_spectrum was set(21) >
 *  [92:0]PRINT
 *  PRINT
 *
 * Parameters:
 *  - std::string line: line from a gdl debug file containing function name
 *
 * Returns:
 *  - std::string of line with all chars except function name culled
 */
std::string codedata::getfunctionname(std::string line)
{
    line.erase(line.begin(),line.begin() + 1 + line.find(")"));
    line.erase(line.begin() + line.find("("),line.end());
    line.erase(line.begin() + line.find("["),line.begin() + 1 + line.find("]"));
    if(line.begin() == line.begin() + line.find(" "))
    {
        line.erase(line.begin(), line.begin() + 1);
    }
    return line;
}

/* getvarrefname: returns name of variable from variable reference
 *
 * Parameters:
 *  - std::string line: line from a gdl debug file containing variable name
 *
 * Returns:
 *  - std::string line with all chars except variable name culled
 */
std::string codedata::getvarrefname(std::string line)
{
    line.erase(line.begin() + line.find("["),line.begin() + 1 + line.find(")"));
    line.erase(line.begin() + line.find("("),line.begin() + 1 + line.find(")"));
    line.erase(line.begin() + line.find("<"),line.begin() + 1 + line.find("]"));
    line.erase(line.begin(),line.begin() + 1 + line.find("!"));
    line.erase(line.begin() + line.find(" "),line.begin() + 1 + line.find(">"));

    return line;
}

/* getparamname: returns name of function parameter from function call
 *
 * Parameters:
 *  - std::string line: line from a gdl debug file containing parameter (keyword)
 *
 * Returns:
 *  - std::string line with all chars except parameter name culled
 */
std::string codedata::getparamname(std::string line)
{
    line.erase(line.begin() + line.find("<"),line.begin() + 1 + line.find(")"));
    line.erase(line.begin() + line.find(" "),line.begin() + 1 + line.find("]"));
    line.erase(line.begin() + line.find("("),line.begin() + 1 + line.find(">"));
    return line;
}

/* getparamname: returns name of function name called (not defined) by file
 *
 * Parameters:
 *  - std::string line: line from a gdl debug file containing called file
 * Returns:
 *  - std::string line with all chars except function name culled
 */
std::string codedata::getfunctioncall(std::string line)
{
    line.erase(line.begin(),line.begin() + 1 + line.find(")"));
    line.erase(line.begin() + line.find("("),line.end());
    line.erase(line.begin() + line.find("["),line.begin() + 1 + line.find("]"));
    if(line.begin() == line.begin() + line.find(" "))
    {
        line.erase(line.begin(), line.begin() + 1);
    }
    return line;

}

/* getcommonname: returns name of common block defined in file
 *
 * <[26:0]commondef(4) [92:0]FUNC_DATA(4) [92:0]N_PTS(4) [92:0]X_IN(4) [92:0]Y_IN(4) [92:0]X_OUT(4) [92:0]Y_OUT(4) >
 *  [92:0]FUNC_DATA(4) [92:0]N_PTS(4) [92:0]X_IN(4) [92:0]Y_IN(4) [92:0]X_OUT(4) [92:0]Y_OUT(4) >
 * FUNC_DATA(4) [92:0]N_PTS(4) [92:0]X_IN(4) [92:0]Y_IN(4) [92:0]X_OUT(4) [92:0]Y_OUT(4) >
 * FUNC_DATA
 *
 * Parameters:
 *  - std::string line: line from a gdl debug file containing defined common block
 * Returns:
 *  - std::string line with all chars except common block name removed
 */
std::string codedata::getcommonname(std::string line)
{
    line.erase(line.begin(), line.begin() + 1 + line.find(")"));
    line.erase(line.begin(), line.begin() + 1 + line.find("]"));
    line.erase(line.begin() + line.find("("), line.end());
    return line;
}
// line retrieval functions

/* getfunctioncallline: returns line number of function name called by file
 *
 * Parameters:
 *  - std::string line: line from a gdl debug file with called function
 * Returns:
 *  - int of line where var is from
 */
int codedata::getfunctioncallline(std::string line)
{
    line.erase(line.begin() + line.find("<"),line.begin() + 1 + line.find(")"));
    line.erase(line.begin() + line.find("["),line.begin() + 1 + line.find("("));
    line.erase(line.end() - 1, line.end());
    return std::stoi(line);
}

/* getfunctionline: returns line number of function name defined by file
 *
 * Parameters:
 *  - std::string line: line from a gdl debug file with function definition
 * Returns:
 *  - int of line where var is from
 */
int codedata::getfunctionline(std::string line)
{
    line.erase(line.begin() + line.find("<"),line.begin() + 1 + line.find("("));
    line.erase(line.begin() + line.find(")"),line.begin() + 1 + line.find(")"));
    return std::stoi(line);
}

/* getvarrefline: returns line number of system variable defined by file
 *
 * Parameters:
 *  - std::string line: line from a gdl debug file containing called var
 * Returns:
 *  - int of line where var is from
 */
int codedata::getvarrefline(std::string line)
{
    line.erase(line.begin(), line.begin() + 1 + line.find("("));
    line.erase(line.begin() + line.find(")"),line.begin() + 1 + line.find(")"));
    line.erase(line.begin() + line.find(" "),line.begin() + 1 + line.find(">"));
    return std::stoi(line);
}

/* getcommonline: returns line number of common block defined by file
 *
 * <[26:0]commondef(4) [92:0]FUNC_DATA(4) [92:0]N_PTS(4) [92:0]X_IN(4) [92:0]Y_IN(4) [92:0]X_OUT(4) [92:0]Y_OUT(4) >
 * 4) [92:0]FUNC_DATA(4) [92:0]N_PTS(4) [92:0]X_IN(4) [92:0]Y_IN(4) [92:0]X_OUT(4) [92:0]Y_OUT(4) >
 * 4
 *
 * Parameters:
 *  - std::string line: line from a gdl debug file containing common block
 * Returns:
 *  - int of line where common is from
 */
int codedata::getcommonline(std::string line)
{
    line.erase(line.begin(), line.begin() + 1 + line.find("("));
    line.erase(line.begin() + line.find(")"),line.end());
    return std::stoi(line);
}

/*==============================================================================

                                Code Parsing

 =============================================================================*/
unsigned int codedata::loc_in_line(std::string name, int line, std::string fnname)
{
    files file;

    name.erase(name.begin() + name.find(".txt"), name.end());
    name.append(".pro");
    std::vector<std::string> fileinfo = file.loadfile(name);
    std::string codeline = fileinfo.at(line - 1);
    for(auto & i : codeline)
    {
        i = std::toupper(i);
    }
    return codeline.find(fnname) + 1; //+1 to resolve fencepost error
}

//returns the std::string subset of the passed vector of strings that corresponds to a nesting
std::vector<std::string> getcallbody(std::vector<std::string> fxnbody, int callline)
{
    long unsigned int callline_depth = fxnbody.at(callline).find("<");
    int block = 0;
    for(unsigned int i = callline + 1; i < fxnbody.size(); ++i)
    {
        block = i;
        if(callline_depth >= fxnbody.at(i).find("<") || callline_depth >= fxnbody.at(i).find(">"))
        {
            break;
        }
    }

    std::vector<std::string> output;
    // !FIXME slow algorithm
    for(int i = callline + 1; i < block; ++i)
    {
        output.push_back(fxnbody.at(i));
    }
    return output;
}

//deletes the local names for variables (which always come one line after !=! calls)
std::vector<std::string> removelocalnames(std::vector<std::string> fxnbody)
{
    for(unsigned int i = 0; i < fxnbody.size(); ++i)
    {
        if(fxnbody.at(i).find("!=!") != std::string::npos)
        {
            fxnbody.erase(fxnbody.begin() + i + 1, fxnbody.begin() + i + 3);
        }
    }
    return fxnbody;
}

std::vector<std::string> removenonvariablenames(std::vector<std::string> fxnbody)
{
    for(unsigned int i = 0; i < fxnbody.size(); ++i)
    {
        //if fxnbody does not contain !=! and does not contain VAR
        if(fxnbody.at(i).find("!=!") == std::string::npos && fxnbody.at(i).find("VAR") == std::string::npos)
        {
            fxnbody.erase(fxnbody.begin() + i);
            i--;
        }
    }
    return fxnbody;
}

//determines the number of implicit arguments the function has
std::vector<abstract_implicit> getimplicitargs(std::vector<std::string> file, int callline)
{

    std::vector<std::string> signature;
    std::vector<abstract_implicit> implicits;
    for(unsigned int i = callline - 1; i < file.size(); ++i)
    {
        signature.push_back(file.at(i));
        if(file.at(i).find("$") == std::string::npos) //stop adding lines if no $ to extend line
        {
            break;
        }
    }
    //now remove the beginning of the string(s) until we try to remove a section
    //containing an = (an explicit keyword)
    for(unsigned int i = 0; i < signature.size(); ++i)
    {
        int index = -1;
        std::string temp = signature.at(i);
        do
        {
            index++;
            std::string sel = temp.substr(0, temp.find(','));
            if(sel.find("=") != std::string::npos)
            {
                temp.erase(temp.begin(), temp.begin() + temp.find(',') + 1);
            }
            else
            {
                if(index > 0)
                {
                    implicits.emplace_back( abstract_implicit( index, temp.substr(0, temp.find(',')) ) );
                }
                temp.erase(temp.begin(), temp.begin() + temp.find(',') + 1);
            }
        } while(temp.find(",") != std::string::npos); //while the string has a comma
    }
    return implicits;
}

std::vector<std::string> getcodefile(std::string name)
{
    files file;
    name.erase(name.begin() + name.find(".txt"), name.end());
    name.append(".pro");
    std::vector<std::string> fileinfo = file.loadfile(name);
    return fileinfo;
}

/* parseast: adds to a codedata object information from given file
 *
 * Parameters:
 *  - std::string name: path of file to open and read
 *  - codedata output: codedata object to add symbols to
 * Returns:
 *  - codedata object containing symbols in file
 */
void codedata::parseast(std::string name)
{
    files dummy_file;
    std::vector<std::string> file = dummy_file.loadfile(name);
    std::vector<std::string> args(0);

    for(unsigned int i = 0; i < file.size(); ++i)
    {
        //get the function and its parameters
        if(file.at(i).find("]FUNCTION(") != std::string::npos || file.at(i).find("]PRO(") != std::string::npos)
        {
            //determine if the function is a procedure
            bool ispro;
            if(file.at(i).find("]PRO(") != std::string::npos)
            {
                ispro = true;
            }
            else
            {
                ispro = false;
            }

            //count number of implicit arguments

            std::vector<std::string> codefile = getcodefile(name);
            std::vector<abstract_implicit> implicits = getimplicitargs(codefile, getfunctionline(file.at(i)));

            //param listings start 2 lines below FUNCTION line
            unsigned int start = i + 2,
                           end = i + 3;
            for(unsigned int j = start; j < file.size(); ++j)
            {
                if(file.at(j).find("key") == std::string::npos)
                {
                    end = j;
                    break;
                }
            }
            std::vector<std::string> paradecl(end-start);
            std::copy(file.begin() + start, file.begin() + end, paradecl.begin());
            for(unsigned int j = 0; j < paradecl.size(); ++j)
            {
                args.emplace_back(getparamname(paradecl[j]));
            }

            //function references
            unsigned int bodystart = end,
                         bodyend =   end + 1;
            for(unsigned int j = start; j < file.size(); ++j)
            {
                if(file.at(j) == ">")
                {
                    bodyend = j;
                    break;
                }
            }
            std::vector<std::string> fxnbody(bodyend-bodystart);
            std::copy(file.begin() + bodystart, file.begin() + bodyend, fxnbody.begin());

            std::vector<function_call> references;
            std::vector<std::string> varreferences;
            std::vector<int> refs_loc;
            std::vector<int> varrefs_loc;
            for(unsigned int j = 0; j < fxnbody.size(); ++j)
            {
                if((fxnbody.at(j).find("]fcall") != std::string::npos) || (fxnbody.at(j).find("]pcall") != std::string::npos))
                {
                    //determine whether the call is to a procedure or function
                    bool callpro;
                    if(fxnbody.at(j).find("]pcall") != std::string::npos)
                    {
                        callpro = true;
                    }
                    else
                    {
                        callpro = false;
                    }

                    //determine what keywords are being referenced by the function call

                    std::vector<std::string> callbody = getcallbody(fxnbody, j);
                    callbody = removelocalnames(callbody);
                    callbody = removenonvariablenames(callbody);

                    std::vector<std::string> callvarnames;
                    for(std::string i : callbody)
                    {
                        callvarnames.push_back(getfunctioncall(i));
                    }

                    int ref_line_loc = loc_in_line(name, getfunctioncallline(fxnbody.at(j)), getfunctioncall(fxnbody.at(j)));
                    references.emplace_back( function_call(getfunctioncall(fxnbody.at(j)), getfunctioncallline(fxnbody.at(j)), ref_line_loc, callpro, callvarnames) );

                }
                if(fxnbody.at(j).find("]SYSVAR") != std::string::npos)
                {
                    varreferences.emplace_back(getvarrefname(fxnbody.at(j)));
                    varrefs_loc.emplace_back(getvarrefline(fxnbody.at(j)));
                }
            }

            //definition location in line

            int line_loc = loc_in_line(name, getfunctionline(file.at(i)), getfunctionname(file.at(i)));

            //commit to object
            functions.emplace_back(abstract_function(getfunctionline(file.at(i)),
                                                     fxnbody.size(),
                                                     name,
                                                     getfunctionname(file.at(i)),
                                                     args,
                                                     references,
                                                     varreferences,
                                                     varrefs_loc,
                                                     ispro,
                                                     line_loc,
                                                     implicits));
        }
        if(file.at(i).find("]commondef(") != std::string::npos)
        {
            commons.emplace_back(abstract_common(getcommonline(file.at(i)), name, getcommonname(file.at(i))));
        }
    }
}
