#include <Servo.h>

// Instâncias dos Servos
Servo pescoco;
Servo coxaEsquerda;
Servo peEsquerdo;
Servo coxaDireita;
Servo peDireito;

// Pinos do Sensor Ultrassônico
const int pinoTrig = 9;
const int pinoEcho = 8;

// Posições Neutras / Seguras
const int POS_NEUTRA_PESCOCO  = 67;
const int POS_NEUTRA_PE_ESQ   = 30;
const int POS_NEUTRA_PE_DIR   = 40;
const int POS_NEUTRA_COXA_ESQ = 100;
const int POS_NEUTRA_COXA_DIR = 30;

// Variáveis de controle de posição atual dos servos
int posAtualPescoco  = POS_NEUTRA_PESCOCO;
int posAtualPeEsq    = POS_NEUTRA_PE_ESQ;
int posAtualPeDir    = POS_NEUTRA_PE_DIR;
int posAtualCoxaEsq  = POS_NEUTRA_COXA_ESQ;
int posAtualCoxaDir  = POS_NEUTRA_COXA_DIR;

void setup() {
  Serial.begin(9600);

  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoEcho, INPUT);

  pescoco.attach(6);
  coxaEsquerda.attach(3);
  peEsquerdo.attach(2);
  coxaDireita.attach(5);
  peDireito.attach(4);

  // Inicializa na posição neutra segura
  irParaPosicaoInicial();
}

void loop() {
  float distancia = lerDistancia();

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  if (distancia <= 30 && distancia > 0) {
    Serial.println("Obstáculo próximo! Parando e voltando à posição segura...");
    irParaPosicaoInicial();
    delay(500); // Pausa de segurança
  } 
  else {
    // Executa a sequência de passos. 
    // Se algum passo retornar 'false' (obstáculo detectado), o loop é interrompido.
    if (!andarPasso(0,  10, 130, 60)) return;
    if (!andarPasso(60, 70, 70,  0))  return;
    if (!andarPasso(30, 40, 100, 30)) return;
  }
}

// -------------------------------------------------------------
// FUNÇÕES AUXILIARES DE NAVEGAÇÃO E SENSORES
// -------------------------------------------------------------

// Lê a distância em cm do sensor ultrassônico
float lerDistancia() {
  digitalWrite(pinoTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);

  long duracao = pulseIn(pinoEcho, HIGH, 30000); // Timeout de 30ms para não travar
  if (duracao == 0) return 999.0; // Sem eco (caminho livre)

  return (duracao * 0.034) / 2.0;
}

// Move todos os servos suavemente de onde estão até a posição neutra
void irParaPosicaoInicial() {
  int maxPassos = 0;
  maxPassos = max(maxPassos, abs(POS_NEUTRA_PE_ESQ - posAtualPeEsq));
  maxPassos = max(maxPassos, abs(POS_NEUTRA_PE_DIR - posAtualPeDir));
  maxPassos = max(maxPassos, abs(POS_NEUTRA_COXA_ESQ - posAtualCoxaEsq));
  maxPassos = max(maxPassos, abs(POS_NEUTRA_COXA_DIR - posAtualCoxaDir));

  if (maxPassos > 0) {
    int peEsqIni = posAtualPeEsq, peDirIni = posAtualPeDir;
    int coxaEsqIni = posAtualCoxaEsq, coxaDirIni = posAtualCoxaDir;

    for (int i = 0; i <= maxPassos; i++) {
      posAtualPeEsq   = map(i, 0, maxPassos, peEsqIni, POS_NEUTRA_PE_ESQ);
      posAtualPeDir   = map(i, 0, maxPassos, peDirIni, POS_NEUTRA_PE_DIR);
      posAtualCoxaEsq = map(i, 0, maxPassos, coxaEsqIni, POS_NEUTRA_COXA_ESQ);
      posAtualCoxaDir = map(i, 0, maxPassos, coxaDirIni, POS_NEUTRA_COXA_DIR);

      peEsquerdo.write(posAtualPeEsq);
      peDireito.write(posAtualPeDir);
      coxaEsquerda.write(posAtualCoxaEsq);
      coxaDireita.write(posAtualCoxaDir);
      delay(10);
    }
  }
}

// Executa um passo partindo da posição ATUAL até a posição ALVO
// Retorna 'false' se um obstáculo for detectado durante o movimento.
bool andarPasso(int peEsqAlvo, int peDirAlvo, int coxaEsqAlvo, int coxaDirAlvo) {

  // --- FASE 1: Movimento dos Pés ---
  int maxPassosPes = max(abs(peEsqAlvo - posAtualPeEsq), abs(peDirAlvo - posAtualPeDir));
  
  if (maxPassosPes > 0) {
    int peEsqIni = posAtualPeEsq;
    int peDirIni = posAtualPeDir;

    for (int i = 0; i <= maxPassosPes; i++) {
      // Checa obstáculo a cada micro-passo dos pés
      if (lerDistancia() <= 30) {
        return false; // Aborta a caminhada imediatamente
      }

      posAtualPeEsq = map(i, 0, maxPassosPes, peEsqIni, peEsqAlvo);
      posAtualPeDir = map(i, 0, maxPassosPes, peDirIni, peDirAlvo);

      peEsquerdo.write(posAtualPeEsq);
      peDireito.write(posAtualPeDir);

      delay(10);
    }
  }

  // --- FASE 2: Movimento das Coxas ---
  int maxPassosCoxas = max(abs(coxaEsqAlvo - posAtualCoxaEsq), abs(coxaDirAlvo - posAtualCoxaDir));
  
  if (maxPassosCoxas > 0) {
    int coxaEsqIni = posAtualCoxaEsq;
    int coxaDirIni = posAtualCoxaDir;

    for (int i = 0; i <= maxPassosCoxas; i++) {
      // Checa obstáculo a cada micro-passo das coxas
      if (lerDistancia() <= 30) {
        return false; // Aborta a caminhada imediatamente
      }

      posAtualCoxaEsq = map(i, 0, maxPassosCoxas, coxaEsqIni, coxaEsqAlvo);
      posAtualCoxaDir = map(i, 0, maxPassosCoxas, coxaDirIni, coxaDirAlvo);

      coxaEsquerda.write(posAtualCoxaEsq);
      coxaDireita.write(posAtualCoxaDir);

      delay(10);
    }
  }

  return true; // Passo concluído com sucesso sem obstáculos
}
