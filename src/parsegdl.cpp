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
std::vector<std::string> loadfile(std::string name)
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
std::string getfunctionname(std::string line)
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
std::string getvarrefname(std::string line)
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
std::string getparamname(std::string line)
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
std::string getfunctioncall(std::string line)
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
std::string getcommonname(std::string line)
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
int getfunctioncallline(std::string line)
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
int getfunctionline(std::string line)
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
int getvarrefline(std::string line)
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
int getcommonline(std::string line)
{
    line.erase(line.begin(), line.begin() + 1 + line.find("("));
    line.erase(line.begin() + line.find(")"),line.end());
    return std::stoi(line);
}

/*==============================================================================

                                Code Parsing

 =============================================================================*/

/* parseast: adds to a codedata object information from given file
 *
 * Parameters:
 *  - std::string name: path of file to open and read
 *  - codedata output: codedata object to add symbols to
 * Returns:
 *  - codedata object containing symbols in file
 */
codedata parseast(std::string name, codedata output)
{
    std::vector<std::string> file = loadfile(name);
    std::vector<std::string> args(0);

    for(uint i = 0; i < file.size(); ++i)
    {
        std::cout << file.at(i) << "\n";
    }
    for(uint i = 0; i < file.size(); ++i)
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

            //param listings start 2 lines below FUNCTION line
            uint start = i + 2,
                 end = i + 3;
            for(uint j = start; j < file.size(); ++j)
            {
                if(file.at(j).find("key") == std::string::npos)
                {
                    end = j;
                    break;
                }
            }
            std::vector<std::string> paradecl(end-start);
            std::copy(file.begin() + start, file.begin() + end, paradecl.begin());
            for(uint j = 0; j < paradecl.size(); ++j)
            {
                std::cout << getparamname(paradecl[j]) << "\n";
                args.emplace_back(getparamname(paradecl[j]));
            }

            std::cout << getfunctionname(file.at(i)) << " " << getfunctionline(file.at(i)) << "\n";

            //function references
            uint bodystart = end,
                 bodyend =   end + 1;
            for(uint j = start; j < file.size(); ++j)
            {
                if(file.at(j) == ">")
                {
                    bodyend = j;
                    break;
                }
            }
            std::vector<std::string> fxnbody(bodyend-bodystart);
            std::copy(file.begin() + bodystart, file.begin() + bodyend, fxnbody.begin());
            std::cout << "size " << bodystart << " " << bodyend << "\n";

            std::vector<function_call> references;
            std::vector<std::string> varreferences;
            std::vector<int> refs_loc;
            std::vector<int> varrefs_loc;
            for(uint j = 0; j < fxnbody.size(); ++j)
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
                    std::cout << getfunctioncall(fxnbody.at(j)) << "\n";
                    std::cout << getfunctioncallline(fxnbody.at(j)) << "\n";
                    references.emplace_back( function_call(getfunctioncall(fxnbody.at(j)), getfunctioncallline(fxnbody.at(j)), callpro) );

                }
                if(fxnbody.at(j).find("]SYSVAR") != std::string::npos)
                {
                    std::cout << getvarrefname(fxnbody.at(j)) << "\n";
                    varreferences.emplace_back(getvarrefname(fxnbody.at(j)));
                    std::cout << getvarrefline(fxnbody.at(j)) << "\n";
                    varrefs_loc.emplace_back(getvarrefline(fxnbody.at(j)));
                }
            }

            //commit to object
            output.functions.emplace_back(abstract_function(getfunctionline(file.at(i)),
                                                            fxnbody.size(),
                                                            name,
                                                            getfunctionname(file.at(i)),
                                                            args,
                                                            references,
                                                            varreferences,
                                                            varrefs_loc,
                                                            ispro));
        }
        if(file.at(i).find("]commondef(") != std::string::npos)
        {
            std::cout << getcommonname(file.at(i)) << "\n";
            std::cout << getcommonline(file.at(i)) << "\n";
            output.commons.emplace_back(abstract_common(getcommonline(file.at(i)), name, getcommonname(file.at(i))));
        }
    }
    return output;
}
