// Copyright 2024 PWrInSpace, Kuba

// Code based on example from esp-idf documentation
// https://github.com/espressif/esp-idf/blob/v5.2/examples/system/console/advanced/main/console_example_main.c

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_console.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "esp_vfs_dev.h"
#include "cli_task.h"
#include "esp_log.h"


#define TAG "CLI"

static struct {
    TaskHandle_t task_handle;
} gb;


bool _configure_default_uart(void) {
    // Drain stdout before reconfiguring it
    fflush(stdout);
    fsync(fileno(stdout));

    // Disable buffering on stdin
    setvbuf(stdin, NULL, _IONBF, 0);

    // Minicom, screen, idf_monitor send CR when ENTER key is pressed
    esp_vfs_dev_uart_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);
    // Move the caret to the beginning of the next line on '\n'
    esp_vfs_dev_uart_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);

    // Install UART driver for interrupt-driven reads and writes
    const uart_config_t uart_config = {
            .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .source_clk = UART_SCLK_REF_TICK,
    };
    esp_err_t err = uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0);
    if (err != ESP_OK) {
        return false;
    }

    err = uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config);
    if (err != ESP_OK) {
        uart_driver_delete(CONFIG_ESP_CONSOLE_UART_NUM);
        return false;
    }

    // Tell VFS to use UART driver
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    return true;
}

void _configure_linenoise(uint8_t command_max_len) {
    // Configure linenoise line completion library
    // Enable multiline editing.
    linenoiseSetMultiLine(1);

    // Tell linenoise where to get command completions and hints
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    // Set command history size
    linenoiseHistorySetMaxLen(100);

    // Set command maximum length
    linenoiseSetMaxLineLen(command_max_len);

    // Don't return empty lines
    linenoiseAllowEmpty(true);
}

bool cli_init(uint8_t command_max_len) {
    if (_configure_default_uart() == false) {
        return false;
    }

    _configure_linenoise(command_max_len);

    return true;
}

static void _print_welcome_message(void) {
    CLI_WRITE("\n"
              "Type 'help' to get the list of commands.\n"
              "Use UP/DOWN arrows to navigate through command history.\n"
              "Press TAB when typing command name to auto-complete.\n");
}

static void _check_escape_sequences(void) {
    // Figure out if the terminal supports escape sequences
    int probe_status = linenoiseProbe();
    if (probe_status != 0) {  // zero indicates success
        linenoiseSetDumbMode(1);
        CLI_WRITE("\nYour terminal application does not support escape sequences.\n"
                  "Line editing and history features are disabled.\n"
                  "On Windows, try using Putty instead.\n");
    }
}

static void _parse_line(char* line) {
    int return_code;

    esp_err_t err = esp_console_run(line, &return_code);
    if (err == ESP_ERR_NOT_FOUND || err == ESP_ERR_INVALID_ARG) {
        CLI_WRITE_E("Unrecognized command or command was empty\n");
    } else if (err != ESP_OK) {
        CLI_WRITE_E("Internal error: %s\n", esp_err_to_name(err));
    } else if (err == ESP_OK && return_code != ESP_OK) {
        CLI_WRITE_E("Command returned non-zero error code: 0x%x (%s)\n",
                    return_code, esp_err_to_name(return_code));
    } else if (err == ESP_ERR_INVALID_STATE) {
        CLI_WRITE_E("esp_console_init wasn't called - aborting cli task");
        cli_deinit();
    }
}

static void _add_line_to_history(char *line) {
    if (strlen(line) > 0) {
        linenoiseHistoryAdd(line);
    }
}

static void _cli_task(void *arg) {
    char* line;

    _print_welcome_message();
    _check_escape_sequences();

    while (true) {
        // The line is returned when ENTER is pressed, so this functions blocks the task
        line = linenoise(CLI_PROMPT);
        if (line != NULL) {
            _add_line_to_history(line);
            _parse_line(line);
        } else {
            CLI_WRITE_E("Error during line reading");
        }

        linenoiseFree(line);  // allocates line on the heap
    }
}

bool cli_run(void) {
    if (gb.task_handle != NULL) {
        ESP_LOGE(TAG, "Task is running");
        return false;
    }

    if (esp_console_register_help_command()!= ESP_OK) {
        ESP_LOGE(TAG, "esp_console_init wasn't called");
        return false;
    }

    xTaskCreatePinnedToCore(
        _cli_task, "CLItask", CLI_TASK_STACK_DEPTH, NULL,
        CLI_TASK_PRIORITY, &gb.task_handle, CLI_TASK_CPU_NUM);

    if (gb.task_handle == NULL) {
        return false;
    }

    return true;
}

bool cli_deinit(void) {
    esp_err_t err = uart_driver_delete(CONFIG_ESP_CONSOLE_UART_NUM);
    if (err != ESP_OK) {
        return false;
    }

    if (gb.task_handle == NULL) {
        return false;
    }

    vTaskDelete(gb.task_handle);
    gb.task_handle = NULL;

    return true;
}
