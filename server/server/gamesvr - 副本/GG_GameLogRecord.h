#ifndef GG_GAMELOG_RECORD_H__
#define GG_GAMELOG_RECORD_H__

#include <string>
#include "json/json.h"
#include "GG_GameSeat.h"
#include "GG_GameArea.h"

class GG_PlayerPresentChips_Request;
class GG_SubmitGameResult_Request;

struct GameResultInfo
{
    int m_player_id;
    GG_INT64 m_net_increase;
    std::vector<PokerCard> left_hand_cards;
    GameResultInfo(int player_id, GG_INT64 net_increase) : 
        m_player_id(player_id), m_net_increase(net_increase){};
};

class GG_GameLogRecord
{
public:
	GG_GameLogRecord(void);

	~GG_GameLogRecord(void);

	void set_room_info(const GG_GameArea::AreaInfo &room_info);

	void set_game_id(const std::string& game_id);

	const std::string& get_game_id(void) const;

	void set_seat_player(std::vector<GG_GameSeat>& seats);

    void set_game_info(GG_INT64 tips_total, GG_INT64 chips_pool);

    void player_blind_chips(std::vector<GG_GameSeat>& seats);

	void deal_cards(std::vector<GG_GameSeat>& seats);

    void player_post_card(int player_id, const std::deque<PokerCard>& card_pool, bool is_active);

	void player_stand_up(int player_id, int seat_number);

	void player_sit_down(int player_id, int seat_number);

    void player_win_game(int player_id, GG_INT64 win_player_chips, GG_INT64 chips);

    void player_lose_game(int player_id, GG_INT64 lose_chips, GG_INT64 chips);

	void wait_palyer_timeout(int player_id);

    void game_result(const std::string& win_type, const std::vector<GameResultInfo>& result_info);

	void submit_game_log(void);

	void deduct_desk_tips(int palyer_id, GG_INT64 tips_count);

	void game_result(const GG_SubmitGameResultRequest& msg);

	void recover_changes(GG_INT64 changes_count);

	void active1_player(int seat_number, int player_id);

	void active2_player(int seat_number, int player_id);

	void wait1_palyer_timeout(int player_id);

	void come_back_game(int player_id, int seat_number);

	void player_escape(int player_id, int seat_number);

	void dig_card(const GG_PlayerDigCardRequest& msg);

	void wait2_palyer_timeout(int player_id);

	void post_card(const GG_PlayerPostCardRequest& msg);

	void player_drop(int user_id, int seat_num, int score, GG_INT64 chips);

	void player_declare(const GG_PlayerDeclareRequest& msg);

	void loser_declare(const GG_PlayerLoseDeclareRequest& msg);
private:
	std::string json_to_string(const Json::Value &json_value);
	std::string get_local_time_string(time_t t);

private:
	GG_INT64 m_tips_total;
    GG_INT64 m_chips_pool;
	std::string m_game_id;
	std::vector<int> m_player_id;
	Json::Value m_log_info; 
	GG_GameArea::AreaInfo m_room_info;
};

#endif //GG_GAMELOG_RECORD_H__

