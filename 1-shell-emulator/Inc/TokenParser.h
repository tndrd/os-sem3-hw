#pragma once

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ShellStatus.h"

#define INITIAL_TOKEN_ARRAY_LENGTH 10

typedef struct {
  char** Tokens;  // No const because exec asks for [char* const*] as argv
                  // argument
  size_t Capacity;
} TokenParser;

ShellStatus TokenParserInit(TokenParser* tp);

// Creates token array from string (Modifies it!)
ShellStatus ParseTokens(TokenParser* tp, char* string, const char* delim);

ShellStatus TokenParserDestroy(TokenParser* tp);