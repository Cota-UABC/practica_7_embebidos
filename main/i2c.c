//code for MPU6050
#include "driver/i2c.h"
#include <esp_log.h>

#define I2C_SLAVE_ADDR 0x68
#define CLOCK_SPD 400000

#define WHO_AM_I 0x75
#define POW_MANG 0x6B
#define POW_MANG2 0x6C
#define TEMP_OUT 0x41

#define BUFFER_SIZE_READ 10

static const char *TAG_I = "I2C";

static uint8_t read_buffer[BUFFER_SIZE_READ];
static int16_t temp_i;

esp_err_t init_i2c(void);
esp_err_t device_register_read(uint8_t reg_addr, uint8_t *data, uint8_t len, char *reg_name);
esp_err_t device_register_write_byte(uint8_t reg_addr, uint8_t data);
float raw_to_real_value(uint8_t raw_data_h, uint8_t raw_data_l, uint16_t range);
esp_err_t activate_sensor(void);
esp_err_t deactivate_sensor(void);
esp_err_t read_temperature(float *value);

esp_err_t init_i2c(void)
{
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .scl_io_num = 22,
        .sda_io_num = 21,
        .scl_pullup_en = true,
        .sda_pullup_en = true,
        .master.clk_speed = CLOCK_SPD,
        .clk_flags = 0
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, ESP_INTR_FLAG_LEVEL1));

    return ESP_OK;
}

esp_err_t device_register_read(uint8_t reg_addr, uint8_t *data, uint8_t len, char *reg_name)
{
    ESP_ERROR_CHECK(
            i2c_master_write_read_device(I2C_NUM_0,
                                        I2C_SLAVE_ADDR,
                                        &reg_addr,
                                        1,
                                        data,
                                        len,
                                        pdMS_TO_TICKS(2000)));

    //if no reg_name, no log
    if(reg_name[0]){
		ESP_LOGI(TAG_I, "READ:");
		ESP_LOGI(TAG_I, "    Addr read: 0x%x",reg_addr);

		if(len <= 1)
			ESP_LOGI(TAG_I, "    %s= 0x%x",reg_name,data[0]);
		else
		{
			ESP_LOGI(TAG_I, "    %s:",reg_name);
			for(int i=0;i<len;i++)
				ESP_LOGI(TAG_I, "    Data= 0x%x",data[i]);
		}
    }

    return ESP_OK;
}

esp_err_t device_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    uint8_t *write_bytes = (uint8_t *) malloc(2);
    write_bytes[0] = reg_addr;
    write_bytes[1] = data;

    ESP_ERROR_CHECK(
            i2c_master_write_to_device(I2C_NUM_0,
                                        I2C_SLAVE_ADDR,
                                        (const uint8_t*)write_bytes,
                                        2,
                                        pdMS_TO_TICKS(2000)));

    ESP_LOGI(TAG_I, "WRITE:");
    ESP_LOGI(TAG_I, "    Adrr: 0x%x Data: 0x%x",write_bytes[0],write_bytes[1]);

    free(write_bytes);

    return ESP_OK;
}

float raw_to_real_value(uint8_t raw_data_h, uint8_t raw_data_l, uint16_t range)
{
    float value_f;
	uint16_t value = (uint16_t)raw_data_h << 8 | raw_data_l;

	if(value>>15)
	{
	    value = ~value;
	    value++;
	    value_f = ((float)value / 0x8000) * (float)range;
	    value_f = value_f*-1;
	}
	else
	    value_f = ((float)value / 0x7fff) * (float)range;

	return value_f;
}

esp_err_t activate_sensor(void)
{
    //reset sensor
    ESP_ERROR_CHECK( device_register_write_byte(POW_MANG, 0x80) );

    //8MHz clock source sensor
    ESP_ERROR_CHECK( device_register_write_byte(POW_MANG, 0x00) );
    ESP_ERROR_CHECK( device_register_read(POW_MANG, read_buffer, 1,"POWER") );

    return ESP_OK;
}

esp_err_t deactivate_sensor(void)
{
    //put to sleep
    ESP_ERROR_CHECK( device_register_write_byte(POW_MANG, 0x40) );
    ESP_ERROR_CHECK( device_register_read(POW_MANG, read_buffer, 1,"POWER") );

    return ESP_OK;
}

esp_err_t read_temperature(float *value)
{
    ESP_ERROR_CHECK( device_register_read(TEMP_OUT, read_buffer, 2, "") );
    temp_i = (read_buffer[0] << 8) | read_buffer[1];
    *value = ((float)temp_i / 340.0) + 36.53;
    ESP_LOGI(TAG_I, "TEMPERATURE: %.2f C",*value);

    return ESP_OK;
}
