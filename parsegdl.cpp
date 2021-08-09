/* parsing of the gdl abstract output files
 */
#include "SourcetrailDBWriter.h"
#include "NameHierarchy.h"
#include "SourceRange.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include "parsegdl.h"

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

std::string getfunctionname(std::string line)
{
    line.erase(line.begin() + line.find("["),line.begin() + 1 + line.find(")"));
    line.erase(line.begin() + line.find("("),line.begin() + 1 + line.find(")"));
    line.erase(line.begin() + line.find("<"),line.begin() + 1 + line.find("]"));
    return line;
}

std::string getparamname(std::string line)
{
    line.erase(line.begin() + line.find("<"),line.begin() + 1 + line.find(")"));
    line.erase(line.begin() + line.find(" "),line.begin() + 1 + line.find("]"));
    line.erase(line.begin() + line.find("("),line.begin() + 1 + line.find(">"));
    return line;
}

std::string getfunctioncall(std::string line)
{
    line.erase(line.begin() + line.find("<"),line.begin() + 1 + line.find(")"));
    line.erase(line.begin() + line.find(" "),line.begin() + 1 + line.find("]"));
    line.erase(line.begin() + line.find("("),line.begin() + 1 + line.find(")"));
    return line;

}
int getfunctioncallline(std::string line)
{
    line.erase(line.begin() + line.find("<"),line.begin() + 1 + line.find(")"));
    line.erase(line.begin() + line.find("["),line.begin() + 1 + line.find("("));
    line.erase(line.end() - 1, line.end());
    return std::stoi(line);
}

int getfunctionline(std::string line)
{
    line.erase(line.begin() + line.find("<"),line.begin() + 1 + line.find("("));
    line.erase(line.begin() + line.find(")"),line.begin() + 1 + line.find(")"));
    return std::stoi(line);
}

codedata parseast(std::string name)
{
    codedata output;
    std::vector<std::string> file = loadfile(name);
    std::vector<std::string> args(0);

    for(uint i = 0; i < file.size(); ++i)
    {
        std::cout << file.at(i) << "\n";
    }
    //get the function and its parameters
    for(uint i = 0; i < file.size(); ++i)
    {
        if(file.at(i).find("FUNCTION") != std::string::npos)
        {
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

            std::vector<std::string> references;
            std::vector<int> refs_loc;
            for(uint j = 0; j < fxnbody.size(); ++j)
            {
                if((fxnbody.at(j).find("fcall") != std::string::npos) || (fxnbody.at(j).find("pcall") != std::string::npos))
                {
                    std::cout << getfunctioncall(fxnbody.at(j)) << "\n";
                    std::cout << getfunctioncallline(fxnbody.at(j)) << "\n";
                    references.emplace_back(getfunctioncall(fxnbody.at(j)));
                    refs_loc.emplace_back(getfunctioncallline(fxnbody.at(j)));
                }
            }
            //commit to object
            output.functions.emplace_back(abstract_function(getfunctionline(file.at(i)), name, getfunctionname(file.at(i)), args, references, refs_loc));
        }
    }
    return output;
}
