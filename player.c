#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "player.h"
#include "playerError.h"
#include "utils.h"

#define BUFFSIZE 512

/**
 * Frees used memeory and exits with the error code provided.
 * @param game,   current gamestate
 * @param status, the status to exit with
 */
void end_game(GameState *game, Error status) {
    free_mem(game);

    exit(exit_with_error(status));
}

/**
 * Converts a string representation of a card to its struct form.
 * @param  card       string representation of card
 * @param  parsedCard
 * @return            [description]
 */
Error string_to_card(char *card, Card *parsedCard) {
    if (card[0] != 'S' && card[0] != 'C' && card[0] != 'D' && card[0] != 'H') {
        return INVALID_MESSAGE;
    }
    if (card[1] < '1' && card[1] > '9' && card[1] < 'a' && card[1] > 'f') {
        return INVALID_MESSAGE;
    }
    parsedCard->suit = card[0];
    parsedCard->rank = card[1];
    return NORMAL;
}

Error argument_checks(int argc, char const *argv[], GameState *game) {
    unsigned int numberOfPlayers, playerNumber, threshold, initHandSize;
    char *err1, *err2, *err3, *err4;

    // Check for correct number of arguments
    if (argc != 5) {
        end_game(game, BAD_ARGS_NUM);
    }
    numberOfPlayers = strtoul(argv[1], &err1, 10);
    playerNumber = strtoul(argv[2], &err2, 10);
    threshold = strtoul(argv[3], &err3, 10);
    initHandSize = strtoul(argv[4], &err4, 10);

    if (*err1 != '\0' || numberOfPlayers < 2) {
        end_game(game, INVALID_PLAYERS);
    }
    if (*err2 != '\0' || playerNumber >= numberOfPlayers || playerNumber < 0) {
        end_game(game, INVALID_POSITION);
    }
    if (*err3 != '\0' || threshold < 2) {
        end_game(game, INVALID_THRESHOLD);
    }
    if (*err4 != '\0' || initHandSize < 1) {
        end_game(game, INVALID_H_SIZE);
    }
    game->playerCount = numberOfPlayers;
    game->initHandSize = initHandSize;
    init_players(game);
    game->currentPlayer = 0;
    game->position = playerNumber;
    game->players[game->position].cardsInHand = initHandSize;
    game->threshold = threshold;

    printf("%c", '@');
    fflush(stdout);
    return NORMAL;
}

Error read_hand_message(char *input, GameState *game) {
    char *err1;
    if (strncmp(input, "HAND", 4) != 0) {
        end_game(game, INVALID_MESSAGE);
    }
    int sizeOfN = 0; // number of bytes read
    char *n = read_line_from_array(&input[4], &sizeOfN, ',');
    game->players[game->position].cardsInHand = strtoul(n, &err1, 10);
    free(n);
    if (*err1 != '\0') {
        end_game(game, INVALID_MESSAGE);
    }
    if (game->players[game->position].cardsInHand != game->initHandSize) {
        end_game(game, INVALID_MESSAGE);
    }

    for (int i = 0; i < game->players[game->position].cardsInHand; i++) {
        char *readCard;
        int size;
        readCard = read_line_from_array(&input[(4 + (sizeOfN - 1) + (i * 3))],
                                        &size, ',');

        if (string_to_card(readCard, &game->players[game->position].hand[i]) !=
            NORMAL) {
            free(readCard);
            end_game(game, INVALID_MESSAGE);
        }
        free(readCard);
    }
    game->lastMessage = HAND;

    return NORMAL;
}

void play_card(GameState *game, int choice) {
    game->lastMoves[game->position].suit =
        game->players[game->position].hand[choice].suit;
    game->lastMoves[game->position].rank =
        game->players[game->position].hand[choice].rank;

    printf("PLAY%c%c\n", game->players[game->position].hand[choice].suit,
           game->players[game->position].hand[choice].rank);
    remove_played_card(game, choice, game->position);
    fflush(stdout);
}

