#ifndef UTILS_H
#define UTILS_H
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
  char suit;
  char rank;
} Card;

typedef enum {
  HAND = 1,       // All is well
  NEW_ROUND = 2,  // Usage: player players myid threshold handsize
  PLAYED = 3,
  GAMEOVER = 4,
} Message;

typedef struct {
  int drawnCards;
  char *deckName;
  FILE *deck;
  Card *parsedDeck;
  int deckSize;
} Deck;

typedef struct {
  int toPlayer[2];
  int fromPlayer[2];
  FILE *readFrom;
  FILE *writeTo;
  pid_t pid;
  Card *hand;
  int cardsInHand;
  int points;
  int V;

} Player;

typedef struct {
  int playerCount;
  int currentPlayer;
  int leadPlayer;
  char leadSuit;
  int threshold;
  int initHandSize;
  int position;
  Card *lastMoves;
  Deck *deck;
  Player *players;
  Message lastMessage;

} GameState;

void free_mem(GameState *game);
int find_card(GameState *game, Card card, int player);
void remove_played_card(GameState *game, int choice, int player);
bool is_card_valid(Card card);
void init_players(GameState *game);
char *read_line_from_array(char *string, int *size, char delimeter);
char *read_line_from_file(FILE *file, int *size, char delimeter);
bool suit_in_hand(Card *hand, int count, char suit);
int highest_card(Card *hand, int count, char suit);
int lowest_card(Card *hand, int count, char suit);
int number_of_D_cards(Card *hand, int count);
void find_new_lead(GameState *game, Card *cards);
void add_points(GameState *game);
bool close_to_winning(GameState *game);
void print_cards(GameState *game, FILE *direction);
#endif