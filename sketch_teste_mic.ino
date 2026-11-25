#include <math.h>

class Microfone {
  private:
    int pinoAnalogico;
    float offsetCalibracao;

  public:
    Microfone(int pino, float offset = 45.0) {
      pinoAnalogico = pino;
      offsetCalibracao = offset;
    }

    void iniciar() {
      pinMode(pinoAnalogico, INPUT);
    }

    int lerDecibeis(int amostras, int &amp;amplitudeOut) {
      if (amostras <= 0) {
        amplitudeOut = 0;
        return 0;
      }

      int menorValor = 4095;
      int maiorValor = 0;

      for (int i = 0; i < amostras; i++) {
        int leitura = analogRead(pinoAnalogico);
        if (leitura < menorValor) menorValor = leitura;
        if (leitura > maiorValor) maiorValor = leitura;
      }

      int amplitude = maiorValor - menorValor;
      amplitudeOut = amplitude;

      if (amplitude <= 1) {
        return 0;
      }

      float db = 20.0 * log10((float)amplitude) + offsetCalibracao;
      if (db < 0.0) db = 0.0;
      return (int)round(db);
    }
};

// Ajuste para um pino ADC1 válido da sua placa (ex.: 34 na ESP32 clássica)
Microfone meuMic(34);

void setup() {
  Serial.begin(115200);
  meuMic.iniciar();
}

void loop() {
  int amplitude = 0;
  int ruido = meuMic.lerDecibeis(200, amplitude);

  Serial.print("Amplitude: ");
  Serial.print(amplitude);
  Serial.print(" | Ruido: ");
  Serial.print(ruido);
  Serial.println(" dB");

  delay(500);
}
