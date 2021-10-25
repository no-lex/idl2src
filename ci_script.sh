touch tempexe.txt
touch tempdb.txt
for f in `find . -name "*.pro" -type f`; do
    ./run_indexer.sh $f "FHD.srctrldb"
done
