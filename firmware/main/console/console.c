#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "cmd_system_common.h"
#include "../types.h"
#include "../globals.h"
#include "../config/config.h"

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

static const char *CONSOLE_TAG = "MINICO2";
#define PROMPT_STR "minico2"

static int console_toggle_print_readings(int argc, char **argv)
{
    set_print_sensor_readings(!MINICO2CONFIG.serial_print_enabled);
    return 0;
}

static char help_str_toggle_print_readings [256];
static void register_toggle_print_readings(void){
    const char* default_state = MINICO2CONFIG_DEFAULT.serial_print_enabled ? "ENABLED" : "DISABLED";
    snprintf(help_str_toggle_print_readings, sizeof(help_str_toggle_print_readings), 
    "Enable/disable printing of sensor measurements on the serial port. Default: %s", 
    default_state);

    const esp_console_cmd_t toggle_print_readings_cmd = {
        .command = "toggle_print",
        .help = help_str_toggle_print_readings,
        .hint = NULL,
        .func = &console_toggle_print_readings
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&toggle_print_readings_cmd) );
}


/** Arguments used by 'console_set_led_brightness' function */
static struct {
    struct arg_int *brightness;
    struct arg_end *end;
} set_led_brightness_args;

static int console_set_led_brightness(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &set_led_brightness_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, set_led_brightness_args.end, argv[0]);
        return 1;
    }
    set_led_brightness((float)set_led_brightness_args.brightness->ival[0]/100);
    return 0;
}

static char help_str_set_led_brightness [256];
static void register_set_led_brightness(void){
    set_led_brightness_args.brightness = arg_int1(NULL, NULL, "<brightness>", "LED brightness in percent");
    set_led_brightness_args.end = arg_end(1);
    snprintf(help_str_set_led_brightness, sizeof(help_str_set_led_brightness), 
    "Set the brightness of the LED from 0 (off) to 100 (maximum). Default: %d", 
    (uint16_t)(MINICO2CONFIG_DEFAULT.led_cfg.brightness * 100));

    const esp_console_cmd_t set_led_brightness_cmd = {
        .command = "set_led_brightness",
        .help = help_str_set_led_brightness,
        .hint = NULL,
        .func = &console_set_led_brightness,
        .argtable = &set_led_brightness_args
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&set_led_brightness_cmd) );
}


/** Arguments used by 'set_nickname' function */
static struct {
    struct arg_str *nickname;
    struct arg_end *end;
} set_nickname_args;

static int console_set_nickname(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &set_nickname_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, set_nickname_args.end, argv[0]);
        return 1;
    }
    set_nickname(set_nickname_args.nickname->sval[0]);
    return 0;
}

static char help_str_set_nickname [256];
static void register_set_nickname(void){
    set_nickname_args.nickname = arg_str1(NULL, NULL, "<nickname>", "The nickname to assign to the MiniCO2");
    set_nickname_args.end = arg_end(1);
    snprintf(help_str_set_nickname, sizeof(help_str_set_nickname), 
    "Set the nickname of this minico2. This is the name it will appear with in all applications. Default: '%s'", 
    MINICO2CONFIG_DEFAULT.name);

    const esp_console_cmd_t set_nickname_cmd = {
        .command = "set_nickname",
        .help = help_str_set_nickname,
        .hint = NULL,
        .func = &console_set_nickname,
        .argtable = &set_nickname_args
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&set_nickname_cmd) );
}

/** Arguments used by 'set_period' function */
static struct {
    struct arg_int *period;
    struct arg_end *end;
} set_period_args;

static int console_set_period(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &set_period_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, set_period_args.end, argv[0]);
        return 1;
    }
    set_measurement_period(set_period_args.period->ival[0]);
    return 0;
}

