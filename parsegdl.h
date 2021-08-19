//a representation of a function
class abstract_function
{
    public:
        std::vector<std::string> fn_references;
        std::vector<int> fn_refs_loc;
        std::vector<std::string> var_references;
        std::vector<int> var_refs_loc;
        std::vector<std::string> params;
        int loc;
        int size;
        std::string file;
        std::string name;

        abstract_function(int location,
                          std::string filename,
                          std::string funcname,
                          std::vector<std::string> args,
                          std::vector<std::string> freferences,
                          std::vector<int> frefs_loc,
                          std::vector<std::string> vreferences,
                          std::vector<int> vrefs_loc)
        {
            loc = location;
            file = filename;
            name = funcname;
            params = args;
            fn_references = freferences;
            fn_refs_loc = frefs_loc;
            var_references = vreferences;
            var_refs_loc = vrefs_loc;
        }

        bool operator==(std::string cmp)
        {
            return this->name == cmp;
        }
};

//a representation of a common block
class abstract_common
{
    public:
        int loc;
        std::string file;
        std::string name;

        abstract_common(int location, std::string filename, std::string comname)
        {
            loc = location;
            file = filename;
            name = comname;
        }
};

//a representation of a keyword
class abstract_keyword
{
    public:
        int loc;
        std::string file;
        std::string name;

        abstract_keyword(int location, std::string funcname)
        {
            loc = location;
            name = funcname;
        }
};

//the object containing all of the objects to be added to the database
class codedata
{
    public:
        std::vector<abstract_function> functions;
        std::vector<abstract_keyword> keywords;
        std::vector<abstract_common> commons;
};

struct files
{
    std::vector<std::string> asts;
    std::vector<std::string> codes;
};
//returns a vector of strings from a file
extern std::vector<std::string> loadfile(std::string name);

//returns a codedata() object representing the structure of the codebase
extern codedata parseast(std::string name, codedata output);


