#ifndef INPUT_H
#define INPUT_C

enum game_keys {
	KUP, KDOWN, KLEFT, KRIGHT,
       	KEYA, KEYB, KEYX, KEYY,
	LKUP, LKDOWN, LKLEFT, LKRIGHT,
	LKEYA, LKEYB, LKEYX, LKEYY,
	GAME_NKEYS
};

int is_key_down(int game_key);
int is_first_pressed(int game_key);
void redefine_key(int game_key, int ksc);
int get_game_key_value(int game_key);

#endif
