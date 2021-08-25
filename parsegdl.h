//a representation of the information a function call inside another function has
class function_call
{
    public:
        std::vector<std::string> fn_called_keywords;               //the keywords referenced by name
        std::string fn_reference;                                  //the name of the referenced function
        int ref_loc;

        function_call(std::string reference, int loc)
        {
            fn_reference = reference;
            ref_loc = loc;
        }
};

//a representation of a function
class abstract_function
{
    public:
        std::vector<function_call> fn_references;               //vector containing information for each function this function calls
        std::vector<std::string> var_references;                //vector of std::string of refernced var calls
        std::vector<int> var_refs_loc;                          //vector of int of external var use
        std::vector<std::string> params;                        //vector of std::string of function's parameters (keywords)
        int loc;                                                //line the function is defined on
        int size;                                               //number of lines the function body takes
        std::string file;                                       //std::string representation of code file path
        std::string name;                                       //std::string representation of function name

        abstract_function(int location,
                          int funcsize,
                          std::string filename,
                          std::string funcname,
                          std::vector<std::string> args,
                          std::vector<function_call> references,
                          std::vector<std::string> vreferences,
                          std::vector<int> vrefs_loc)
        {
            loc = location;
            size = funcsize;
            file = filename;
            name = funcname;
            params = args;
            fn_references = references;
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


