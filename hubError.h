
#ifndef HUB_ERROR_H
#define HUB_ERROR_H
#include <stdio.h>

typedef enum {
  NORMAL = 0,             // All is well
  BAD_ARGS_NUM = 1,       // Less than 4 commandline arguments
  INVALID_THRESHOLD = 2,  // Threshold < 2 or not a number
  BAD_DECK = 3,           // Problem reading / parsing the deck
  NEED_CARDS = 4,         // Less than P cards in the deck
  PLAYER_ERROR = 5,       // Unable to start one of the players
  PLAYER_EOF = 6,         // Unexpected EOF from a player
  INVALID_MESSAGE = 7,    // Invalid message from a player
  INVALID_CHOICE =
      8,  // Player chooses card they don’t have or don’t follow suit
  SIGHUP_RECEIVED = 9  // Received SIGHUP
} Error;

const char *error_message(Error error);
Error exit_with_error(Error error);

#endif
