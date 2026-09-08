#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "BLINK";
constexpr gpio_num_t LIGHT_STATUS = GPIO_NUM_33;
constexpr gpio_num_t LIGHT_FLASH = GPIO_NUM_4;

extern "C" void app_main(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LIGHT_STATUS) | (1ULL << LIGHT_FLASH),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "Led blinking started on GPIO %d", LIGHT_STATUS);

    bool led_state = false;
    bool flash_state = false;
    int count = 0;

    while (1)
    {
        led_state = !led_state;
        gpio_set_level(LIGHT_STATUS, led_state ? 1 : 0);
        ESP_LOGI(TAG, "Led state: %s", led_state ? "ON" : "OFF");
        vTaskDelay(pdMS_TO_TICKS(500));
        count += 1;
        if (count % 4 == 0) {
            flash_state = !flash_state;
            gpio_set_level(LIGHT_FLASH, flash_state ? 1 : 0);
            ESP_LOGI(TAG, "Flash State: %s", flash_state ? "ON" : "OFF");
            count = 0;
        }
    }
}
