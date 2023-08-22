#include <esp_event_loop.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_spiffs.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <stdio.h>
#include <string.h>

#define TAG "HTTP_SERVER"

// Function to handle serving the index.html file
esp_err_t index_handler(httpd_req_t *req) {
    FILE *indexFile = fopen("/spiffs/index.html", "r");
    if (indexFile == NULL) {
        ESP_LOGE(TAG, "Failed to open index.html file");
        return ESP_FAIL;
    }

    char line[256];
    while (fgets(line, sizeof(line), indexFile)) {
        if (httpd_resp_sendstr_chunk(req, line) != ESP_OK) {
            fclose(indexFile);
            ESP_LOGE(TAG, "Error sending data");
            return ESP_FAIL;
        }
    }
    fclose(indexFile);
    httpd_resp_sendstr_chunk(req, NULL); // End response
    return ESP_OK;
}

// HTTP server event handler
esp_err_t http_event_handler(httpd_req_t *req) {
    switch (req->event_id) {
    case HTTP_EVENT_ON_REQUEST:
        if (strcmp(req->uri, "/") == 0) {
            return index_handler(req);
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

// Initialize SPIFFS
void init_spiffs() {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs", .partition_label = NULL, .max_files = 5, .format_if_mount_failed = true};

    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));
    size_t total, used;
    ESP_ERROR_CHECK(esp_spiffs_info(NULL, &total, &used));
    printf("SPIFFS Info - Total: %d, Used: %d\n", total, used);
}

// Initialize the HTTP server
void start_http_server() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    // Start the server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    ESP_ERROR_CHECK(httpd_start(&server, &config));

    // Register URI handlers
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &index_handler));
}

// Main application
void app_main() {
    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());

    // Initialize SPIFFS
    init_spiffs();

    // Start the HTTP server
    start_http_server();
}
