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

void serialize_codedata(codedata data, std::string name)
{
    std::ofstream file;
    file.open(name);
    for(abstract_function i : data.functions)
    {
        file << "=== FUNCTION ===\n";
        file << i.name << "\n";
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

codedata deserialize_codedata(std::string name)
{
    codedata data;

    files dummy_file;

    std::vector<std::string> lines = dummy_file.loadfile("temp1.txt");

    for(unsigned int i = 0; i < lines.size() ; ++i)
    {
        std::cout << lines.at(i) << "\n";
    }

    return data;
}
