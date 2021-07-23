//a representation of a function
class abstract_function
{
    public:
        std::vector<std::string> fn_references;
        std::vector<std::string> params;
        int loc;
        int size;
        std::string file;
        std::string name;

        abstract_function(int location, std::string funcname, std::vector<std::string> args, std::vector<std::string> references)
        {
            loc = location;
            name = funcname;
            params = args;
            fn_references = references;
        }

        bool operator==(std::string cmp)
        {
            return this->name == cmp;
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
};

//returns a codedata() object representing the structure of the codebase
extern codedata parseast();


