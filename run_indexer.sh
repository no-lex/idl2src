#script to run inside sourcetrail to index an IDL/GDL project

# in "Custom Command", use the following command to allow the indexer to work correctly:

# bash ./run_indexer.sh %{SOURCE_FILE_PATH} %{DATABASE_FILE_PATH}

#create txt file

echo $1
b="${1%.*}".txt
echo $b
echo ".compile" $1 > tempexe.txt
echo "exit" >> tempexe.txt
gdl < tempexe.txt > $b

#index the file
./idl2src -f "${1%.*}" -d "tempdb.txt" -s "tempdb.txt" -n $2

#delete txt file
rm "${1%.*}".txt
