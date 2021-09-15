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

void serialize(codedata data, std::string name)
{
}

codedata deserialize(std::string name)
{
    codedata data;
    return data;
}
