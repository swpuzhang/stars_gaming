protoc -I=./ --cpp_out=./cpp/ ./GameCmd.proto
protoc -I=./ --cpp_out=./cpp/ ./GameMsg.proto
protoc -I=./ --cpp_out=./cpp/ ./MsgType.proto
protoc -I=./ --cpp_out=./cpp/ ./SystemMsg.proto
protoc -I=./ --cpp_out=./cpp/ ./ErrorCode.proto
protoc -I=./ --cpp_out=./cpp/ ./SystemCmd.proto
protoc -I=./ --cpp_out=./cpp/ ./MsgHeader.proto
protoc -I=./ --cpp_out=./cpp/ ./RoomCmd.proto
protoc -I=./ --cpp_out=./cpp/ ./RoomMsg.proto
protoc -I=./ --cpp_out=./cpp/ ./SvrCmd.proto
protoc -I=./ --cpp_out=./cpp/ ./SvrMsg.proto
protoc -I=./ --cpp_out=./cpp/ ./HallCmd.proto
protoc -I=./ --cpp_out=./cpp/ ./HallMsg.proto
protoc -I=./ --cpp_out=./cpp/ ./LoginCmd.proto
protoc -I=./ --cpp_out=./cpp/ ./LoginMsg.proto
set /p a=