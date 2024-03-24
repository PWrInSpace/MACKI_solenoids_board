// Copyright 2024 PWrInSpace, Krzys

#include "cmd.h"

static QueueHandle_t queue;
static char new_message[256] = {0};

QueueHandle_t* cmd_ble_queue_init(void) {
    queue = xQueueCreate(10, sizeof(char) * 256);
    return &queue;
}

inline QueueHandle_t* cmd_get_queue(void) { return &queue; }

void cmd_queue_send(const char* format, ...) {
    va_list args;

    va_start(args, format);
    vsnprintf(new_message, 256, format, args);
    va_end(args);

    xQueueSend(queue, new_message, 0);
}
