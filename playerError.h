
#ifndef PLAYER_ERROR_H
#define PLAYER_ERROR_H
#include <stdio.h>

typedef enum {
  NORMAL = 0,        // All is well
  BAD_ARGS_NUM = 1,  // Usage: player players myid threshold handsize
  INVALID_PLAYERS = 2,
  INVALID_POSITION = 3,
  INVALID_THRESHOLD = 4,
  INVALID_H_SIZE = 5,  // Invalid hand size
  INVALID_MESSAGE = 6,
  UNEXPECTED_EOF = 7,
  NO_INPUT = 8,

} Error;

const char *error_message(Error error);
Error exit_with_error(Error error);

#endif
