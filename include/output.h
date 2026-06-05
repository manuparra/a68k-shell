#ifndef A68K_OUTPUT_H
#define A68K_OUTPUT_H

#include <stdio.h>

void output_set(FILE *file);
FILE *output_get(void);
int output_printf(const char *format, ...);
int output_puts(const char *text);
int output_putchar(int ch);
int output_fputs(const char *text);
int output_write(const char *buffer, int size);

#endif

