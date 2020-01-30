#ifndef PLAYER_H
#define PLAYER_H
#include "utils.h"
#include "playerError.h"

#define BUFFSIZE 512

void play_card(GameState *game, int choice);
char get_suit(int suitNum);
Error string_to_card(char *card, Card *parsedCard);
Error argument_checks(int argc, char const *argv[], GameState *game);
Error read_hand_message(char *input, GameState *game);
Error read_new_round_message(char *input, GameState *game);
Error read_played_message(char *input, GameState *game);
Error read_gameover_message(char *input, GameState *game);
Error parse_input(GameState *game);
Error run_game(GameState *game, int (*chooser)(GameState *));

#endif
