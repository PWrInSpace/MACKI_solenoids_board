// Copyright 2024 PWrInSpace, Kuba

#ifndef _CMD_H_
#define _CMD_H_

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "stdarg.h"

#define CMD_RESPONSE_UART 1
#define CMD_RESPONSE_BLE 1

/// \brief Initiated by BLE task, only initializes the queue
QueueHandle_t* cmd_ble_queue_init(void);

QueueHandle_t* cmd_get_queue(void);

void cmd_queue_send(const char* format, ...);

bool cmd_register_common(void);

bool cmd_register_valves(void);

bool cmd_register_load_cell(void);

#ifdef CMD_RESPONSE_UART
#define CLI_UART_WRITE_G(format, ...) printf("OK: " format "\n", ##__VA_ARGS__)
#define CLI_UART_WRITE_E(format, ...) printf("ERR: " format "\n", ##__VA_ARGS__)
#define CLI_UART_WRITE(format, ...) printf(format "\n", ##__VA_ARGS__)
#else
#define CLI_UART_WRITE_G
#define CLI_UART_WRITE_E
#define CLI_UART_WRITE
#endif

#ifdef CMD_RESPONSE_BLE
#define CLI_BLE_WRITE_G(format, ...) cmd_queue_send("OK: " format "" , ##__VA_ARGS__)
#define CLI_BLE_WRITE_E(format, ...) cmd_queue_send("ERR: " format "" , ##__VA_ARGS__)
#define CLI_BLE_WRITE(format, ...) cmd_queue_send(format "" , ##__VA_ARGS__)
#else
#define CLI_BLE_WRITE_G
#define CLI_BLE_WRITE_E
#define CLI_BLE_WRITE
#endif

#define CLI_WRITE_G(format, ...) CLI_UART_WRITE_G(format, ##__VA_ARGS__); CLI_BLE_WRITE_G(format, ##__VA_ARGS__)
// Print error message to default stream
#define CLI_WRITE_E(format, ...) CLI_UART_WRITE_E(format, ##__VA_ARGS__); CLI_BLE_WRITE_E(format, ##__VA_ARGS__)
// Print to default stream
#define CLI_WRITE(format, ...) CLI_UART_WRITE(format, ##__VA_ARGS__); CLI_BLE_WRITE(format, ##__VA_ARGS__)

#endif
