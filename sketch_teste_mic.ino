#include <Arduino.h>
#include "driver/i2s_std.h"

// Pinos do microfone INMP441
#define I2S_SCK 4
#define I2S_WS  5
#define I2S_SD  6

#define SAMPLE_RATE 16000
#define BUFFER_SIZE 256

i2s_chan_handle_t rx_handle = NULL;

int32_t samples[BUFFER_SIZE];

void configurarMicrofone() {

  i2s_chan_config_t chan_cfg =
      I2S_CHANNEL_DEFAULT_CONFIG(
          I2S_NUM_0,
          I2S_ROLE_MASTER
      );

  ESP_ERROR_CHECK(
      i2s_new_channel(
          &chan_cfg,
          NULL,
          &rx_handle
      )
  );

  i2s_std_config_t std_cfg = {

    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),

    .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
        I2S_DATA_BIT_WIDTH_32BIT,
        I2S_SLOT_MODE_MONO
    ),

    .gpio_cfg = {
      .mclk = I2S_GPIO_UNUSED,
      .bclk = (gpio_num_t) I2S_SCK,
      .ws = (gpio_num_t) I2S_WS,
      .dout = I2S_GPIO_UNUSED,
      .din = (gpio_num_t) I2S_SD,
      .invert_flags = {
        .mclk_inv = false,
        .bclk_inv = false,
        .ws_inv = false
      }
    }
  };

  ESP_ERROR_CHECK(
      i2s_channel_init_std_mode(rx_handle, &std_cfg)
  );

  ESP_ERROR_CHECK(
      i2s_channel_enable(rx_handle)
  );

  Serial.println("Microfone I2S inicializado!");
}

void lerAudio() {

  size_t bytes_read = 0;

  esp_err_t resultado = i2s_channel_read(
      rx_handle,
      samples,
      sizeof(samples),
      &bytes_read,
      1000
  );

  if (resultado == ESP_OK && bytes_read > 0) {

    int quantidade = bytes_read / sizeof(int32_t);

    for (int i = 0; i < quantidade; i++) {

      // Extrai os 24 bits úteis do INMP441
      int32_t audio = samples[i] >> 8;

      Serial.println(audio);
    }
  }
}

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println("Iniciando ESP32-S3...");

  configurarMicrofone();
}

void loop() {

  lerAudio();

}
