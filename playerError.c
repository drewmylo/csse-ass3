#include "playerError.h"
#include <stdio.h>

/**
 * Returns the corresponding error message.
 *
 * Parameters:
 *      r   :   the code to get the error message for
 */
const char *error_message(Error error) {
  switch (error) {
      case NORMAL: {
        return "";
      }  // All is well
      case BAD_ARGS_NUM: {
        return "Usage: player players myid threshold handsize";
      }  // Usage: player players myid threshold handsize
      case INVALID_PLAYERS: {
        return "Invalid players";
      }
      case INVALID_POSITION: {
        return "Invalid position";
      }
      case INVALID_THRESHOLD: {
        return "Invalid threshold";
      }
      case INVALID_H_SIZE: {
        return "Invalid hand size";
      }  // Invalid hand size
      case INVALID_MESSAGE: {
        return "Invalid message";
      }
      case UNEXPECTED_EOF: {
        return "EOF";
      }
      default: { return "Something went very wrong"; }
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
