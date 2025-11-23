#ifndef PARSER_H
#define PARSER_H
#include "process.h"
int parse_config_file(const char *path, struct process **out, int *out_n);
#endif