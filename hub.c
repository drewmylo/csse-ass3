#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hubError.h"
#include "utils.h"
#define BUFFSIZE 512

bool sigHupOccured = false;  // global variable

void handle(int s) {  // parameter for if we handle multiple signals
  if (s == SIGHUP) {
    sigHupOccured = true;
  }

  return;
}

Error parse_deckfile(GameState *game) {
  if (!game->deck->deck) {
    return BAD_DECK;
  }
  if (fscanf(game->deck->deck, "%d\n", &game->deck->deckSize) == 0) {
    return BAD_DECK;
  }
  if (game->deck->deckSize < game->playerCount) {
    return NEED_CARDS;
  }
  game->deck->parsedDeck = calloc(game->deck->deckSize, sizeof(Card));
  for (int i = 0; i < game->deck->deckSize; i++) {
    game->deck->parsedDeck[i].suit = fgetc(game->deck->deck);
    game->deck->parsedDeck[i].rank = fgetc(game->deck->deck);
    if (is_card_valid(game->deck->parsedDeck[i]) == false) {
      return BAD_DECK;
    }
    if (i != (game->deck->deckSize) - 1) {
      char newline = fgetc(game->deck->deck);
      if (newline != '\n') {
        return BAD_DECK;
      }
    }
  }
  fclose(game->deck->deck);
  return NORMAL;
}

Error argument_checks(int argc, char const *argv[], GameState *game) {
  unsigned int threshold;
  char *err1;

  // Check for correct number of arguments
  if (argc <= 3) {
    exit(exit_with_error(BAD_ARGS_NUM));
  }

  threshold = strtoul(argv[2], &err1, 10);

  if (*err1 != '\0' || threshold < 2) {
    exit(exit_with_error(INVALID_THRESHOLD));
  }
  game->threshold = threshold;
  return NORMAL;
}

Error send_hand(FILE *writeTo, int number, Card *cards) {
  fprintf(writeTo, "HAND%d", number);
  for (int i = 0; i < number; i++) {
    fputc(',', writeTo);
    fputc(cards[i].suit, writeTo);
    fputc(cards[i].rank, writeTo);
  }
  fputc('\n', writeTo);

  fflush(writeTo);
  return NORMAL;
}

Error start_new_round(GameState *game) {
  printf("Lead player=%d\n", game->leadPlayer);
  fflush(stdout);
  for (int i = 0; i < game->playerCount; i++) {
    if (fprintf(game->players[i].writeTo, "NEWROUND%d\n", game->leadPlayer) <
        1) {
      return PLAYER_ERROR;
    }
    fflush(game->players[i].writeTo);
  }

  return NORMAL;
}
Error send_played(GameState *game, int player, Card card) {
  game->lastMoves[player].rank = card.rank;
  game->lastMoves[player].suit = card.suit;

  for (int i = 0; i < game->playerCount; i++) {
    if (i != player) {
      if (fprintf(game->players[i].writeTo, "PLAYED%d,%c%c\n", player,
                  card.suit, card.rank) < 1) {
        return PLAYER_ERROR;
      }
      fflush(game->players[i].writeTo);
    }
  }
  return NORMAL;
}

void send_gameover_and_exit(GameState *game, Error exit_status) {
  for (int i = 0; i < game->playerCount; i++) {
    if (fprintf(game->players[i].writeTo, "GAMEOVER\n") < 1) {
      kill(game->players[i].pid, SIGTERM);
    }
    fflush(game->players[i].writeTo);
  }
  sleep(0.2);
  for (int i = 0; i < game->playerCount; i++) {
    kill(game->players[i].pid, SIGTERM);
  }

  sleep(0.2);
  for (int i = 0; i < game->playerCount; i++) {
    kill(game->players[i].pid, SIGKILL);
  }
  free_mem(game);

  exit(exit_with_error(exit_status));
}

Error parse_input(FILE *readFrom, Card *card) {
  // eof_check(game);
  char inputBuffer[BUFFSIZE];
  if (!(fgets(inputBuffer, BUFFSIZE, readFrom))) {
    return PLAYER_EOF;
  }
  if (strncmp(inputBuffer, "PLAY", 4) != 0) {
    return INVALID_MESSAGE;
  }
  card->suit = inputBuffer[4];
  card->rank = inputBuffer[5];

  return NORMAL;
}

void init_GameState(GameState *game) { game->deck = calloc(1, sizeof(Deck)); }

