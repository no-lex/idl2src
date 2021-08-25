#script to run inside sourcetrail to index an IDL/GDL project

# in "Custom Command", use the following command to allow the indexer to work correctly:

# bash ./run_indexer.sh %{SOURCE_FILE_PATH} %{DATABASE_FILE_PATH}

#create txt file

echo $1
b="${1%.*}".txt
echo $b
echo ".compile" $1 > temp.txt
echo "exit" >> temp.txt
gdl < temp.txt > $b

#index the file
./idl2src -f "${1%.*}" -n $2

#delete txt file
rm "${1%.*}".txt
