#ifndef A68K_HISTORY_H
#define A68K_HISTORY_H

void history_add(const char *line);
const char *history_get_display_number(int number);
int history_count(void);

#endif

