#include <Servo.h>

Servo pescoco;
Servo coxaEsquerda;
Servo peEsquerdo;
Servo coxaDireita;
Servo peDireito;

// Variáveis de controle de tempo (substituem o delay)
unsigned long tempoAnterior = 0;
int estadoMovimento = 0;

void setup() {
  Serial.begin(9600);    // Inicializa a comunicação serial para visualizar os prints

  pescoco.attach(6);  // Pino 6 = Pescoço (Angulo seguro = 67)
  coxaEsquerda.attach(3);  // Pino 3 = Coxa Esquerda (Angulo seguro = 70)
  peEsquerdo.attach(2);  // Pino 2 = Pé Esquerdo (Angulo seguro = 40)
  coxaDireita.attach(5);  // Pino 5 = Coxa Direita (Angulo seguro = 0)
  peDireito.attach(4);  // Pino 4 = Pé Direito (Angulo seguro = 0 | Limite de angulo = 50)

  // POSIÇÃO INICIAL DE CADA SERVO MOTOR
  pescoco.write(67);
  coxaEsquerda.write(100);
  peEsquerdo.write(40);
  coxaDireita.write(30);
  peDireito.write(40);
}

void loop() {

  //controlarPescoco();
  andarFrente();

}

void controlarPescoco() {
  unsigned long tempoAtual = millis();

  // Verifica se já passaram os milissegundos definidos abaixo para o próximo case
  if (tempoAtual - tempoAnterior >= 3000) {
    tempoAnterior = tempoAtual; // Atualiza a referência de tempo

    // Sequência de movimentos
    switch (estadoMovimento) {
      case 0:
        Serial.println("Pecoço = Direita");
        pescoco.write(10);
        estadoMovimento = 1;
        break;
        
      case 1:
        Serial.println("Pecoço = Frente");
        pescoco.write(67);
        estadoMovimento = 2;
        break;
        
      case 2:
        Serial.println("Pecoço = Esquerda");
        pescoco.write(135);
        estadoMovimento = 3;
        break;
        
      case 3:
        Serial.println("Pecoço = Frente");
        pescoco.write(67);
        estadoMovimento = 0; // Reinicia o ciclo
        break;
    }
  }
}


void andarFrente() {
  unsigned long tempoAtual = millis();

  // Verifica se já passaram os milissegundos definidos abaixo para o próximo case
  if (tempoAtual - tempoAnterior >= 3000) {
    tempoAnterior = tempoAtual; // Atualiza a referência de tempo

    // Sequência de movimentos
    switch (estadoMovimento) {
      case 0:
        Serial.println("Pés para frente");
        peEsquerdo.write(40); // Pé no chão
        peDireito.write(40); // Pé no chão
        coxaEsquerda.write(100); // Coxa para frente
        coxaDireita.write(30); // Coxa para frente
        estadoMovimento = 1;
        break;

      case 1:
        Serial.println("Pés para dentro");
        peEsquerdo.write(70); // Pé para dentro
        peDireito.write(70); // Pé para fora
        coxaEsquerda.write(130); // Coxa para direita
        coxaDireita.write(60); // Coxa para  direita
        estadoMovimento = 2;
        break;

      case 2:
        Serial.println("Pés para frente");
        peEsquerdo.write(40); // Pé no chão
        peDireito.write(40); // Pé no chão
        coxaEsquerda.write(100); // Coxa para frente
        coxaDireita.write(30); // Coxa para frente
        estadoMovimento = 3;
        break;
      
      case 3:
        Serial.println("Pés para fora");
        peEsquerdo.write(10); // Pé para fora
        peDireito.write(10); // Pé para dentro
        coxaEsquerda.write(70); // Coxa para esquerda
        coxaDireita.write(0); // Coxa para  esquerda
        estadoMovimento = 0;
        break;
    }
  }
}
