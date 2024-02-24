// Copyright 2024 PWrInSpace, Kuba

#ifndef _CLI_TASK_H_
#define _CLI_TASK_H_

#include <stdbool.h>
#include <stdint.h>

#define CLI_PROMPT "dupa xD: "
#define CLI_TASK_STACK_DEPTH 4096
#define CLI_TASK_CPU_NUM PRO_CPU_NUM
#define CLI_TASK_PRIORITY 0

// USE MACROS ONLY IN CLI COMMANDS OR CLI ENVIROMENT
// Print ok message
#define CLI_WRITE_G(format, ...) printf("OK: "format"\n", ##__VA_ARGS__)
// Print error message to default stream
#define CLI_WRITE_E(format, ...) printf("ERR: "format"\n", ##__VA_ARGS__)
// Print to default stream
#define CLI_WRITE(format, ...) printf(format"\n", ##__VA_ARGS__)

bool cli_init(uint8_t command_max_len);

bool cli_run(void);

bool cli_deinit(void);

#endif