static char help_str_set_period [256];
static void register_set_period(void){
    set_period_args.period = arg_int1(NULL, NULL, "<period>", "Measurement period in seconds");
    set_period_args.end = arg_end(1);
    snprintf(help_str_set_period, sizeof(help_str_set_period), 
    "Set the measurement period. This is the time in seconds between each CO2 measurement. \\The lowest possible value is 5 seconds. Default: %d seconds", 
    MINICO2CONFIG_DEFAULT.measurement_period);

    const esp_console_cmd_t set_period_cmd = {
        .command = "set_period",
        .help = help_str_set_period,
        .hint = NULL,
        .func = &console_set_period,
        .argtable = &set_period_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&set_period_cmd) );
}


/** Arguments used by 'console_config' function */
static struct {
    struct arg_str *option;
    struct arg_end *end;
} config_args;

static int console_config(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &config_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, config_args.end, argv[0]);
        return 1;
    }
    if (config_args.option->count == 1){
        const char *option = config_args.option->sval[0];
        if (option != NULL && strcmp(option, "-reset") == 0){
            reset_config();
        } else {
            printf("Invalid config option '%s'", option);
            return 1;
        }
    } else {
        log_config(&MINICO2CONFIG);
    }
    return 0;
}

static void register_config(void){
    config_args.option = arg_str0(NULL, NULL, "-reset", "Reset the configuration to default values");
    config_args.end = arg_end(1);

    const esp_console_cmd_t console_config_cmd = {
        .command = "config",
        .help = "Print the current configuration as an INFO log statement or reset to the default configuration",
        .hint = NULL,
        .func = &console_config,
        .argtable = &config_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&console_config_cmd) );
}

/** Arguments used by 'console_set_led_co2_limits' function */
static struct {
    struct arg_str *lim_type;
    struct arg_int *lim_val;
    struct arg_end *end;
} set_led_co2_limits_args;

static int console_set_led_co2_limits(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &set_led_co2_limits_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, set_led_co2_limits_args.end, argv[0]);
        return 1;
    }
    uint16_t medium_limit = MINICO2CONFIG.led_cfg.limit_medium;
    uint16_t high_limit = MINICO2CONFIG.led_cfg.limit_high;
    uint16_t critical_limit = MINICO2CONFIG.led_cfg.limit_critical;

    const int lim_val = set_led_co2_limits_args.lim_val->ival[0];
    const char *lim_type = set_led_co2_limits_args.lim_type->sval[0];
    if (lim_type != NULL && strcmp(lim_type, "medium") == 0){
        medium_limit = lim_val;
    } else if (lim_type != NULL && strcmp(lim_type, "high") == 0){
        high_limit = lim_val;
    } else if (lim_type != NULL && strcmp(lim_type, "critical") == 0){
        critical_limit = lim_val;
    } else {
        printf("Invalid co2 limit name '%s'. Choose from [medium|high|critical]", lim_type);
        return 1;
    }
    set_led_co2_limits(medium_limit, high_limit, critical_limit);
    return 0;
}

static void register_set_led_co2_limits(void){
    set_led_co2_limits_args.lim_type = arg_str1(NULL, NULL, "<medium|high|critical>", "The limit to set");
    set_led_co2_limits_args.lim_val = arg_int1(NULL, NULL, "<limit>", "The CO2 limit in PPM");
    set_led_co2_limits_args.end = arg_end(2);

    const esp_console_cmd_t set_led_co2_limits_cmd = {
        .command = "set_co2_limit",
        .help = "Set the CO2 limits in PPM. The LED will change color depending on these limits.",
        .hint = NULL,
        .func = &console_set_led_co2_limits,
        .argtable = &set_led_co2_limits_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&set_led_co2_limits_cmd) );
}


void start_console(void)
{
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();

    repl_config.task_priority = 10;
    repl_config.task_stack_size = 8096; // Extra large stack size for printing the config string
    repl_config.prompt = PROMPT_STR ">";
    repl_config.max_cmdline_length = 100;

    /* Register commands */
    esp_console_register_help_command();
    register_toggle_print_readings();
    register_set_led_brightness();
    register_set_led_co2_limits();
    register_set_nickname();
    register_set_period();
    register_system_common();
    register_config();

#if defined(CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG)
    esp_console_dev_usb_serial_jtag_config_t hw_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&hw_config, &repl_config, &repl));

#else
#error Unsupported console type
#endif

    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
