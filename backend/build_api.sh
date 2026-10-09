#!/bin/sh
set -eu
gcc -std=c11 -Wall -Wextra -Wpedantic -o mil-log-api \
  api_server.c api_globals.c units.c inventory.c requests.c storage.c filesystem.c input.c
