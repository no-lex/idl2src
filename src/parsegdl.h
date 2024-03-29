//a representation of the information a function call inside another function has
class function_call
{
    public:
        std::vector<std::string> fn_called_keywords;               //the keywords referenced by name
        std::string fn_reference;                                  //the name of the referenced function
        int ref_loc;                                               //line where the function was referenced
        int ref_loc_in_line;                                       //# of chars from beginning of line where function was referenced
        bool is_procedure;                                         //whether the function is a function (0) or procedure (1)

        function_call(std::string reference, int loc, int ref_in_line, bool pro, std::vector<std::string> fn_called)
        {
            fn_reference = reference;
            ref_loc = loc;
            ref_loc_in_line = ref_in_line;
            is_procedure = pro;
            fn_called_keywords = fn_called;
        }
};

//a representation of the information relating to parameters without a keydecl call
class abstract_implicit
{
    public:
        int argnum;
        std::string name;

        abstract_implicit(int arg, std::string abs_name)
        {
            argnum = arg;
            name = abs_name;
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
        bool is_procedure;                                      //whether the function is a function (0) or procedure (1)
        int loc_in_line;                                        //how far from the beginning of the line the beginning of the function name is
        std::vector<abstract_implicit> implicit_keywords;       //vector containing information on implicit keywords
        bool local;

        abstract_function()
        {
            loc = -1;
            size = -1;
            file = "dummy";
            name = "dummy";
            params = {};
            fn_references = {};
            var_references = {};
            var_refs_loc = {};
            is_procedure = false;
            loc_in_line = -1;
            implicit_keywords = {};
        }

        abstract_function(int location,
                          int funcsize,
                          std::string filename,
                          std::string funcname,
                          std::vector<std::string> args,
                          std::vector<function_call> references,
                          std::vector<std::string> vreferences,
                          std::vector<int> vrefs_loc,
                          bool pro,
                          int line_loc,
                          std::vector<abstract_implicit> implicits)
        {
            loc = location;
            size = funcsize;
            file = filename;
            name = funcname;
            params = args;
            fn_references = references;
            var_references = vreferences;
            var_refs_loc = vrefs_loc;
            is_procedure = pro;
            loc_in_line = line_loc;
            implicit_keywords = implicits;
            local = true;
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

        void parseast(std::string name); //assigns to the codedata object info

        //serialization/deserialization
        void serialize(std::string name);
        void deserialize(std::string name);
    private:
        //these are the string parsing helper functions for parseast()
        std::string getfunctionname(std::string line);
        std::string getvarrefname(std::string line);
        std::string getparamname(std::string line);
        std::string getfunctioncall(std::string line);
        std::string getcommonname(std::string line);
        int getfunctioncallline(std::string line);
        int getfunctionline(std::string line);
        int getvarrefline(std::string line);
        int getcommonline(std::string line);
        unsigned int loc_in_line(std::string name, int line, std::string fnname);

};

class files
{
    public:
        std::vector<std::string> asts;
        std::vector<std::string> codes;

        std::vector<std::string> loadfile(std::string name);
};
