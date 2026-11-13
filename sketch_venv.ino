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
  //cabecaNao(10,135);
  //cabecaNao(135,10);

  // ANDAR PARA FRENTE
  //numInicial e numFinal
  //peEsquerdo, peDireito, coxaEsquerda, coxaDireita
  andar(70,10,  70,10,  70,130, 0,60);
  andar(10,70,  10,70,  130,70, 60,0);

  // Girar
  //andar(10, 70,   70, 10,   70, 130,   60,  0);
  //andar(70, 10,   10, 70,   130, 70,    0, 60);

}

// Função auxiliar que move QUALQUER servo de forma fluida
void moverServoFluido(Servo &motor, int inicial, int final) {
  if (inicial < final) {
    for (int angulo = inicial; angulo <= final; angulo++) {
      motor.write(angulo);
      delay(15);
    }
  } else {
    for (int angulo = inicial; angulo >= final; angulo--) {
      motor.write(angulo);
      delay(15);
    }
  }
}

void cabecaNao(int anguloInicial, int anguloFinal) {
  moverServoFluido(pescoco, anguloInicial, anguloFinal);
}

void andar(int peEsqIni, int peEsqFin, int peDirIni, int peDirFin, 
                 int coxaEsqIni, int coxaEsqFin, int coxaDirIni, int coxaDirFin) {
  
  // Descobre o maior deslocamento comparando de 2 em 2
  int maxPassos = max(max(abs(peEsqFin - peEsqIni), abs(peDirFin - peDirIni)),
                      max(abs(coxaEsqFin - coxaEsqIni), abs(coxaDirFin - coxaDirIni)));

  // Executa o movimento de todos juntos passo a passo
  for (int i = 0; i <= maxPassos; i++) {
    int anguloPeEsq = map(i, 0, maxPassos, peEsqIni, peEsqFin);
    int anguloPeDir = map(i, 0, maxPassos, peDirIni, peDirFin);
    int anguloCoxaEsq = map(i, 0, maxPassos, coxaEsqIni, coxaEsqFin);
    int anguloCoxaDir = map(i, 0, maxPassos, coxaDirIni, coxaDirFin);

    peEsquerdo.write(anguloPeEsq);
    peDireito.write(anguloPeDir);
    coxaEsquerda.write(anguloCoxaEsq);
    coxaDireita.write(anguloCoxaDir);

    delay(15); 
  }
}