int main(int argc, char const *argv[]) {
  struct sigaction sigpipe_action;
  sigpipe_action.sa_handler = handle;
  sigpipe_action.sa_flags = SA_RESTART;
  memset(&sigpipe_action, 0, sizeof(struct sigaction));
  sigaction(SIGPIPE, &sigpipe_action, 0);

  struct sigaction sighup_action;
  sighup_action.sa_handler = handle;
  sighup_action.sa_flags = SA_RESTART;
  memset(&sighup_action, 0, sizeof(struct sigaction));
  sigaction(SIGHUP, &sighup_action, 0);

  GameState *game = calloc(1, sizeof(GameState));
  game->position = -1;
  game->playerCount = (argc - 3);
  init_GameState(game);
  game->deck->deck = fopen(argv[1], "r");

  argument_checks(argc, argv, game);
  if (parse_deckfile(game) != NORMAL) {
    return exit_with_error(BAD_DECK);
  }

  game->initHandSize = (game->deck->deckSize / game->playerCount);
  game->leadPlayer = 0;
  init_players(game);

  for (int i = 0; i < (game->playerCount); i++) {
    if (pipe(game->players[i].fromPlayer) != 0) {
      return exit_with_error(PLAYER_ERROR);
    }
    if (pipe(game->players[i].toPlayer) != 0) {
      return exit_with_error(PLAYER_ERROR);
    }
    game->players[i].pid = fork();
    if (game->players[i].pid == 0) {
      /* Is child */
      freopen("/dev/null", "w", stderr);
      close(game->players[i].fromPlayer[0]);
      close(game->players[i].toPlayer[1]);
      dup2(game->players[i].fromPlayer[1], 1);
      dup2(game->players[i].toPlayer[0], 0);

      char playerCount[10] = {0};
      char playerNum[10] = {0};
      // char threshold[30] = {0};
      char initHandSize[10] = {0};

      sprintf(playerCount, "%d", game->playerCount);
      sprintf(playerNum, "%d", i);
      // sprintf(threshold, "%d", argv[2]);
      sprintf(initHandSize, "%d", game->initHandSize);

      if (execlp(argv[3 + i], argv[3 + i], playerCount, playerNum, argv[2],
                initHandSize, (char *)NULL) == -1) {
        return exit_with_error(PLAYER_ERROR);
      }
    } else {
      /*Is parent*/
      close(game->players[i].fromPlayer[1]);
      close(game->players[i].toPlayer[0]);
      game->players[i].readFrom = fdopen(game->players[i].fromPlayer[0], "r");
      game->players[i].writeTo = fdopen(game->players[i].toPlayer[1], "w");
      if (fgetc(game->players[i].readFrom) != '@') {
        return exit_with_error(PLAYER_ERROR);
      }
    }
  }
  if (sigHupOccured == true) {
    send_gameover_and_exit(game, SIGHUP_RECEIVED);
  }

  // Hand out cards
  for (int i = 0; i < game->playerCount; i++) {
    memcpy(game->players[i].hand,
           &(game->deck->parsedDeck[(i * game->initHandSize)]),
           (sizeof(Card) * game->initHandSize));
    send_hand(game->players[i].writeTo, game->initHandSize,
              game->players[i].hand);
  }
  // Round loop
  for (int h = 0; h < game->initHandSize; h++) {
    if (sigHupOccured == true) {
      send_gameover_and_exit(game, SIGHUP_RECEIVED);
    }

    if (start_new_round(game) != NORMAL) {
      return exit_with_error(PLAYER_ERROR);
    }
    // player loop
    for (int relativePlayer = 0; relativePlayer < game->playerCount;
         relativePlayer++) {
      if (sigHupOccured == true) {
        send_gameover_and_exit(game, SIGHUP_RECEIVED);
      }
      int player = ((game->leadPlayer + relativePlayer) % game->playerCount);

      Error code =
          parse_input(game->players[player].readFrom, &game->lastMoves[player]);
      if (find_card(game, game->lastMoves[player], player) == -1) {
        send_gameover_and_exit(game, INVALID_MESSAGE);
      }
      if (game->lastMoves[player].suit != game->leadSuit) {
        if (suit_in_hand(game->players[player].hand,
                         game->players[player].cardsInHand, game->leadSuit)) {
          send_gameover_and_exit(game, INVALID_CHOICE);
        }
      }

      if (code == INVALID_MESSAGE)  // todo addd wrong elad here or wrong card
      {
        send_gameover_and_exit(game, INVALID_MESSAGE);
      }
      if (code == PLAYER_EOF) {
        send_gameover_and_exit(game, PLAYER_EOF);
      }

      fflush(stdout);
      if (relativePlayer == 0) {
        game->leadSuit = game->lastMoves[player].suit;
      }

      if (send_played(game, player, game->lastMoves[player]) != NORMAL) {
        return exit_with_error(PLAYER_ERROR);
      }
      remove_played_card(game, find_card(game, game->lastMoves[player], player),
                         player);
    }
    printf("Cards=");
    print_cards(game, stdout);
    fflush(stdout);
    find_new_lead(game, game->lastMoves);
  }
  add_points(game);
  for (int i = 0; i < game->playerCount; i++) {
    if (i == game->playerCount - 1) {
      printf("%d:%d", i, game->players[i].points);
    } else {
      printf("%d:%d ", i, game->players[i].points);
    }
  }
  putchar('\n');
  fflush(stdout);
  send_gameover_and_exit(game, NORMAL);

  /* code */
  while (1) {
  }
  return 0;
}
