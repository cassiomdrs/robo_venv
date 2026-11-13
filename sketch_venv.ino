#include <Servo.h>

Servo pescoco;
Servo coxaEsquerda;
Servo peEsquerdo;
Servo coxaDireita;
Servo peDireito;

// Variáveis de controle de tempo (substituem o delay)
unsigned long tempoAnterior = 0;
int estadoMovimento = 0;

const int pinoTrig = 9;
const int pinoEcho = 8;

void setup() {
  Serial.begin(9600);    // Inicializa a comunicação serial para visualizar os prints

  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoEcho, INPUT);

  pescoco.attach(6);  // Pino 6 = Pescoço (Angulo seguro = 67)
  coxaEsquerda.attach(3);  // Pino 3 = Coxa Esquerda (Angulo seguro = 70)
  peEsquerdo.attach(2);  // Pino 2 = Pé Esquerdo (Angulo seguro = 40)
  coxaDireita.attach(5);  // Pino 5 = Coxa Direita (Angulo seguro = 0)
  peDireito.attach(4);  // Pino 4 = Pé Direito (Angulo seguro = 0 | Limite de angulo = 50)

  // POSIÇÃO INICIAL DE CADA SERVO MOTOR
  pescoco.write(67);
  
  peEsquerdo.write(40);
  peDireito.write(40);
  coxaEsquerda.write(100);
  coxaDireita.write(30);
}

void loop() {
  
  // Limpa o pino Trig
  digitalWrite(pinoTrig, LOW);
  delayMicroseconds(2);
  
  // Envia um pulso de 10 microssegundos para o Trig
  digitalWrite(pinoTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);
  
  // Lê o tempo de duração do eco em microssegundos
  long duracao = pulseIn(pinoEcho, HIGH);
  
  // Calcula a distância em centímetros (Velocidade do som = 0.034 cm/us, dividido por 2 ida e volta)
  float distancia = duracao * 0.034 / 2;

  if (distancia <= 30) {
  // Ações quando o obstáculo estiver perto (< 30 cm)
  peEsquerdo.write(40);
  peDireito.write(40);
  coxaEsquerda.write(100);
  coxaDireita.write(30);
  
  Serial.println("Obstáculo próximo! Parando ou ajustando...");
  cabecaNao(67,10);
  cabecaNao(10,135);
  cabecaNao(135,67);
  } 
  else {
    // O robô só anda se a distância for MAIOR que 10 cm
    andar(40,10,  40,10,  100,130, 30,60);
    andar(10,70,  10,70,  130,70, 60,0);
    andar(70,40,  70,40,  70,100, 0,30);
  }

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  
  /*  
  pernasInicial(); // 40, 40, 100, 30
  delay(300);
  andar(40,10,  40,10,  100,130, 30,60);
  andar(10,30,  10,30,  130,90, 60,20);
  
  pernasInicial();
  delay(300);
  andar(40,70,  40,70,  100,70, 30,0);
  andar(70,30,  70,30,  70,90, 0,20);
  */

  /*
  cabecaNao(67,10);
  cabecaNao(10,135);
  cabecaNao(135,67);
  
  andar(40,70,  40,70,  100,70, 30,0);

  andar(70,10,  70,10,  70,130, 0,60);
  andar(10,70,  10,70,  130,70, 60,0);

  andar(70,10,  70,10,  70,130, 0,60);
  andar(10,70,  10,70,  130,70, 60,0);
  
  andar(70,10,  70,10,  70,130, 0,60);
  andar(10,70,  10,70,  130,70, 60,0);

  andar(70,40,  70,40,  70,100, 0,30);
  */

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

void pernasInicial() {
  peEsquerdo.write(40);
  peDireito.write(40);
  coxaEsquerda.write(100);
  coxaDireita.write(30);
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
