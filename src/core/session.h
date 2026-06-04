#ifndef A68K_SESSION_H
#define A68K_SESSION_H

int session_change_dir(const char *path);
const char *session_current_path(void);
const char *session_current_leaf(void);
void session_cleanup(void);

#endif
