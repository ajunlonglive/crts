#ifndef CLIENT_INPUT_CFG_H
#define CLIENT_INPUT_CFG_H

#define KEYMAP_CFG "keymap.ini"

#include <stdbool.h>
#include <stdint.h>

#include "client/ui/common.h"

bool input_cfg_parse(void);
#endif
