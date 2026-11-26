#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "driver/i2s_std.h"
#include <cassiomdrs-project-1_inferencing.h>

// Pinos do microfone INMP441
#define I2S_SCK 4
#define I2S_WS  5
#define I2S_SD  6

// LED RGB integrado no ESP32-S3
#define RGB_LED_PIN 48
#define NUM_PIXELS  1

// Ganho digital para reforçar sinais baixos (ajuste conforme necessário)
static const float AUDIO_GAIN = 2.0f;

// Confiança mínima para acionar a cor (ajuste para 0.50f se estiver testando com poucas amostras)
static const float MIN_CONFIDENCE = 0.70f;

Adafruit_NeoPixel ledRgb(NUM_PIXELS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);
i2s_chan_handle_t rx_handle = NULL;

typedef struct {
  int16_t *buffer;
  size_t buf_ready;
  size_t buf_count;
  size_t n_samples;
} inference_t;

static inference_t inference;
static int32_t i2s_read_buf[256];
static volatile bool record_ready = false;

static void setLedColor(uint8_t r, uint8_t g, uint8_t b) {
  ledRgb.setPixelColor(0, ledRgb.Color(r, g, b));
  ledRgb.show();
}

static void handleVoiceCommand(const char *label, float value) {
  if (value < MIN_CONFIDENCE) {
    return;
  }

  if (strcmp(label, "verde") == 0) {
    setLedColor(0, 255, 0);
    Serial.printf("Detectado 'verde' (%.2f) -> LED VERDE\n", value);
  } else if (strcmp(label, "azul") == 0) {
    setLedColor(0, 0, 255);
    Serial.printf("Detectado 'azul' (%.2f) -> LED AZUL\n", value);
  } else if (strcmp(label, "vermelho") == 0) {
    setLedColor(255, 0, 0);
    Serial.printf("Detectado 'vermelho' (%.2f) -> LED VERMELHO\n", value);
  }
}

static void audio_inference_callback(uint32_t n_bytes) {
  size_t n_samples_read = n_bytes / sizeof(int32_t);

  for (size_t i = 0; i < n_samples_read; i++) {
    int32_t sample24 = i2s_read_buf[i] >> 8;
    float scaled = ((float)sample24 / 32768.0f) * AUDIO_GAIN;

    if (scaled > 32767.0f) scaled = 32767.0f;
    if (scaled < -32768.0f) scaled = -32768.0f;

    inference.buffer[inference.buf_count++] = (int16_t)scaled;

    if (inference.buf_count >= inference.n_samples) {
      inference.buf_count = 0;
      inference.buf_ready = 1;
    }
  }
}

static void capture_samples_task(void *arg) {
  const size_t n_bytes = sizeof(i2s_read_buf);

  while (record_ready) {
    size_t bytes_read = 0;
    esp_err_t res = i2s_channel_read(
      rx_handle,
      i2s_read_buf,
      n_bytes,
      &bytes_read,
      1000
    );

    if (res == ESP_OK && bytes_read > 0) {
      audio_inference_callback(bytes_read);
    }
  }

  vTaskDelete(NULL);
}

static bool microphone_inference_start(size_t n_samples) {
  inference.buffer = (int16_t *)malloc(n_samples * sizeof(int16_t));
  if (inference.buffer == NULL) {
    Serial.println("Falha ao alocar buffer de audio na SRAM interna.");
    return false;
  }

  inference.buf_count = 0;
  inference.n_samples = n_samples;
  inference.buf_ready = 0;
  record_ready = true;

  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  if (i2s_new_channel(&chan_cfg, NULL, &rx_handle) != ESP_OK) {
    Serial.println("Falha ao criar canal I2S.");
    return false;
  }

  i2s_std_config_t std_cfg = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(EI_CLASSIFIER_FREQUENCY),
    .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
      I2S_DATA_BIT_WIDTH_32BIT,
      I2S_SLOT_MODE_MONO
    ),
    .gpio_cfg = {
      .mclk = I2S_GPIO_UNUSED,
      .bclk = (gpio_num_t)I2S_SCK,
      .ws = (gpio_num_t)I2S_WS,
      .dout = I2S_GPIO_UNUSED,
      .din = (gpio_num_t)I2S_SD,
      .invert_flags = {
        .mclk_inv = false,
        .bclk_inv = false,
        .ws_inv = false
      }
    }
  };

  if (i2s_channel_init_std_mode(rx_handle, &std_cfg) != ESP_OK ||
      i2s_channel_enable(rx_handle) != ESP_OK) {
    Serial.println("Falha ao iniciar microfone I2S.");
    return false;
  }

  xTaskCreate(capture_samples_task, "i2s_capture", 4096, NULL, 10, NULL);
  return true;
}

static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr) {
  numpy::int16_to_float(&inference.buffer[offset], out_ptr, length);
  return 0;
}

void setup() {
  Serial.begin(115200);
  delay(800);

  ledRgb.begin();
  ledRgb.setBrightness(40);
  setLedColor(0, 0, 0);

  Serial.println("Iniciando inferencia de voz na ESP32-S3...");

  if (!microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT)) {
    Serial.println("Erro ao iniciar captura de audio.");
    while (true) {
      delay(1000);
    }
  }
}

void loop() {
  while (!inference.buf_ready) {
    delay(5);
  }
  inference.buf_ready = 0;

  signal_t signal;
  signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
  signal.get_data = &microphone_audio_signal_get_data;

  ei_impulse_result_t result = {0};
  EI_IMPULSE_ERROR r = run_classifier(&signal, &result, false);

  if (r != EI_IMPULSE_OK) {
    Serial.printf("Erro ao executar classificador (%d)\n", r);
    return;
  }

  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    handleVoiceCommand(result.classification[ix].label, result.classification[ix].value);
  }

  delay(20);
}
