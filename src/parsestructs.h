
extern sourcetrail::SourcetrailDBWriter *createdb(const char * name);
extern void parse(codedata data, sourcetrail::SourcetrailDBWriter *writer, files file, bool nolink);
extern void closedb(sourcetrail::SourcetrailDBWriter *writer);
