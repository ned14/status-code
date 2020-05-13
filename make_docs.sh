#!/bin/bash
rm -rf doc/html/*
cd doc/html
../../../standardese --output.format=commonmark_html --input.blacklist_namespace=detail -DSTANDARDESE_IS_IN_THE_HOUSE=1 -D__cplusplus=202000L ../../include/*.hpp
cp ../../Readme.md index.md
cat standardese_entities.md >> index.md
rm standardese_*.md
cat ../custom_domain_worked_example.md >> index.md
for f in *.md
do
  echo "Rendering $f to html ..."
  echo '<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>' > ${f%.md}.html
  echo ${f%.md} >> ${f%.md}.html
  echo '</title>
</head>
<body>
' >> ${f%.md}.html
  python -m markdown $f -x markdown.extensions.fenced_code >> ${f%.md}.html
  echo '</body>
</html>' >> ${f%.md}.html
  sed -i -e 's/standardese_entities\.md/index.md/g' ${f%.md}.html
  sed -i -e 's/\.md/.html/g' ${f%.md}.html
done
rm *.md
cd ../..
