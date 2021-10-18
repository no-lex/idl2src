for f in `find . -name "*.pro" -type f`; do
    bash run_indexer.sh $f "FHD"
done