Error read_new_round_message(char *input, GameState *game) {
    if (strncmp(input, "NEWROUND", 8) != 0) {
        end_game(game, INVALID_MESSAGE);
    }
    int numberLength = 0;
    char *err1;
    char *leadPlayer = read_line_from_array(&input[8], &numberLength, '\n');
    if (game->leadPlayer != strtoul(leadPlayer, &err1, 10)) {
        free(leadPlayer);
        end_game(game, INVALID_MESSAGE);
    }
    free(leadPlayer);

    if (*err1 != '\0') {
        end_game(game, INVALID_MESSAGE);
    }
    game->lastMessage = NEW_ROUND;
    return NORMAL;
}

Error run_game(GameState *game, int (*chooser)(GameState *)) {
    while (game->lastMessage != GAMEOVER) {
        if (parse_input(game) != NORMAL && game->lastMessage != HAND &&
            game->lastMessage != GAMEOVER) {
            end_game(game, INVALID_MESSAGE);
        }
        // Round loop
        for (int h = 0; h < game->initHandSize; h++) {
            // New round

            if (parse_input(game) != NORMAL && game->lastMessage != NEW_ROUND &&
                game->lastMessage != GAMEOVER) {
                end_game(game, INVALID_MESSAGE);
            }

            // player loop
            for (int player = 0; player < game->playerCount; player++) {
                game->currentPlayer =
                    ((game->leadPlayer + player) % game->playerCount);
                if (game->currentPlayer == (game->position)) {
                    play_card(game, ((*chooser)(game)));
                } else if (parse_input(game) != NORMAL &&
                           game->lastMessage != PLAYED &&
                           game->lastMessage != GAMEOVER) {
                    end_game(game, INVALID_MESSAGE);
                }

                // get played
                if (player == 0) {
                    game->leadSuit = game->lastMoves[game->currentPlayer].suit;
                }
            }
            fprintf(stderr, "Lead player=%d: ", game->leadPlayer);
            print_cards(game, stderr);
            fflush(stderr);
            find_new_lead(game, game->lastMoves);
        }
    }

    return NORMAL;
}

Error read_played_message(char *input, GameState *game) {
    if (strncmp(input, "PLAYED", 6) != 0) {
        end_game(game, INVALID_MESSAGE);
    }
    int player = 0;
    if (sscanf(input, "PLAYED%d,%c%c", &player,
               &game->lastMoves[game->currentPlayer].suit,
               &game->lastMoves[game->currentPlayer].rank) != 3) {
        end_game(game, INVALID_MESSAGE);
    }
    if (player != game->currentPlayer) {
        end_game(game, INVALID_MESSAGE);
    }
    if (!is_card_valid(game->lastMoves[game->currentPlayer])) {
        end_game(game, INVALID_MESSAGE);
    }

    game->lastMessage = PLAYED;
    return NORMAL;
}

Error read_gameover_message(char *input, GameState *game) {
    if (strncmp(input, "GAMEOVER", 8) != 0) {
        end_game(game, INVALID_MESSAGE);
    }
    end_game(game, NORMAL);
    return NORMAL;
}

Error parse_input(GameState *game) {
    char inputBuffer[BUFFSIZE];
    if (!fgets(inputBuffer, BUFFSIZE, stdin)) {
        end_game(game, UNEXPECTED_EOF);
    }

    switch (inputBuffer[0]) {
        case 'H':
            return read_hand_message(inputBuffer, game);
            break;

        case 'N':
            return read_new_round_message(inputBuffer, game);
            break;

        case 'P':
            return read_played_message(inputBuffer, game);
            break;

        case 'G':
            return read_gameover_message(inputBuffer, game);
            break;

        default:
            end_game(game, INVALID_MESSAGE);
            break;
    }
    return INVALID_MESSAGE;
}
