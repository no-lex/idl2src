
extern sourcetrail::SourcetrailDBWriter *createdb();
extern void parse(codedata data, sourcetrail::SourcetrailDBWriter *writer);
extern void closedb(sourcetrail::SourcetrailDBWriter *writer);
