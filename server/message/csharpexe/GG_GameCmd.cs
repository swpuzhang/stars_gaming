//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

// Generated from: GG_GameCmd.proto
namespace GG_GameCmd
{
    [global::ProtoBuf.ProtoContract(Name=@"GG_EnumGameCmd")]
    public enum GG_EnumGameCmd
    {
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_BEGIN", Value=0)]
      GG_CMD_BEGIN = 0,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_ENTER_ROOM", Value=1000)]
      GG_CMD_APPLY_ENTER_ROOM = 1000,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_QUIT_ROOM", Value=1001)]
      GG_CMD_APPLY_QUIT_ROOM = 1001,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_FLUSH_ROOM", Value=1002)]
      GG_CMD_FLUSH_ROOM = 1002,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_SEAT", Value=1003)]
      GG_CMD_APPLY_SEAT = 1003,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_PLAYER_SITDOWN", Value=1004)]
      GG_CMD_PLAYER_SITDOWN = 1004,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_STANDUP", Value=1005)]
      GG_CMD_APPLY_STANDUP = 1005,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_PLAYER_STANDUP", Value=1006)]
      GG_CMD_PLAYER_STANDUP = 1006,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_FLUSHROOM", Value=1007)]
      GG_CMD_APPLY_FLUSHROOM = 1007,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_PLAYER_BUYIN", Value=1008)]
      GG_CMD_PLAYER_BUYIN = 1008,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_DEAL_CARDS", Value=1009)]
      GG_CMD_DEAL_CARDS = 1009,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_FACESHOW", Value=1010)]
      GG_CMD_APPLY_FACESHOW = 1010,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_PLAYER_FACESHOW", Value=1011)]
      GG_CMD_PLAYER_FACESHOW = 1011,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_CHAT", Value=1012)]
      GG_CMD_APPLY_CHAT = 1012,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_PLAYER_CHAT", Value=1013)]
      GG_CMD_PLAYER_CHAT = 1013,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_USE_PROPS", Value=1014)]
      GG_CMD_APPLY_USE_PROPS = 1014,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_PLAYER_USE_PROPS", Value=1015)]
      GG_CMD_PLAYER_USE_PROPS = 1015,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_PRESENT_CHIPS", Value=1016)]
      GG_CMD_APPLY_PRESENT_CHIPS = 1016,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_PLAYER_PRESENT_CHIPS", Value=1017)]
      GG_CMD_PLAYER_PRESENT_CHIPS = 1017,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_PRESENT_GIFT", Value=1018)]
      GG_CMD_APPLY_PRESENT_GIFT = 1018,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_PLAYER_PRESENT_GIFT", Value=1019)]
      GG_CMD_PLAYER_PRESENT_GIFT = 1019,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_PLAYER_ENTER_ROOM", Value=1020)]
      GG_CMD_PLAYER_ENTER_ROOM = 1020,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_PLAYER_QUIT_ROOM", Value=1021)]
      GG_CMD_PLAYER_QUIT_ROOM = 1021,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_TRANSMIT_MESSAGE", Value=1022)]
      GG_CMD_APPLY_TRANSMIT_MESSAGE = 1022,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_PLAYER_TRANSMIT_MESSAGE", Value=1023)]
      GG_CMD_PLAYER_TRANSMIT_MESSAGE = 1023,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_USER_TOKEN_LOGIN_GAMESVR", Value=1024)]
      GG_CMD_USER_TOKEN_LOGIN_GAMESVR = 1024,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_PRESENT_GIFT_NEWINTER", Value=1025)]
      GG_CMD_APPLY_PRESENT_GIFT_NEWINTER = 1025,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_PLAYER_PRESENT_GIFT_NEWINTER", Value=1026)]
      GG_CMD_PLAYER_PRESENT_GIFT_NEWINTER = 1026,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_TELL_TIME_MODE", Value=1027)]
      GG_CMD_TELL_TIME_MODE = 1027,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_WAIT_PLAYER_JOIN", Value=1028)]
      GG_CMD_WAIT_PLAYER_JOIN = 1028,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_PLAYER_TRUSTEE_ON", Value=1029)]
      GG_CMD_PLAYER_TRUSTEE_ON = 1029,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_TRUSTEE_OFF", Value=1030)]
      GG_CMD_APPLY_TRUSTEE_OFF = 1030,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_PLAYER_TRUSTEE_OFF", Value=1031)]
      GG_CMD_PLAYER_TRUSTEE_OFF = 1031,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_POST_CARD", Value=1032)]
      GG_CMD_APPLY_POST_CARD = 1032,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_PLAYER_POST_CARD", Value=1033)]
      GG_CMD_PLAYER_POST_CARD = 1033,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_TIMELY_ACCOUNT", Value=1034)]
      GG_CMD_TIMELY_ACCOUNT = 1034,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_GAME_OVER", Value=1035)]
      GG_CMD_GAME_OVER = 1035,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_ACTIVE_PLAYER", Value=1036)]
      GG_CMD_ACTIVE_PLAYER = 1036,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_CHIPS_CHANGE", Value=1037)]
      GG_CMD_APPLY_CHIPS_CHANGE = 1037,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_CHIPS_CHANGE", Value=1038)]
      GG_CMD_CHIPS_CHANGE = 1038,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_READY", Value=1039)]
      GG_CMD_APPLY_READY = 1039,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_NOTIFY_UPDATE_READY_STATUS", Value=1040)]
      GG_CMD_NOTIFY_UPDATE_READY_STATUS = 1040,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_AUTO_STANDUP", Value=1041)]
      GG_CMD_APPLY_AUTO_STANDUP = 1041,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_CANCLE_AUTO_STANDUP", Value=1042)]
      GG_CMD_APPLY_CANCLE_AUTO_STANDUP = 1042,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_DRAG_CARD", Value=1043)]
      GG_CMD_APPLY_DRAG_CARD = 1043,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_BACK_CARD", Value=1044)]
      GG_CMD_APPLY_BACK_CARD = 1044,
            
      [global::ProtoBuf.ProtoEnum(Name=@"GG_CMD_APPLY_CLOSE_ACCOUNT_BOX", Value=1045)]
      GG_CMD_APPLY_CLOSE_ACCOUNT_BOX = 1045
    }
  
}