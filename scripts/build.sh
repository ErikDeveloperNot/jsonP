#!/bin/bash

home=/root/jsonP
src=${home}/src

if [ -f libjsonP.so ]
then
  echo "deleting libjsonP.so"
  rm -f libjsonP.so
fi

/opt/rh/devtoolset-8/root/bin/g++ -fPIC ${src}/*.cpp -shared -o ${home}/libjsonP.so -Wl,-rpath,/opt/jsonP

cp ${src}/*.h /opt/jsonP/include/

rm -f ${src}/*.cpp ${src}/*.h

cp -f ${home}/libjsonP.so /opt/jsonP/
