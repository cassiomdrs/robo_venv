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
  
  peEsquerdo.write(30);
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
  peEsquerdo.write(30);
  peDireito.write(40);
  coxaEsquerda.write(100);
  coxaDireita.write(30);
  
  Serial.println("Obstáculo próximo! Parando ou ajustando...");
  /*cabecaNao(67,10);
  cabecaNao(10,135);
  cabecaNao(135,67);*/
  } 
  else {
    /*/ O robô só anda se a distância for MAIOR que 10 cm
    andar(30,0,    40,10,    100,130,   30,60);
    andar(0,60,    10,70,    130,70,    60,0);
    andar(60,30,    70,40,    70,100,    0,30);*/

/*---------------------------------------------------------*/

/*/ Andar para frente com os pés invertidos
    andar(30, 0,   40, 70,   100, 130,  30, 0);
    andar(0, 60,   70, 10,   130, 70,   0, 60);
    andar(60, 30,   10, 40,   70, 100,   60, 30);*/

    // Andar para frente com os pés para o mesmo lado
    andar(30,0,    40,10,    100,130,   30,60);
    andar(0,60,    10,70,    130,70,    60,0);
    andar(60,30,    70,40,    70,100,    0,30);
  
    /*/ Tentativa de girar
    //    Pé esq    Pé Dir    Coxa Esq   Coxa Dir
    andar(30, 0,   40, 40,   100, 130,  30, 30);
    andar(0, 0,   40, 70,   130, 130,  30, 0);

    andar(0, 60,   70, 70,   130, 70,   0, 0);
    andar(60, 60,   70, 10,   70, 70,   0, 60);
    
    andar(60, 30,   10, 10,   70, 100,   60, 60);
    andar(30, 30,   10, 40,   100, 100,   60, 30);*/

  }

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

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
  
  // --- FASE 1: Movimento dos Pés ---
  // Descobre o maior deslocamento apenas para os pés
  int maxPassosPes = max(abs(peEsqFin - peEsqIni), abs(peDirFin - peDirIni));
  
  if (maxPassosPes > 0) {
    for (int i = 0; i <= maxPassosPes; i++) {
      int anguloPeEsq = map(i, 0, maxPassosPes, peEsqIni, peEsqFin);
      int anguloPeDir = map(i, 0, maxPassosPes, peDirIni, peDirFin);

      peEsquerdo.write(anguloPeEsq);
      peDireito.write(anguloPeDir);

      delay(10); // Velocidade dos pés
    }
  }

  // --- FASE 2: Movimento das Coxas (só inicia após os pés terminarem) ---
  // Descobre o maior deslocamento apenas para as coxas
  int maxPassosCoxas = max(abs(coxaEsqFin - coxaEsqIni), abs(coxaDirFin - coxaDirIni));
  
  if (maxPassosCoxas > 0) {
    for (int i = 0; i <= maxPassosCoxas; i++) {
      int anguloCoxaEsq = map(i, 0, maxPassosCoxas, coxaEsqIni, coxaEsqFin);
      int anguloCoxaDir = map(i, 0, maxPassosCoxas, coxaDirIni, coxaDirFin);

      coxaEsquerda.write(anguloCoxaEsq);
      coxaDireita.write(anguloCoxaDir);

      delay(10); // Delay de 8 ms solicitado para as coxas
    }
  }
}
