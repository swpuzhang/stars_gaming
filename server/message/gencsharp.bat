protoc_csharp -i:./GameCmd.proto -o:./csharp/GameCmd.cs
protoc_csharp -i:./GameMsg.proto -o:./csharp/GameMsg.cs
protoc_csharp -i:./MsgType.proto -o:./csharp/MsgType.cs
protoc_csharp -i:./SystemMsg.proto -o:./csharp/SystemMsg.cs
protoc_csharp -i:./ErrorCode.proto -o:./csharp/ErrorCode.cs
protoc_csharp -i:./SystemCmd.proto -o:./csharp/SystemCmd.cs
protoc_csharp -i:./MsgHerder.proto -o:./csharp/MsgHerder.cs
protoc_csharp -i:./HallCmd.proto -o:./csharp/HallCmd.cs
protoc_csharp -i:./HallMsg.proto -o:./csharp/HallMsg.cs
@pause