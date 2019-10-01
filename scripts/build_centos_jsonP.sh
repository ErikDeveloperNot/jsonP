#!/bin/bash

local_src=/Users/user1/udemy/CPP/UdemyCPP/jsonP_dyn
host=192.168.56.102

#copy files
scp ${local_src}/*.h ${local_src}/*.cpp root@${host}:/root/jsonP/src

#run build
ssh root@${host} jsonP/build.sh

#verify success
success=`ssh root@${host} ls /root/jsonP/libjsonP.so`

if [ $success == "/root/jsonP/libjsonP.so" ]
then
  #backup old
  if [ -f "libjsonP.so" ]
  then
    mv libjsonP.so libjsonP.so.`date "+%Y.%m.%d.%H.%M.%S"`
  fi
  # pull down libjsonP.so
  scp root@${host}:/root/jsonP/libjsonP.so .
else
  echo "Build may have failed"
fi
