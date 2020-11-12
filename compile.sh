#!/bin/bash

CURRENTDIR=`pwd`

function quit {
  exit 1;
}

function clean {
  cd $CURRENTDIR/hclasses
  rm -f *.o
  rm -f *.a

  cd $CURRENTDIR/CCDFDataModel
  rm -f *.o
  rm -f *.a

  cd $CURRENTDIR/adagucserverEC
  rm -f *.o
  rm -f adagucserver
  rm -f h5ncdump
  rm -f aggregate_time
  rm -f geojsondump

  test -d $CURRENTDIR/bin || mkdir $CURRENTDIR/bin/
  rm -f $CURRENTDIR/bin/adagucserver
  rm -f $CURRENTDIR/bin/h5ncdump
  rm -f $CURRENTDIR/bin/aggregate_time
  rm -f $CURRENTDIR/bin/geojsondump
}

function build {
  
  cd $CURRENTDIR/bin
  
  cmake .. &&  cmake --build . -v

  if [ -f adagucserver ]
    then
    echo "[OK] ADAGUC has been succesfully compiled."
    else
      echo "[FAILED] ADAGUC compilation failed"
      quit;
  fi
 
  echo "[OK] Everything is installed in the ./bin directory"
}

if [ "$*" = "--clean" ]; then
  clean
  else
  build
fi
