export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
protoc -I=./ --cpp_out=./cpp/ ./*.proto
cd cpp
make
cd ../
