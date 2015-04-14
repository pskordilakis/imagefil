#! /bin/bash

BLUR_FILTER="1 2 1 2 4 2 1 2 1"
EDGE_FILTER="0 1 0 1 -4 1 0 1 0"
IMGSRC="../../waterfall"
IMGDES="images"
 
echo -e "applying blur filter to grayscale image....\n"
/usr/bin/time -o gs.report.txt ./imagfil -i $IMGSRC/waterfall_grey_1920_2520.data -o $IMGDES/waterfall_grey_1920_2520_blured.data -t GS -w 1920 -h 2520 -n $1 -f 3 3 $BLUR_FILTER
echo -e "done\n"

echo "report : "
cat gs.report.txt

#echo -e "applying edge filter to grayscale image....\n"
#/usr/bin/time -o gs.report.txt ./imagfil -i $IMGSRC/waterfall_grey_1920_2520.data -o $IMGDES/waterfall_grey_1920_2520_edged.data -t GS -w 1920 -h 2520 -n $1 -f 3 3 $EDGE_FILTER
#echo -e "done\n"

#echo "report : "
#cat gs.report.txt

echo -e "\napplying blur filter to rgb image.....\n"
/usr/bin/time -o rgb.report.txt ./imagfil -i $IMGSRC/waterfall_1920_2520.data -o $IMGDES/waterfall_1920_2520_blured.data -t RGB -w 1920 -h 2520 -n $1 -f 3 3 $BLUR_FILTER
echo -e "done\n"

echo "report : "
cat rgb.report.txt

