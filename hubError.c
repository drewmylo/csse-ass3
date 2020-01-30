#include "hubError.h"
#include <stdio.h>

/**
 * Returns the corresponding error message.
 *
 * Parameters:
 *      r   :   the code to get the error message for
 */
const char *error_message(Error error) {
  switch (error) {
    case NORMAL:
      return "";  // All is well
    case BAD_ARGS_NUM:
      return "Usage: 2310hub deck threshold player0 {player1}";  // Less than 4
                                                                 // commandline
                                                                 // arguments
    case INVALID_THRESHOLD:
      return "Invalid threshold";  // Threshold < 2 or not a number
    case BAD_DECK:
      return "Deck error";  // Problem reading / parsing the deck
    case NEED_CARDS:
      return "Not enough cards";  // Less than P cards in the deck
    case PLAYER_ERROR:
      return "Player error";  // Unable to start one of the players
    case PLAYER_EOF:
      return "Player EOF";  // Unexpected EOF from a player
    case INVALID_MESSAGE:
      return "Invalid message";  // Invalid message from a player
    case INVALID_CHOICE:
      return "Invalid card choice";  // Player chooses card they don’t have or
                                     // don’t follow suit
    case SIGHUP_RECEIVED:
      return "Ended due to signal";  // Received SIGHUP
    default:
      return "Something went very wrong";
  }
}

/**
 * Prints the correct error string to stderr and returns error code.
 * Parameters:
 *        error Error code to print and return.
 */
Error exit_with_error(Error error) {
  if (error != NORMAL) {
    fprintf(stderr, "%s\n", error_message(error));
  }
  return error;
}
