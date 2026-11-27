#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>
#include "driver/i2s_std.h"

// Wi-Fi e IP do seu PC onde roda o servidor Python com Whisper
const char* WIFI_SSID = "NOMEDOWIFI";
const char* WIFI_PASS = "SENHADOWIFI";
const char* STT_SERVER_URL = "http://IP_DO_SERVIDOR:5005/transcribe";

// Pinos do microfone INMP441
#define I2S_SCK 4
#define I2S_WS  5
#define I2S_SD  6

// LED RGB integrado no ESP32-S3
#define RGB_LED_PIN 48
#define NUM_PIXELS  1

// Captura de áudio: 16 kHz, 16-bit, mono por 2.5s = 40.000 amostras (~80 KB na SRAM)
static const uint32_t SAMPLE_RATE = 16000;
static const float RECORD_SECONDS = 2.5f;
static const size_t TOTAL_SAMPLES = (size_t)(SAMPLE_RATE * RECORD_SECONDS);
static const size_t CHUNK_SAMPLES = 512;

// Ajuste de ganho e disparo por energia de voz
static const float AUDIO_GAIN = 1.0f;
static const float TRIGGER_RMS = 2200.0f;

Adafruit_NeoPixel ledRgb(NUM_PIXELS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);
i2s_chan_handle_t rx_handle = NULL;

static int16_t *audio_buffer = NULL;
static int32_t i2s_read_buf[CHUNK_SAMPLES];

static void setLedColor(uint8_t r, uint8_t g, uint8_t b) {
  ledRgb.setPixelColor(0, ledRgb.Color(r, g, b));
  ledRgb.show();
}

static float calculateRms(const int16_t *buf, size_t count) {
  if (count == 0) return 0.0f;

  double sum_sq = 0.0;
  for (size_t i = 0; i < count; i++) {
    double s = (double)buf[i];
    sum_sq += s * s;
  }
  return (float)sqrt(sum_sq / (double)count);
}

static bool initMicrophoneI2S() {
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  if (i2s_new_channel(&chan_cfg, NULL, &rx_handle) != ESP_OK) {
    Serial.println("Falha ao criar canal I2S.");
    return false;
  }

  i2s_std_config_t std_cfg = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
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

  return true;
}

static size_t readAudioChunk(int16_t *dest, size_t max_samples) {
  size_t bytes_to_read = max_samples * sizeof(int32_t);
  size_t bytes_read = 0;

  esp_err_t res = i2s_channel_read(rx_handle, i2s_read_buf, bytes_to_read, &bytes_read, 1000);
  if (res != ESP_OK || bytes_read == 0) {
    return 0;
  }

  size_t samples_read = bytes_read / sizeof(int32_t);
  for (size_t i = 0; i < samples_read; i++) {
    int16_t sample16 = (int16_t)(i2s_read_buf[i] >> 16);
    float scaled = (float)sample16 * AUDIO_GAIN;

    if (scaled > 32767.0f) scaled = 32767.0f;
    if (scaled < -32768.0f) scaled = -32768.0f;

    dest[i] = (int16_t)scaled;
  }

  return samples_read;
}

static String sendAudioToLocalTranscriber(const int16_t *samples, size_t sampleCount) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado.");
    return "";
  }

  HTTPClient http;
  http.setTimeout(15000);

  if (!http.begin(STT_SERVER_URL)) {
    Serial.println("Falha ao iniciar requisicao HTTP.");
    return "";
  }

  http.addHeader("Content-Type", "application/octet-stream");

  size_t payloadBytes = sampleCount * sizeof(int16_t);
  int httpCode = http.POST((uint8_t *)samples, payloadBytes);

  String transcript = "";
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      transcript = http.getString();
      transcript.trim();
    } else {
      Serial.printf("Servidor retornou HTTP %d\n", httpCode);
    }
  } else {
    Serial.printf("Erro HTTP: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  return transcript;
}

void setup() {
  Serial.begin(115200);
  delay(800);

  ledRgb.begin();
  ledRgb.setBrightness(40);
  setLedColor(0, 0, 20); // Azul fraco indicando aguardando Wi-Fi

  audio_buffer = (int16_t *)malloc(TOTAL_SAMPLES * sizeof(int16_t));
  if (audio_buffer == NULL) {
    Serial.println("Falha ao alocar buffer de audio na SRAM interna.");
    while (true) delay(1000);
  }

  if (!initMicrophoneI2S()) {
    while (true) delay(1000);
  }

  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Wi-Fi conectado! IP da ESP32-S3: ");
  Serial.println(WiFi.localIP());

  setLedColor(0, 20, 0); // Verde fraco indicando pronto para ouvir
  Serial.println("Pronto! Fale pelo microfone INMP441 para ver o texto no Monitor Serial.");
}

void loop() {
  size_t readCount = readAudioChunk(audio_buffer, CHUNK_SAMPLES);
  if (readCount == 0) {
    delay(10);
    return;
  }

  float rms = calculateRms(audio_buffer, readCount);

  if (rms >= TRIGGER_RMS) {
    setLedColor(80, 80, 0); // Amarelo durante captura e transcrição
    Serial.printf("\n[Fala detectada | RMS: %.0f] Gravando %.1fs...\n", rms, RECORD_SECONDS);

    size_t captured = readCount;
    while (captured < TOTAL_SAMPLES) {
      size_t remaining = TOTAL_SAMPLES - captured;
      size_t toRead = (remaining > CHUNK_SAMPLES) ? CHUNK_SAMPLES : remaining;
      size_t got = readAudioChunk(&audio_buffer[captured], toRead);
      if (got == 0) break;
      captured += got;
    }

    String text = sendAudioToLocalTranscriber(audio_buffer, captured);
    if (text.length() > 0) {
      Serial.print("VOCE DISSE: ");
      Serial.println(text);
    } else {
      Serial.println("(Nenhuma transcricao retornada)");
    }

    setLedColor(0, 20, 0); // Volta para pronto
    delay(300);
  }

  delay(10);
}
