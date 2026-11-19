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
const int POS_NEUTRA_COXA_ESQ = 90;
const int POS_NEUTRA_COXA_DIR = 45;

// Variáveis de controle de posição atual dos servos
int posAtualPescoco  = POS_NEUTRA_PESCOCO;
int posAtualPeEsq    = POS_NEUTRA_PE_ESQ;
int posAtualPeDir    = POS_NEUTRA_PE_DIR;
int posAtualCoxaEsq  = POS_NEUTRA_COXA_ESQ;
int posAtualCoxaDir  = POS_NEUTRA_COXA_DIR;

// Variável global de controle de direção (1 = Frente)
int caminhoLivre = 1;

// Distância mínima de um obstáculo em centímetros
int distanciaObstaculo = 20;

void setup() {
  Serial.begin(115200);

  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoEcho, INPUT);

  pescoco.attach(6);
  coxaEsquerda.attach(3);
  peEsquerdo.attach(2);
  coxaDireita.attach(5);
  peDireito.attach(4);

  // Força o envio imediato para a posição neutra ao ligar
  pescoco.write(POS_NEUTRA_PESCOCO);
  peEsquerdo.write(POS_NEUTRA_PE_ESQ);
  peDireito.write(POS_NEUTRA_PE_DIR);
  coxaEsquerda.write(POS_NEUTRA_COXA_ESQ);
  coxaDireita.write(POS_NEUTRA_COXA_DIR);
}

