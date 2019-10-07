#!/bin/bash

home=/root/jsonP
src=${home}/src

if [ -f libjsonP.so.1.0 ]
then
  echo "deleting libjsonP.so.1.0"
  rm -f libjsonP.so.1.0
fi

#/opt/rh/devtoolset-8/root/bin/g++ -fPIC ${src}/*.cpp -shared -o ${home}/libjsonP.so -Wl,-rpath,/opt/jsonP

/opt/rh/devtoolset-8/root/bin/g++ -fPIC ${src}/*.cpp -shared -o ${home}/libjsonP.so.1.0 -Wl,-soname,libjsonP.so.1

cp -f ${src}/*.h /opt/jsonP/include/

rm -f ${src}/*.cpp ${src}/*.h

cp -f ${home}/libjsonP.so.1.0 /opt/jsonP/
