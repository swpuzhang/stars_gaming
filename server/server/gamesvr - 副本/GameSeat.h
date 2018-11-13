#ifndef GAME_SEAT_H__
#define GAME_SEAT_H__


#include "GG_CardCombination.h"
#include "CardDealer.h"
#include "GG_GameMsg.pb.h"
#include <set>

class GameDesk;
class Player;


class GameSeat
{
public:
	enum
	{
		seated = 0x0001,
		ingame = 0x0002,
		leave = 0x0004,
		win = 0x0008,
		cur_join = 0x0010,
		abort = 0x00020,
		escape = 0x00040,
		drop = 0x00080,
		declared = 0x000100,
	};
	

public:
	GameSeat(void);

	virtual ~GameSeat(void);

	void ready_for_game(void);

	void set_game_desk(GameDesk *desk);
	
	void set_seat_number(int number);

	int get_seat_number(void) const;

	PLAYER_PTR  get_seated_player(void)  const;

	int seat_status(void);

	int seat_status(int new_status);

	bool is_seated(void) const;

	bool is_in_game(void) const;

	bool is_winner(void) const;

	void player_stand_up(void);

	void game_over();

	void player_sit_down(PLAYER_PTR player);

	void deal_cards(const std::vector<PokerCard> &hand_cards);

	const std::vector<PokerCard>& get_hand_cards_total(void);

    TY_INT64 get_match_chips_account(void);

	void set_match_chips_account(TY_INT64 chips) { m_match_over_account_chips = chips; }

	TY_INT64 match_over_account(TY_INT64 chips);

    void send_match_over_account(void);

	int get_ingame_player_id() { return m_ingame_player_id; };

	bool is_player_joined() const {return m_seat_stauts & GameSeat::cur_join;}

	bool is_player_escape();

	PokerCard post_default_card();

	void change_chips_increament(const TY_INT64 chips){ m_match_over_account_chips += chips;}

	TY_INT64 game_over_account(const TY_INT64 small_blind);

	void reset_seat();

	void dig_card(const PokerCard &card);

	bool discard_card(const PokerCard&  card);

	void player_drop(bool drop_card);

	void drop_win();

	void borning_over();

	bool is_drop() { return m_seat_stauts & drop; }

	bool is_declared() { return m_seat_stauts & declared; }

	int get_score() { return m_score; }

	int get_card_score(){ return m_card_score; }

	PokerCard& get_dig_card() { return m_dig_card; }

	bool check_declare(const GG_ApplyDeclareRequest& msg);

	bool check_lose_declare(const GG_ApplyLoseDeclareRequest& msg, int& score);

	void default_declare(int& score, std::vector<CardGroup>& cardGroup);

	std::vector<CardGroup>& get_card_group() { return m_card_group; }
private:
	std::string get_status_name(int status) const;

private:
	GameDesk *m_desk;
	PLAYER_PTR m_player;
	int m_seat_number;
	int m_seat_stauts;
	int m_ingame_player_id;
    CARD_SEQUNCE m_handcards_total;
	TY_INT64 m_match_over_account_chips;
	int m_score;
	int m_card_score;
	PokerCard m_dig_card;
	std::vector<CardGroup> m_card_group;
};

#endif  //GAME_SEAT_H__

