#! /bin/bash

PROCESSES=$1
BLUR_FILTER="1 2 1 2 4 2 1 2 1"
EDGE_FILTER="-1 -1 -1 -1 0 1 1 1 1"
IMGSRC="../../waterfall"
IMGDES="images"
 
echo "applying blur filter to grayscale image....."
time mpiexec -np $PROCESSES ./imagfil -i $IMGSRC/waterfall_grey_1920_2520.data -o $IMGDES/waterfall_grey_1920_2520_blured.data -t GS -w 1920 -h 2520 -n $2 -f 3 3 $BLUR_FILTER
echo -e "done\n"

#echo "applying edge filter to grayscale image....."
#mpirun -n $PROCESSES ./imagfil -i $IMGSRC/waterfall_grey_1920_2520.data -o $IMGDES/waterfall_grey_1920_2520_edged.data -t GS -w 1920 -h 2520 -n 1 -f 3 3 $EDGE_FILTER
#echo -e "done\n"

echo "applying blur filter to rgb image....."
time mpiexec -np $PROCESSES ./imagfil -i $IMGSRC/waterfall_1920_2520.data -o $IMGDES/waterfall_1920_2520_blured.data -t RGB -w 1920 -h 2520 -n $2 -f 3 3 $BLUR_FILTER
echo -e "done\n"

#echo "applying edge filter to rgb image....."
#mpirun -n $PROCESSES ./imagfil -i $IMGSRC/waterfall_1920_2520.data -o $IMGDES/waterfall_1920_2520_edged.data -t RGB -w 1920 -h 2520 -n 1 -f 3 3 $EDGE_FILTER
#echo -e "done\n"

