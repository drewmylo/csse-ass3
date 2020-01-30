
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "playerError.h"

#include "player.h"

#define SUITS 4


int
alice_choose_card (GameState * game)
{
  if (game->leadPlayer == game->position) {
    for (int i = 0; i < SUITS; i++) {
      if (suit_in_hand (game->players[game->position].hand,
                        game->players[game->position].cardsInHand,
                        get_suit (i))) {
        return highest_card (game->players[game->position].hand,
                             game->players[game->position].cardsInHand,
                             get_suit (i));
      }
    }
  }
  else if (suit_in_hand (game->players[game->position].hand,
                         game->players[game->position].cardsInHand,
                         game->leadSuit)) {
    return lowest_card (game->players[game->position].hand,
                        game->players[game->position].cardsInHand,
                        game->leadSuit);
  }
  else {
    for (int i = 2; i < SUITS + 2; i++) {
      if (suit_in_hand (game->players[game->position].hand,
                        game->players[game->position].cardsInHand,
                        get_suit (i))) {
        return highest_card (game->players[game->position].hand,
                             game->players[game->position].cardsInHand,
                             get_suit (i));
      }
    }
  }
  return 0;
}

int main (int argc, char const *argv[])
{
  GameState *game = calloc (1, sizeof (GameState));
  Error argumentProblem = argument_checks (argc, argv, game);
  if (argumentProblem) {
    return exit_with_error (argumentProblem);
  }
  int (*alice) (GameState *);
  alice = &alice_choose_card;

  run_game (game, alice);

  return 0;
}
