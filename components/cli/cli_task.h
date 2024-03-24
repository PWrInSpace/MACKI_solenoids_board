// Copyright 2024 PWrInSpace, Kuba

#ifndef _CLI_TASK_H_
#define _CLI_TASK_H_

#include <stdbool.h>
#include <stdint.h>
#include "esp_console.h"
#include "cmd.h"


#define CLI_PROMPT CONFIG_CLI_PROMPT
#define CLI_TASK_STACK_DEPTH CONFIG_CLI_TASK_STACK_DEPTH
#define CLI_TASK_PRIORITY CONFIG_CLI_TASK_PRIORITY
#define CLI_TASK_CPU_NUM CONFIG_CLI_TASK_CPU

bool cli_init(uint8_t command_max_len);

bool cli_run(void);

bool cli_deinit(void);

#endif
