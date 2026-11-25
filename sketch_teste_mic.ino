#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_timer.h"

const char* WIFI_SSID = "F-30";
const char* WIFI_SENHA = "Domo@1234";
const char* SERVER_URL = "http://192.168.0.100:5000/transcrever"; // Troque pelo IP do seu PC

const int PINO_MIC = 4; // Escolha um pino ADC1 válido na sua ESP32-S3
const int SAMPLE_RATE = 16000;
const int SEGUNDOS_GRAVACAO = 2;
const size_t NUM_AMOSTRAS = SAMPLE_RATE * SEGUNDOS_GRAVACAO;
const size_t PCM_BYTES = NUM_AMOSTRAS * sizeof(int16_t);
const size_t WAV_SIZE = 44 + PCM_BYTES;

volatile size_t idxAmostra = 0;
volatile bool gravando = false;
int16_t* bufferPCM = nullptr;
uint8_t* bufferWAV = nullptr;
esp_timer_handle_t timerAmostra;

void IRAM_ATTR onSampleTimer(void* arg) {
  if (!gravando) return;
  if (idxAmostra < NUM_AMOSTRAS) {
    int raw = analogRead(PINO_MIC); // 0..4095 (12 bits)
    // Centraliza em torno de 2048 e converte para int16_t
    int32_t centered = (int32_t)raw - 2048;
    int16_t sample16 = (int16_t)(centered * 16);
    bufferPCM[idxAmostra++] = sample16;
  } else {
    gravando = false;
    esp_timer_stop(timerAmostra);
  }
}

void escreverCabecalhoWAV(uint8_t* wav, uint32_t pcmBytes, uint32_t sampleRate) {
  uint32_t byteRate = sampleRate * 1 * 2; // 1 canal, 16 bits (2 bytes)
  uint32_t chunkSize = 36 + pcmBytes;

  memcpy(wav + 0, "RIFF", 4);
  memcpy(wav + 4, &chunkSize, 4);
  memcpy(wav + 8, "WAVE", 4);
  memcpy(wav + 12, "fmt ", 4);
  uint32_t subchunk1Size = 16;
  uint16_t audioFormat = 1; // PCM
  uint16_t numChannels = 1;
  uint16_t bitsPerSample = 16;
  uint16_t blockAlign = 2;
  memcpy(wav + 16, &subchunk1Size, 4);
  memcpy(wav + 20, &audioFormat, 2);
  memcpy(wav + 22, &numChannels, 2);
  memcpy(wav + 24, &sampleRate, 4);
  memcpy(wav + 28, &byteRate, 4);
  memcpy(wav + 32, &blockAlign, 2);
  memcpy(wav + 34, &bitsPerSample, 2);
  memcpy(wav + 36, "data", 4);
  memcpy(wav + 40, &pcmBytes, 4);
}

void gravarEEnviar() {
  if (!bufferPCM || !bufferWAV) {
    Serial.println("Erro: buffers de audio nao alocados.");
    return;
  }

  idxAmostra = 0;
  gravando = true;
  Serial.println("Falando agora... gravando 2s!");

  esp_timer_start_periodic(timerAmostra, 1000000ULL / SAMPLE_RATE);

  while (gravando) {
    delay(10);
  }

  Serial.println("Gravacao concluida. Montando WAV e enviando...");
  escreverCabecalhoWAV(bufferWAV, PCM_BYTES, SAMPLE_RATE);
  memcpy(bufferWAV + 44, bufferPCM, PCM_BYTES);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "audio/wav");

    int httpCode = http.POST(bufferWAV, WAV_SIZE);
    if (httpCode > 0) {
      String resposta = http.getString();
      resposta.trim();
      Serial.print("Voce disse: ");
      Serial.println(resposta);
    } else {
      Serial.print("Falha no HTTP POST: ");
      Serial.println(http.errorToString(httpCode));
    }
    http.end();
  } else {
    Serial.println("Wi-Fi desconectado.");
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(PINO_MIC, ADC_11db);

  // Usa PSRAM se disponível, senão heap normal
  if (psramFound()) {
    bufferPCM = (int16_t*)ps_malloc(PCM_BYTES);
    bufferWAV = (uint8_t*)ps_malloc(WAV_SIZE);
  } else {
    bufferPCM = (int16_t*)malloc(PCM_BYTES);
    bufferWAV = (uint8_t*)malloc(WAV_SIZE);
  }

  esp_timer_create_args_t timerArgs = {
    .callback = &onSampleTimer,
    .arg = nullptr,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "mic_adc_16khz"
  };
  esp_timer_create(&timerArgs, &timerAmostra);

  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado! IP: " + WiFi.localIP().toString());
  Serial.println("Envie 'g' no Serial para gravar 2s e transcrever.");
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'g' || cmd == 'G') {
      gravarEEnviar();
    }
  }
}
