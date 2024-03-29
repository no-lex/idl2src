/* serialize.cpp: handling for serializing and deserializing the codedata object
 * for persistent storage between iterations
 *
 * Serialization is needed because of how sourcetrail's indexer works: by running
 * an executable (idl2src) for each file. Because of this, and to get information
 * about implicit keywords which are not captured by the sourcetrail database,
 * limited serialization support is included.
 *
 */

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include "parsegdl.h"

//writes data to a file at path "name", in plaintext, using
//strings delineated by === to separate data fields
//given a codedata object and using its functions vector (of abstract_function)
void codedata::serialize(std::string name)
{
    std::ofstream file;
    file.open(name);
    for(abstract_function i : functions)
    {
        file << "=== FUNCTION ===\n";
        file << i.name << "\n";

        file << "=== ISPRO ===\n";
        file << (i.is_procedure ? "1" : "0") << "\n";

        file << "=== LOCATION ===\n";
        file << i.file << "\n";
        file << "=== IMPLICIT NAMES ===\n";
        for(abstract_implicit j : i.implicit_keywords)
        {
            file << j.name << "\n";
        }
        file << "=== IMPLICIT NUMS ===\n";
        for(abstract_implicit j : i.implicit_keywords)
        {
            file << j.argnum << "\n";
        }
        file << "=== END ===\n";
    }
    file.close();
}

void codedata::deserialize(std::string name)
{
    files dummy_file;

    std::vector<std::string> lines = dummy_file.loadfile(name);

    std::vector<std::vector<std::string>> functionstack;

    //separate file into function sections
    for(unsigned int i = 0; i < lines.size() ; ++i)
    {
        if(lines.at(i).find("=== END ===") != std::string::npos)
        {
            std::vector<std::string> functions;
            for(unsigned int j = 0; j < i; ++j)
            {
                functions.push_back(lines.at(j));
                //std::cout << lines.at(j) << "\n";
            }
            //std::cout << lines.at(i) << "\n\n";

            lines.erase(lines.begin(), lines.begin() + i + 1);
            i = 0; //go back to beginning

            functionstack.push_back(functions);
        }
    }

    //parse generated function stack
    /* index i monotonically increases using the sub indices j, which loop
     * through specific parts of the file
     * for each function data in the function stack (all lines from the file which
     * correspond to a specific abstract_function object)
     *
     */
    for(std::vector<std::string> i : functionstack)
    {
        abstract_function func;
        unsigned int idx = 0;
        //pass 1: function name
        for(unsigned int j = 0; j < i.size(); ++j)
        {
            if(i.at(j).find("=== ISPRO ===") != std::string::npos)
            {
                idx = j;
                break;
            }
            if(i.at(j).find("===") != std::string::npos)
            {
                //nothing atm, skip these structuring lines
            }
            else
            {
                func.name = i.at(j);
            }
            idx = j;
        }
        //pass 2: function name
        for(unsigned int j = 0; j < i.size(); ++j)
        {
            if(i.at(j).find("=== LOCATION ===") != std::string::npos)
            {
                idx = j;
                break;
            }
            if(i.at(j).find("===") != std::string::npos)
            {
                //nothing atm, skip these structuring lines
            }
            else
            {
                func.is_procedure = i.at(j) == "1" ? true : false;
            }
            idx = j;
        }
        //pass 3: function location
        for(unsigned int j = idx; j < i.size(); ++j)
        {
            if(i.at(j).find("=== IMPLICIT NAMES ===") != std::string::npos)
            {
                idx = j;
                break;
            }
            if(i.at(j).find("===") != std::string::npos)
            {
                //nothing atm, skip these structuring lines
            }
            else
            {
                func.file = i.at(j);
            }
            idx = j;
        }
        //pass 4: names of implicit parameters
        for(unsigned int j = idx; j < i.size(); ++j)
        {
            if(i.at(j).find("=== IMPLICIT NUMS ===") != std::string::npos)
            {
                idx = j;
                break;
            }
            if(i.at(j).find("===") != std::string::npos)
            {
                //nothing atm, skip these structuring lines
            }
            else
            {
                //std::cout << i.at(j) << "IMP\n";
                abstract_implicit a(0, i.at(j));
                func.implicit_keywords.emplace_back(a);
            }
            idx = j;
        }
        //pass 5: locations of implicit parameters
        unsigned int index = 0;

        for(unsigned int j = idx; j < i.size(); ++j)
        {
            if(i.at(j).find("=== END ===") != std::string::npos)
            {
                break;
            }
            if(i.at(j).find("===") != std::string::npos)
            {
                //nothing atm, skip these structuring lines
            }
            else
            {
                func.implicit_keywords.at(index).argnum = std::stoi(i.at(j));
                index++;
            }
        }
        func.local = false;
        functions.push_back(func);
    }
}