void loop() {
  float distancia = lerDistancia();
  float distanciaEsq = 0;
  float distanciaDir = 0;

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  if (distancia <= distanciaObstaculo && distancia > 0) {
    Serial.println("Obstáculo próximo! Parando e voltando à posição segura...");
    irParaPosicaoInicial();

    moverPescoco(125);
    distanciaEsq = lerDistancia();
    Serial.print("Distância Esquerda: ");
    Serial.println(distanciaEsq);

    moverPescoco(20);
    distanciaDir = lerDistancia();
    Serial.print("Distância Direita: ");
    Serial.println(distanciaDir);

    if(distancia <= distanciaObstaculo && distanciaEsq <= distanciaObstaculo && distanciaDir <= distanciaObstaculo){
      Serial.println("Decisão: Beco sem saída! Executando manobra de escape...");
      caminhoLivre = 7;
    }
    
    else if(distancia >= distanciaEsq && distancia >= distanciaDir){
      caminhoLivre = 1;
      Serial.println("Decisão: Continuar em Frente");
    }

    else if(distanciaDir > distanciaEsq){
      caminhoLivre = 2;
      Serial.println("Decisão: Virar à Direita");
    }

    else{
      caminhoLivre = 4;
      Serial.println("Decisão: Virar à Esquerda");
    }

    irParaPosicaoInicial();
    delay(500); // Pausa de segurança
  } 
  else {

    switch (caminhoLivre) {
      case 1:
        // Andar para FRENTE
        //       PéEsq | PéDir | CoxaEsq | CoxaDir
        if (!andarPasso(0,  10, 120, 75)) return;
        if (!andarPasso(60, 70, 60,  15))  return;
        break;

      case 2:
        // Andar para TRÁS
        //       PéEsq | PéDir | CoxaEsq | CoxaDir
        if (!andarPasso(60,  70, 120, 75)) return;
        if (!andarPasso(0,  10, 60, 15)) return;

        // Virar para DIREITA (Giro curto / Próprio eixo)
        //       PéEsq | PéDir | CoxaEsq | CoxaDir
        if (!andarPasso(0,  10, 60,  75)) return;
        if (!andarPasso(60, 70, 120, 15)) return;

        if (!andarPasso(0,  10, 60,  75)) return;
        if (!andarPasso(60, 70, 120, 15)) return;
        caminhoLivre = 1;
        break;

      case 3:
        // Virar para DIREITA (Giro longo)
        //       PéEsq | PéDir | CoxaEsq | CoxaDir
        if (!andarPasso(0,  10, 90, 75)) return;
        if (!andarPasso(60, 70, 60,  15))  return;
        caminhoLivre = 1;
        break;

      case 4:
        // Andar para TRÁS
        //       PéEsq | PéDir | CoxaEsq | CoxaDir
        if (!andarPasso(60,  70, 120, 75)) return;
        if (!andarPasso(0,  10, 60, 15)) return;

        // Virar para ESQUERDA (Giro curto / Próprio eixo)
        //       PéEsq | PéDir | CoxaEsq | CoxaDir
        if (!andarPasso(0,  10, 120, 15)) return;
        if (!andarPasso(60, 70, 60,  75)) return;

        if (!andarPasso(0,  10, 120, 15)) return;
        if (!andarPasso(60, 70, 60,  75)) return;
        caminhoLivre = 1;
        break;

      case 5:
        // Virar para ESQUERDA (Giro longo)
        //       PéEsq | PéDir | CoxaEsq | CoxaDir
        if (!andarPasso(0,  10, 120, 75)) return;
        if (!andarPasso(60, 70, 60,  45))  return;
        caminhoLivre = 1;
        break;
      
      case 6:
        // Andar para TRÁS
        //       PéEsq | PéDir | CoxaEsq | CoxaDir
        if (!andarPasso(60,  70, 120, 75)) return;
        if (!andarPasso(0,  10, 60, 15)) return;
        caminhoLivre = 1;
        break;

      case 7:
        // Andar para TRÁS
        //       PéEsq | PéDir | CoxaEsq | CoxaDir
        if (!andarPasso(60,  70, 120, 75)) return;
        if (!andarPasso(0,  10, 60, 15)) return;

        if (!andarPasso(60,  70, 120, 75)) return;
        if (!andarPasso(0,  10, 60, 15)) return;

        // Virar para ESQUERDA (Giro curto / Próprio eixo)
        //       PéEsq | PéDir | CoxaEsq | CoxaDir
        if (!andarPasso(0,  10, 120, 15)) return;
        if (!andarPasso(60, 70, 60,  75)) return;

        if (!andarPasso(0,  10, 120, 15)) return;
        if (!andarPasso(60, 70, 60,  75)) return;
        caminhoLivre = 1;
        break;
    }

  // pescoco:       Neutro = 67   |  Esquerda  = 135      |   Direita  = 10
  // peEsquerdo:    Neutro = 30   |  ParaDentro  = 60     |   ParaFora  = 0
  // peDireito:     Neutro = 40   |  ParaDentro  = 10     |   ParaFora  = 70
  // coxaEsquerda:  Neutro = 90   |  Esquerda  = 60       |   Direita  = 120
  // coxaDireita:   Neutro = 45   |  Esquerda  = 15       |   Direita  = 75
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

  long duracao = pulseIn(pinoEcho, HIGH, 20000); 
  if (duracao == 0) return 999.0;

  return (duracao * 0.034) / 2.0;
}

// Move todos os servos suavemente de onde estão até a posição neutra
void irParaPosicaoInicial() {
  int maxPassos1 = max(abs(POS_NEUTRA_PE_ESQ - posAtualPeEsq), abs(POS_NEUTRA_PE_DIR - posAtualPeDir));
  int maxPassos2 = max(abs(POS_NEUTRA_COXA_ESQ - posAtualCoxaEsq), abs(POS_NEUTRA_COXA_DIR - posAtualCoxaDir));
  int maxPassos3 = max(maxPassos1, maxPassos2);
  int maxPassos  = max(maxPassos3, abs(POS_NEUTRA_PESCOCO - posAtualPescoco)); // Inclui o pescoço no cálculo

  if (maxPassos > 0) {
    int peEsqIni   = posAtualPeEsq,   peDirIni   = posAtualPeDir;
    int coxaEsqIni = posAtualCoxaEsq, coxaDirIni = posAtualCoxaDir;
    int pescocoIni = posAtualPescoco;

    for (int i = 0; i <= maxPassos; i++) {
      posAtualPeEsq   = map(i, 0, maxPassos, peEsqIni,   POS_NEUTRA_PE_ESQ);
      posAtualPeDir   = map(i, 0, maxPassos, peDirIni,   POS_NEUTRA_PE_DIR);
      posAtualCoxaEsq = map(i, 0, maxPassos, coxaEsqIni, POS_NEUTRA_COXA_ESQ);
      posAtualCoxaDir = map(i, 0, maxPassos, coxaDirIni, POS_NEUTRA_COXA_DIR);
      posAtualPescoco = map(i, 0, maxPassos, pescocoIni, POS_NEUTRA_PESCOCO); // Atualiza o pescoço

      peEsquerdo.write(posAtualPeEsq);
      peDireito.write(posAtualPeDir);
      coxaEsquerda.write(posAtualCoxaEsq);
      coxaDireita.write(posAtualCoxaDir);
      pescoco.write(posAtualPescoco); // Envia o comando para o servo do pescoço
      
      delay(10); 
    }
  }
}

// Executa um passo partindo da posição ATUAL até a posição ALVO de forma fluida
bool andarPasso(int peEsqAlvo, int peDirAlvo, int coxaEsqAlvo, int coxaDirAlvo) {

  // --- FASE 1: Movimento dos Pés ---
  int maxPassosPes = max(abs(peEsqAlvo - posAtualPeEsq), abs(peDirAlvo - posAtualPeDir));
  
  if (maxPassosPes > 0) {
    int peEsqIni = posAtualPeEsq;
    int peDirIni = posAtualPeDir;

    for (int i = 0; i <= maxPassosPes; i++) {
      if (i % 5 == 0) { 
        if (lerDistancia() <= 30 && lerDistancia() > 0) {
          return false; 
        }
      }

      posAtualPeEsq = map(i, 0, maxPassosPes, peEsqIni, peEsqAlvo);
      posAtualPeDir = map(i, 0, maxPassosPes, peDirIni, peDirAlvo);

      peEsquerdo.write(posAtualPeEsq);
      peDireito.write(posAtualPeDir);

      delay(5); 
    }
  }

  // --- FASE 2: Movimento das Coxas ---
  int maxPassosCoxas = max(abs(coxaEsqAlvo - posAtualCoxaEsq), abs(coxaDirAlvo - posAtualCoxaDir));
  
  if (maxPassosCoxas > 0) {
    int coxaEsqIni = posAtualCoxaEsq;
    int coxaDirIni = posAtualCoxaDir;

    for (int i = 0; i <= maxPassosCoxas; i++) {
      if (i % 5 == 0) {
        if (lerDistancia() <= 30 && lerDistancia() > 0) {
          return false; 
        }
      }

      // CORREÇÃO APLICADA AQUI: Atualizando corretamente a variável global e o write da coxa direita
      posAtualCoxaEsq = map(i, 0, maxPassosCoxas, coxaEsqIni, coxaEsqAlvo);
      posAtualCoxaDir = map(i, 0, maxPassosCoxas, coxaDirIni, coxaDirAlvo);

      coxaEsquerda.write(posAtualCoxaEsq);
      coxaDireita.write(posAtualCoxaDir);

      delay(5); 
    }
  }

  return true; 
}

// Executa apenas o movimento fluido do pescoço até o ângulo alvo
bool moverPescoco(int alvo) {
  int maxPassos = abs(alvo - posAtualPescoco);
  
  if (maxPassos > 0) {
    int inicio = posAtualPescoco;

    for (int i = 0; i <= maxPassos; i++) {
      posAtualPescoco = map(i, 0, maxPassos, inicio, alvo);
      pescoco.write(posAtualPescoco);
      delay(10); // Ajuste a velocidade do pescoço aqui (menor = mais rápido)
    }
  }
  return true;
}
