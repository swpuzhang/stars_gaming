protoc -I=./ --cpp_out=./cpp/ ./GameCmd.proto
protoc -I=./ --cpp_out=./cpp/ ./GameMsg.proto
protoc -I=./ --cpp_out=./cpp/ ./MsgType.proto
protoc -I=./ --cpp_out=./cpp/ ./SystemMsg.proto
protoc -I=./ --cpp_out=./cpp/ ./ErrorCode.proto
protoc -I=./ --cpp_out=./cpp/ ./SystemCmd.proto
protoc -I=./ --cpp_out=./cpp/ ./MsgHeader.proto
protoc -I=./ --cpp_out=./cpp/ ./MatchCmd.proto
protoc -I=./ --cpp_out=./cpp/ ./MatchMsg.proto
protoc -I=./ --cpp_out=./cpp/ ./SvrCmd.proto
protoc -I=./ --cpp_out=./cpp/ ./SvrMsg.proto
protoc -I=./ --cpp_out=./cpp/ ./LobbyCmd.proto
protoc -I=./ --cpp_out=./cpp/ ./LobbyMsg.proto
protoc -I=./ --cpp_out=./cpp/ ./LoginCmd.proto
protoc -I=./ --cpp_out=./cpp/ ./LoginMsg.proto
set /p a=