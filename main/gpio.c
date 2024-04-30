#include "driver/gpio.h"

#define GPIO_LED 2
#define ON 1
#define OFF 0

uint8_t toggle = 0;

esp_err_t gpio_init();
void toggle_led();

esp_err_t gpio_init()
{
	gpio_set_direction(2, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_LED, OFF);

	return ESP_OK;
}

void toggle_led()
{
    //static uint8_t toggle = 0;

    toggle = !toggle;
    (toggle == 1) ? gpio_set_level(GPIO_LED, ON) : gpio_set_level(GPIO_LED, OFF);
}