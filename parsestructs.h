
extern sourcetrail::SourcetrailDBWriter *createdb(const char * name);
extern void parse(codedata data, sourcetrail::SourcetrailDBWriter *writer, files file);
extern void closedb(sourcetrail::SourcetrailDBWriter *writer);
