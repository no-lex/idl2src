
extern sourcetrail::SourcetrailDBWriter *createdb();
extern void parse(codedata data, sourcetrail::SourcetrailDBWriter *writer, files file);
extern void closedb(sourcetrail::SourcetrailDBWriter *writer);
