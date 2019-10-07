#!/bin/bash

local_src=/Users/user1/udemy/CPP/UdemyCPP/jsonP_dyn
host=192.168.56.102

#copy files
scp ${local_src}/*.h ${local_src}/*.cpp root@${host}:/root/jsonP/src

#run build
ssh root@${host} jsonP/build.sh

#verify success
success=`ssh root@${host} ls /root/jsonP/libjsonP.so.1.0`

if [ $success == "/root/jsonP/libjsonP.so.1.0" ]
then
  #backup old
  if [ -f "libjsonP.so.1.0" ]
  then
    mv libjsonP.so.1.0 libjsonP.so.1.0.`date "+%Y.%m.%d.%H.%M.%S"`
  fi
  # pull down libjsonP.so
  scp root@${host}:/root/jsonP/libjsonP.so.1.0 .
else
  echo "Build may have failed"
fi
