#ifndef LAUNCHER_SAVE_SETTINGS_H
#define LAUNCHER_SAVE_SETTINGS_H

#include "client/opts.h"

bool save_settings(struct client_opts *opts);
bool load_settings(struct client_opts *opts);
#endif
