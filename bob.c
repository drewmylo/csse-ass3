
#include "player.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUITS 4
/*
 */
int bob_choose_card(GameState *game) {
    if (game->leadPlayer == game->position) {
        for (int i = 2; i < SUITS + 2; i++) {
            if (suit_in_hand(game->players[game->position].hand,
                             game->players[game->position].cardsInHand,
                             get_suit(i))) {
                return lowest_card(game->players[game->position].hand,
                                   game->players[game->position].cardsInHand,
                                   get_suit(i));
            }
        }
    } else if (close_to_winning(game) &&
               (number_of_D_cards(game->lastMoves, game->playerCount) > 0)) {
        if (suit_in_hand(game->players[game->position].hand,
                         game->players[game->position].cardsInHand,
                         game->leadSuit)) {
            return highest_card(game->players[game->position].hand,
                                game->players[game->position].cardsInHand,
                                game->leadSuit);
        } else {
            for (int i = 0; i < 2; i++) {
                if (suit_in_hand(game->players[game->position].hand,
                                 game->players[game->position].cardsInHand,
                                 get_suit(i))) {
                    return highest_card(
                        game->players[game->position].hand,
                        game->players[game->position].cardsInHand, get_suit(i));
                }
            }
            if (suit_in_hand(game->players[game->position].hand,
                             game->players[game->position].cardsInHand,
                             get_suit(3))) {
                return highest_card(game->players[game->position].hand,
                                    game->players[game->position].cardsInHand,
                                    get_suit(3));
            }
            if (suit_in_hand(game->players[game->position].hand,
                             game->players[game->position].cardsInHand,
                             get_suit(2))) {
                return highest_card(game->players[game->position].hand,
                                    game->players[game->position].cardsInHand,
                                    get_suit(2));
            }
        }
    }

    else if (suit_in_hand(game->players[game->position].hand,
                          game->players[game->position].cardsInHand,
                          game->leadSuit)) {
        return lowest_card(game->players[game->position].hand,
                           game->players[game->position].cardsInHand,
                           game->leadSuit);
    } else {
        for (int i = 0; i < SUITS; i++) {
            if (suit_in_hand(game->players[game->position].hand,
                             game->players[game->position].cardsInHand,
                             get_suit(i))) {
                return highest_card(game->players[game->position].hand,
                                    game->players[game->position].cardsInHand,
                                    get_suit(i));
            }
        }
    }
    return 0;
}

/*
 */
int main(int argc, char const *argv[]) {
    GameState *game = calloc(1, sizeof(GameState));
    int argumentProblem = argument_checks(argc, argv, game);
    if (argumentProblem) {
        return exit_with_error(argumentProblem);
    }
    int (*bob)(GameState *);
    bob = &bob_choose_card;

    run_game(game, bob);
    return 0;
}
