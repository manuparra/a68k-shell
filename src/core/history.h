#ifndef A68K_HISTORY_H
#define A68K_HISTORY_H

void history_add(const char *line);
const char *history_get(int index);
int history_count(void);

#endif

