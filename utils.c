#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"

void free_mem(GameState *game) {
  for (int i = 0; i < game->playerCount; i++) {
    free(game->players[i].hand);
  }

  free(game->lastMoves);
  if (game->position == -1) {
    for (int i = 0; i < game->playerCount; i++) {
      fclose(game->players[i].readFrom);
      fclose(game->players[i].writeTo);
    }
    free(game->deck->parsedDeck);
    free(game->deck);
  }
  free(game->players);

  free(game);
}
bool is_card_valid(Card card) {
  if (card.suit != 'S' && card.suit != 'C' && card.suit != 'D' &&
      card.suit != 'H') {
    return false;
  }
  if (card.rank < '1' && card.rank > '9' && card.rank < 'a' &&
      card.rank > 'f') {
    return false;
  }
  return true;
}

void remove_played_card(GameState *game, int choice, int player) {
  for (int i = choice; i < (game->players[player].cardsInHand - 1); i++) {
    game->players[player].hand[i].rank = game->players[player].hand[i + 1].rank;
    game->players[player].hand[i].suit = game->players[player].hand[i + 1].suit;
  }
  game->players[player].cardsInHand--;
}

int find_card(GameState *game, Card card, int player) {
  for (int i = 0; i < (game->players[player].cardsInHand); i++) {
    if (game->players[player].hand[i].rank == card.rank &&
        game->players[player].hand[i].suit == card.suit) {
      return i;
    }
  }
  return -1;
}
/**
 * Reads a line and creates and returns null terminated string.
 * Parameters:
 *       string The array to read a line from.
 *       size Size of he resulting string.
 *
 */
char *read_line_from_array(char *string, int *size, char delimeter) {
  char readCharacter = 'a';
  (*size) = 0;
  char *line = malloc(sizeof(char));
  while (readCharacter != '\n' && readCharacter != '\0' &&
         readCharacter != EOF && readCharacter != delimeter) {
    (*size)++;
    line = realloc(line, sizeof(char) * ((*size)));
    readCharacter = string[(*size) - 1];
    line[(*size) - 1] = readCharacter;
  }
  (*size)++;
  line[(*size) - 2] = '\0';
  return line;
}

char *read_line_from_file(FILE *file, int *size, char delimeter) {
  char readCharacter = 'a';
  (*size) = 0;
  char *line = malloc(sizeof(char));
  while (readCharacter != '\n' && readCharacter != '\0' &&
         readCharacter != EOF && readCharacter != delimeter) {
    (*size)++;
    line = realloc(line, sizeof(char) * ((*size)));
    readCharacter = fgetc(file);
    line[(*size) - 1] = readCharacter;
  }
  (*size)++;
  line[(*size) - 2] = '\0';
  if (readCharacter == EOF) {
    (*size) = EOF;
  }
  return line;
}

void print_cards(GameState *game, FILE *direction) {
  for (int x = game->leadPlayer; x < (game->playerCount + game->leadPlayer);
       x++) {
    int i = (x % game->playerCount);
    fputc(game->lastMoves[i].suit, direction);
    fputc('.', direction);
    fputc(game->lastMoves[i].rank, direction);
    if (x != (game->playerCount + game->leadPlayer - 1)) {
      fputc(' ', direction);
    }
  }
  fputc('\n', direction);
}

char get_suit(int suitNum) {
  if ((suitNum % 4) == 0) {
    return 'S';
  }
  if ((suitNum % 4) == 1) {
    return 'C';
  }
  if ((suitNum % 4) == 2) {
    return 'D';
  }
  if ((suitNum % 4) == 3) {
    return 'H';
  }
  return 'Z';
}

bool suit_in_hand(Card *hand, int count, char suit) {
  for (int i = 0; i < count; i++) {
    if (hand[i].suit == suit) {
      return true;
    }
  }
  return false;
}

int highest_card(Card *hand, int count, char suit) {
  char currentHighest = '\0';
  int position = 0;

  for (int i = 0; i < count; i++) {
    if (hand[i].suit == suit) {
      if (hand[i].rank > currentHighest) {
        position = i;
        currentHighest = hand[i].rank;
      }
    }
  }

  return position;
}
void add_points(GameState *game) {
  for (int i = 0; i < game->playerCount; i++) {
    if (game->players[i].V >= game->threshold) {
      game->players[i].points += game->players[i].V;
    } else {
      game->players[i].points -= game->players[i].V;
    }
  }
}
void find_new_lead(GameState *game, Card *cards) {
  int newLead = ((highest_card(cards, game->playerCount, game->leadSuit)));
  game->leadPlayer = newLead;
  game->players[newLead].points++;
  game->players[newLead].V += number_of_D_cards(cards, game->playerCount);
}

void init_players(GameState *game) {
  game->lastMoves = calloc(game->playerCount, sizeof(Card));
  game->players = calloc(game->playerCount, sizeof(Player));
  for (int i = 0; i < game->playerCount; i++) {
    game->players[i].hand = calloc(game->initHandSize, sizeof(Card));
    game->players[i].cardsInHand = game->initHandSize;
  }
}

bool close_to_winning(GameState *game) {
  for (int i = 0; i < game->playerCount; i++) {
    if (game->players[i].V >= (game->threshold - 2)) {
      return true;
    }
  }
  return false;
}

int lowest_card(Card *hand, int count, char suit) {
  char currentLowest = 127;
  int position = 0;

  for (int i = 0; i < count; i++) {
    if (hand[i].suit == suit) {
      if (hand[i].rank < currentLowest) {
        position = i;
        currentLowest = hand[i].rank;
      }
    }
  }
  return position;
}

int number_of_D_cards(Card *hand, int count) {
  int number = 0;

  for (int i = 0; i < count; i++) {
    if (hand[i].suit == 'D') {
      number++;
    }
  }
  return number;
}
