#include <ESP32Servo.h>

// Definição dos pinos
const int pinPescoco      = 4;
const int pinCoxaDireita  = 5;
const int pinCoxaEsquerda = 6;
const int pinPeDireiro    = 7;
const int pinPeEsquerdo   = 15;

// Objetos Servo
Servo servoPescoco;
Servo servoCoxaDireita;
Servo servoCoxaEsquerda;
Servo servoPeDireiro;
Servo servoPeEsquerdo;

// Variáveis para controlar o tempo da cabeça
unsigned long tempoAnteriorCabeca = 0;
int estadoCabeca = 0;
const unsigned long intervaloCabeca = 2000; // 2 segundos em cada posição

// Variáveis para controlar o tempo e os passos das pernas
unsigned long tempoAnteriorPernas = 0;
int estadoPernas = 0;
const unsigned long intervaloPernas = 500; // 500ms entre cada movimento de perna

void setup() {
  Serial.begin(115200);

  // Anexando os motores com os limites seguros do MG90S (500us a 2400us)
  servoPescoco.attach(pinPescoco, 500, 2400);
  servoCoxaDireita.attach(pinCoxaDireita, 500, 2400);
  servoCoxaEsquerda.attach(pinCoxaEsquerda, 500, 2400);
  servoPeDireiro.attach(pinPeDireiro, 500, 2400);
  servoPeEsquerdo.attach(pinPeEsquerdo, 500, 2400);

  // Posição inicial de todas as juntas em 70°
  servoPescoco.write(70);
  servoCoxaEsquerda.write(70);
  servoPeEsquerdo.write(70);
  servoCoxaDireita.write(70);
  servoPeDireiro.write(70);
  
  Serial.println("Robô inicializado e pernas posicionadas em 70°!");
}

void loop() {
  // 1. A cabeça roda de forma independente em segundo plano
  atualizarCabeca();

  // 2. As pernas executam a sequência alternada em segundo plano
  atualizarPernas();
}

// Função independente para controlar a cabeça do robô
void atualizarCabeca() {
  unsigned long tempoAtual = millis();

  if (tempoAtual - tempoAnteriorCabeca >= intervaloCabeca) {
    tempoAnteriorCabeca = tempoAtual;

    switch (estadoCabeca) {
      case 0:
        Serial.println("Cabeça - Posição: 0°");
        servoPescoco.write(0);
        estadoCabeca = 1;
        break;
      case 1:
        Serial.println("Cabeça - Posição: 70° (Centro)");
        servoPescoco.write(70);
        estadoCabeca = 2;
        break;
      case 2:
        Serial.println("Cabeça - Posição: 143°");
        servoPescoco.write(143);
        estadoCabeca = 3;
        break;
      case 3:
        Serial.println("Cabeça - Posição: 70° (Centro)");
        servoPescoco.write(70);
        estadoCabeca = 0;
        break;
    }
  }
}

// Função independente para controlar a sequência alternada das pernas
void atualizarPernas() {
  unsigned long tempoAtual = millis();

  if (tempoAtual - tempoAnteriorPernas >= intervaloPernas) {
    tempoAnteriorPernas = tempoAtual;

    switch (estadoPernas) {
      case 0:
        /*/ 1. Coxa esquerda vai para 40° (enquanto as outras ficam em 70°)
        Serial.println("Pernas -> Coxa Esquerda: 40°");
        servoCoxaEsquerda.write(40);
        servoPeEsquerdo.write(70);
        servoCoxaDireita.write(70);
        servoPeDireiro.write(70);*/
        estadoPernas = 1;
        break;

      case 1:
        /*/ 2. Pé esquerdo vai para 40°
        Serial.println("Pernas -> Pé Esquerdo: 40°");
        servoCoxaEsquerda.write(70); // Retorna coxa esquerda ao centro
        servoPeEsquerdo.write(40);  // Move pé esquerdo
        servoCoxaDireita.write(70);
        servoPeDireiro.write(70);*/
        estadoPernas = 2;
        break;

      case 2:
        /*/ 3. Coxa direita vai para 40°
        Serial.println("Pernas -> Coxa Direita: 40°");
        servoCoxaEsquerda.write(70);
        servoPeEsquerdo.write(70);  // Retorna pé esquerdo ao centro
        servoCoxaDireita.write(40); // Move coxa direita
        servoPeDireiro.write(70);*/
        estadoPernas = 3;
        break;

      case 3:
        // 4. Pé direito vai para 40°
        Serial.println("Pernas -> Pé Direito: 40°");
        //servoCoxaEsquerda.write(70);
        //servoPeEsquerdo.write(70);
        //servoCoxaDireita.write(70); // Retorna coxa direita ao centro
        servoPeDireiro.write(70);  // Move pé direito
        servoPeDireiro.write(0);  // Move pé direito
        estadoPernas = 0;          // Reinicia o ciclo da caminhada
        break;
    }
  }
}