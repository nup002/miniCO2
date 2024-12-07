#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "cmd_system_common.h"

/*
 * We warn if a secondary serial console is enabled. A secondary serial console is always output-only and
 * hence not very useful for interactive console applications. If you encounter this warning, consider disabling
 * the secondary serial console in menuconfig unless you know what you are doing.
 */
#if SOC_USB_SERIAL_JTAG_SUPPORTED
#if !CONFIG_ESP_CONSOLE_SECONDARY_NONE
#warning "A secondary serial console is not useful when using the console component. Please disable it in menuconfig."
#endif
#endif

static const char* TAG = "console";
#define PROMPT_STR "minico2"

/** Arguments used by 'custom_command' function */
static struct {
    struct arg_int *number;
    struct arg_str *string;
    struct arg_end *end;
} custom_args;

static int custom_command(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &custom_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, custom_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(TAG, "Thank you for the command! String: %s, number: %d", custom_args.string->sval[0], custom_args.number->ival[0]);
    return 0;
}

static void register_custom(void)
{
    custom_args.number = arg_int0("n", "number", "<t>", "A command option representing an optional number");
    custom_args.string = arg_str1("s", "string", "<string>", "A command option representing a mandatory string");
    custom_args.end = arg_end(2);

    const esp_console_cmd_t custom_cmd = {
        .command = "custom",
        .help = "Send a custom command :)",
        .hint = NULL,
        .func = &custom_command,
        .argtable = &custom_args
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&custom_cmd) );
}

void console_task(void *pvParameters)
{
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    repl_config.prompt = PROMPT_STR ">";
    repl_config.max_cmdline_length = 100;

    /* Register commands */
    esp_console_register_help_command();
    register_system_common();
    register_custom();

#if defined(CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG)
    esp_console_dev_usb_serial_jtag_config_t hw_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&hw_config, &repl_config, &repl));

#else
#error Unsupported console type
#endif

    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
