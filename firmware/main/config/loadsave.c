/* Functions for the saving and loading of the minico2 configuration to non-volatile storage (NVS) */
#include <nvs_flash.h>
#include <nvs.h>
#include <esp_log.h>
#include "../globals.h"
#include "../types.h"
#include "config.h"
#include "loadsave.h"

static const char* LOADSAVE_TAG = "config_save&load";
static const char* STORAGE_NAMESPACE = "minico2";

esp_err_t save_config() {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open NVS handle
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LOADSAVE_TAG, "Error opening NVS handle for writing: %s", esp_err_to_name(err));
        return err;
    }

    // Write configuration blob
    err = nvs_set_blob(nvs_handle, "config", &MINICO2CONFIG, sizeof(MINICO2CONFIG));
    if (err != ESP_OK) {
        ESP_LOGE(LOADSAVE_TAG, "Error writing config to NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    // Commit changes
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LOADSAVE_TAG, "Error committing NVS changes: %s", esp_err_to_name(err));
    }

    nvs_close(nvs_handle);
    return err;
}

esp_err_t load_config() {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open NVS handle
    err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LOADSAVE_TAG, "Error opening NVS handle for reading: %s. Using defaults", esp_err_to_name(err));
        MINICO2CONFIG = MINICO2CONFIG_DEFAULT;
        return save_config();
    }

    // Read configuration
    size_t required_size = sizeof(MINICO2CONFIG);
    err = nvs_get_blob(nvs_handle, "config", &MINICO2CONFIG, &required_size);
    
    if (err != ESP_OK) {
        ESP_LOGI(LOADSAVE_TAG, "No saved config found. Using defaults");
        MINICO2CONFIG = MINICO2CONFIG_DEFAULT;
        nvs_close(nvs_handle);
        save_config();
    } else {
        ESP_LOGI(LOADSAVE_TAG, "Loaded config");
        log_config(&MINICO2CONFIG);
    }

    nvs_close(nvs_handle);
    return err;
}

static void config_state_change_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    ESP_LOGD(LOADSAVE_TAG, "config_state_change_handler received event");
    ESP_ERROR_CHECK(save_config());
}

esp_err_t init_config_storage(void) {
    // Initialize NVS and load the configuration. Connect to the event loop to save the config whenever it changes.
    esp_err_t err = nvs_flash_init();
    
    // Handle case where NVS partition was truncated
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(LOADSAVE_TAG, "Erasing NVS flash");
        err = nvs_flash_erase();
        if (err != ESP_OK) {
            return err;
        }
        err = nvs_flash_init();
        if (err != ESP_OK) {
            return err;
        }
    }
    ESP_LOGI(LOADSAVE_TAG, "Initialized NVS flash");
    err = load_config();
    if (err != ESP_OK) {
        return err;
    }

    // Register event handler to save the config to NVS whenever it changes
    err = esp_event_handler_instance_register(CONFIG_EVENTS, ESP_EVENT_ANY_ID, config_state_change_handler, NULL, NULL);

    return err;
}