// Definição da Classe Led

/*
class Led {
  private:
    int pino; // Atributo privado para guardar o número do pino

  public:
    // Construtor: recebe o pino ao criar o objeto
    Led(int numeroPino) {
      pino = numeroPino;
    }

    // Método para configurar o pino como saída
    void iniciar() {
      pinMode(pino, OUTPUT);
    }

    // Método para acender o LED
    void acender() {
      digitalWrite(pino, HIGH);
      Serial.println("Alô Mundo! LED Aceso.");
    }

    // Método para apagar o LED
    void apagar() {
      digitalWrite(pino, LOW);
      Serial.println("LED Apagado.");
    }
};

// Instanciação: cria o objeto 'meuLed' conectado ao pino 17
Led meuLed(17);

void setup() {
  // Inicia a comunicação serial
  Serial.begin(115200);
  
  // Chama o método de inicialização do objeto
  meuLed.iniciar();
}

void loop() {
  meuLed.acender(); // Acende o LED e imprime a mensagem
  delay(5000);      // Espera 1 segundo

  meuLed.apagar();  // Apaga o LED e imprime a mensagem
  delay(5000);      // Espera 1 segundo
}


*/

/*
  ============================================================
  VELOCÍMETRO DE RUÍDO - ESP32 + LED + WI-FI + API
  ============================================================
  O que este programa faz, em uma frase:
  a cada 5 segundos o LED acende, a placa sorteia um número de
  0 a 120 (o "ruído"), manda esse número pela internet para um
  site (a API) e o LED apaga.

  Três "bate-papos" explicam cada linha:
    [ELETRICO] -> o que acontece com a eletricidade e a placa
    [LED]      -> o que acontece com a luzinha
    [CODER]    -> o que a linha de código significa
  ============================================================
*/

// ============================================================
// BIBLIOTECAS (caixas de ferramentas prontas)
// ============================================================

#include <WiFi.h>              // [CODER] Traz as ferramentas para conectar no Wi-Fi. [ELETRICO] A ESP32 tem um minirrádio dentro dela; esta biblioteca liga e controla esse rádio.
#include <WiFiClientSecure.h>  // [CODER] Traz as ferramentas para conversar de forma segura (HTTPS, o "cadeadinho" dos sites). [ELETRICO] Os dados viajam embaralhados pelo ar.
#include <HTTPClient.h>        // [CODER] Traz as ferramentas para "visitar" um site e entregar um recado, igual um navegador faz, mas sem tela.
#include <time.h>              // [CODER] Traz as ferramentas para trabalhar com data e hora. [ELETRICO] A placa não tem relógio de pilha; ela precisa perguntar a hora para a internet.

#include <math.h>

// ============================================================
// CONFIGURAÇÕES (os dados que você pode mudar)
// ============================================================

const char* WIFI_SSID  = "F-30";        // [CODER] Guarda o NOME da rede Wi-Fi (o hotspot do celular). "const" = não muda durante o programa. [ELETRICO] É o nome que aparece na lista de redes do celular.
const char* WIFI_SENHA = "Domo@1234";   // [CODER] Guarda a SENHA do Wi-Fi. Troque pela sua. Não compartilhe este arquivo com a senha preenchida!
const char* URL_API    = "https://hushwave-lab-radar.lovable.app/api/public/insereruido"; // [CODER] O endereço do site que vai receber o ruído. Funciona como o "endereço da caixa de correio" onde a placa deixa o recado.
const char* COD_LAB    = "F-29";        // [CODER] O código do laboratório que está medindo. Vai junto no recado para a API saber de onde veio.

const unsigned long INTERVALO_ENVIO = 5000; // [CODER] O tempo de espera entre um envio e outro, em milissegundos (1000 ms = 1 segundo, então 5000 = 5 segundos). "unsigned long" é um número grande, sem negativo, ideal para contar tempo.

// ============================================================
// CLASSE LED (o "molde" de uma luzinha)
// ============================================================
// [CODER] Uma classe é como a planta de uma casa: descreve como
// uma coisa é e o que ela sabe fazer. Depois você constrói
// quantas casas (objetos) quiser a partir dessa planta.
// ============================================================

class Led {                       // [CODER] Começa a planta chamada "Led". Tudo entre as chaves { } abaixo pertence a ela.
  private:                        // [CODER] "private" = área reservada. Só a própria classe mexe no que está aqui dentro.
    int pino;                     // [CODER] Guarda um número inteiro: qual pino da placa o LED usa. [ELETRICO] Pino é a "perninha" metálica da placa por onde sai a eletricidade. [LED] É o pino ligado na perna do LED (através de um resistor).

  public:                         // [CODER] "public" = área aberta. O resto do programa pode usar o que está aqui.
    Led(int numeroPino) { pino = numeroPino; } // [CODER] CONSTRUTOR: roda uma vez, quando o LED é criado. Recebe o número do pino e o guarda na caixinha "pino". Pense em colar a etiqueta "este LED mora no pino X".

    void iniciar() { pinMode(pino, OUTPUT); }  // [CODER] Prepara o pino. "void" = só faz o trabalho, não devolve resposta. [ELETRICO] OUTPUT = SAÍDA: o pino vai ENVIAR eletricidade (e não ler). É como configurar uma torneira para soltar água. [LED] Sem esta linha o LED nunca acende direito.

    void acender() { digitalWrite(pino, HIGH); } // [CODER] Manda o pino para o nível alto. [ELETRICO] HIGH = o pino solta 3,3 volts (a "pressão" máxima de eletricidade desta placa). A corrente passa pelo resistor e pelo LED até o fio terra (GND). [LED] Corrente passando = o LED ACENDE.

    void apagar() { digitalWrite(pino, LOW); }   // [CODER] Manda o pino para o nível baixo. [ELETRICO] LOW = 0 volt; sem "pressão", a corrente para de passar. [LED] Sem corrente = o LED APAGA.
};                                // [CODER] Fecha a planta Led. O ponto e vírgula depois da chave é obrigatório em classes.

class Microfone {
  private:
    int pinoAnalogico;
    float offsetCalibracao; // Ajuste fino conforme seu módulo e ambiente

  public:
    Microfone(int pino, float offset = 45.0) {
      pinoAnalogico = pino;
      offsetCalibracao = offset;
    }

    void iniciar() {
      pinMode(pinoAnalogico, INPUT);
    }

    int lerDecibeis(int amostras) {
      if (amostras <= 0) {
        return 0;
      }

      int menorValor = 4095;
      int maiorValor = 0;

      for (int i = 0; i < amostras; i++) {
        int leitura = analogRead(pinoAnalogico);
        if (leitura < menorValor) {
          menorValor = leitura;
        }
        if (leitura > maiorValor) {
          maiorValor = leitura;
        }
      }

      int amplitude = maiorValor - menorValor;
      if (amplitude <= 1) {
        return 0;
      }

      // Converte amplitude em escala logarítmica (dB relativo/estimado)
      float db = 20.0 * log10((float)amplitude) + offsetCalibracao;

      if (db < 0.0) {
        db = 0.0;
      }

      return (int)round(db);
    }
};

Microfone meuMic(10); // Instanciação do microfone no pino 10

int gerarRuido() {
  return meuMic.lerDecibeis(20);
}

// ============================================================
// CLASSE ALIMENTAAPI (o "carteiro" que leva o ruído até o site)
// ============================================================

class AlimentaApi {               // [CODER] Começa a planta chamada "AlimentaApi". Ela sabe conectar no Wi-Fi, saber a hora, sortear o ruído e entregar o recado ao site.
  private:                        // [CODER] Área reservada: ferramentas internas do carteiro que o resto do programa não precisa ver.
    const char* url;              // [CODER] Caixinha para guardar o endereço do site.
    const char* codigoLab;        // [CODER] Caixinha para guardar o código do laboratório.
    int ultimoValor;              // [CODER] Caixinha para guardar o último ruído sorteado (número inteiro).

    int gerarRuido() {            // [CODER] Função que SORTEIA e DEVOLVE um número inteiro ("int" no começo diz que ela devolve um número).
      return ::gerarRuido();
      //return random(0, 121);      // [CODER] Sorteia de 0 até 120. O limite de cima (121) nunca sai, por isso usamos 121 para o 120 poder aparecer. "return" entrega o número sorteado para quem chamou a função. [ELETRICO] Aqui é só uma simulação; depois dá para trocar por um microfone de verdade.
    }                             // [CODER] Fecha a função gerarRuido.

    void conectarWifi() {         // [CODER] Função que conecta a placa no Wi-Fi.
      if (WiFi.status() == WL_CONNECTED) return; // [CODER] "Se JÁ estiver conectado, volte e não faça nada." WL_CONNECTED quer dizer "conectado". [ELETRICO] Evita religar o rádio sem necessidade.

      Serial.print("Conectando ao Wi-Fi"); // [CODER] Escreve o texto no Monitor Serial (a "telinha de conversa" no computador). "print" escreve sem pular linha. [ELETRICO] A placa fala com o computador pelo cabo USB.
      WiFi.begin(WIFI_SSID, WIFI_SENHA);   // [CODER] Manda a placa procurar a rede pelo nome e entrar com a senha. [ELETRICO] O minirrádio da ESP32 "bate na porta" do hotspot do celular e pede para entrar.
      while (WiFi.status() != WL_CONNECTED) { // [CODER] "ENQUANTO ainda não estiver conectado, repita o que está entre as chaves." (!= significa "diferente de")
        delay(500);               // [CODER] Espera meio segundo (500 ms) antes de olhar de novo. [ELETRICO] A placa fica "esperando a porta abrir".
        Serial.print(".");        // [CODER] Escreve um pontinho a cada tentativa, para você ver que está tentando.
      }                           // [CODER] Fecha o "enquanto". Ao sair daqui, a placa está conectada.
      Serial.println("\nWi-Fi conectado!"); // [CODER] Avisa que deu certo. "println" escreve e pula para a próxima linha. O "\n" pula uma linha antes do texto.
    }                             // [CODER] Fecha a função conectarWifi.

    // Sincroniza o relógio via NTP (Brasília = UTC-3, sem horário de verão)
    void sincronizarHorario() {   // [CODER] Função que pergunta a hora certa para a internet. [ELETRICO] A placa não tem bateria de relógio: quando desliga, esquece que horas são.
      configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // [CODER] Diz à placa: "o fuso é -3 horas (Brasília; 3600 segundos = 1 hora), sem horário de verão (0), e pergunte a hora nestes dois servidores". NTP é o serviço da internet que informa a hora exata.

      struct tm agora;            // [CODER] Cria uma "ficha de data e hora" chamada "agora" (com dia, mês, ano, hora, minuto, segundo). "tm" é o formato padrão dessa ficha.
      Serial.print("Sincronizando horario"); // [CODER] Avisa no Monitor Serial que está buscando a hora.
      while (!getLocalTime(&agora, 1000)) { // [CODER] "Enquanto NÃO conseguir preencher a ficha com a hora (esperando até 1 segundo por tentativa), repita." O "!" significa "não".
        Serial.print(".");        // [CODER] Escreve um pontinho a cada tentativa.
      }                           // [CODER] Fecha o "enquanto". Ao sair, a placa já sabe a hora.
      Serial.println("\nHorario sincronizado!"); // [CODER] Avisa que o relógio da placa está certo.
    }                             // [CODER] Fecha a função sincronizarHorario.

    // Preenche data e hora nos formatos que a API espera
    bool obterDataHora(char* data, char* hora) { // [CODER] Função que pega a data e a hora atuais e escreve nos textos "data" e "hora". Devolve "true" (verdadeiro) se deu certo e "false" (falso) se falhou. "bool" = verdadeiro ou falso.
      struct tm agora;            // [CODER] Cria a ficha de data e hora.
      if (!getLocalTime(&agora, 2000)) return false; // [CODER] Tenta preencher a ficha (espera até 2 segundos). Se NÃO conseguir, devolve "false" e sai da função.

      strftime(data, 11, "%Y-%m-%d", &agora); // [CODER] Escreve a data em texto no formato ANO-MÊS-DIA, como a API quer (ex.: 2026-11-18). %Y = ano, %m = mês, %d = dia. O 11 é o tamanho máximo do texto (10 letras + 1 de fechamento).
      strftime(hora, 9,  "%H:%M:%S", &agora); // [CODER] Escreve a hora em texto no formato HORA:MINUTO:SEGUNDO (ex.: 14:46:00). O 9 é o tamanho máximo (8 letras + 1 de fechamento).
      return true;                // [CODER] Devolve "true": deu tudo certo.
    }                             // [CODER] Fecha a função obterDataHora.

  public:                         // [CODER] Área aberta: o que o resto do programa pode chamar.
    AlimentaApi(const char* endereco, const char* codigo) { // [CODER] CONSTRUTOR: roda quando o carteiro é criado; recebe o endereço do site e o código do laboratório.
      url = endereco;             // [CODER] Guarda o endereço na caixinha "url".
      codigoLab = codigo;         // [CODER] Guarda o código na caixinha "codigoLab".
      ultimoValor = 0;            // [CODER] Começa com o último ruído igual a zero.
    }                             // [CODER] Fecha o construtor.

    void iniciar() {              // [CODER] Prepara o carteiro; é chamada uma vez, no começo do programa.
      randomSeed(esp_random());   // [CODER] Dá uma "semente" ao sorteio para os números não repetirem a mesma ordem toda vez que a placa liga. [ELETRICO] esp_random() usa ruído elétrico do próprio chip, que é imprevisível, então o sorteio fica bem aleatório.
      conectarWifi();             // [CODER] Chama a função que conecta no Wi-Fi.
      sincronizarHorario();       // [CODER] Chama a função que acerta o relógio (só funciona depois de conectar, porque precisa da internet).
    }                             // [CODER] Fecha a função iniciar.

    bool enviar() {               // [CODER] A função principal do carteiro: sorteia o ruído e entrega ao site. Devolve "true" se o site aceitou.
      conectarWifi();             // [CODER] Antes de enviar, confere se o Wi-Fi ainda está conectado (se caiu, reconecta).

      char data[11];              // [CODER] Reserva espaço para o texto da data (10 letras + 1 de fechamento).
      char hora[9];               // [CODER] Reserva espaço para o texto da hora (8 letras + 1 de fechamento).
      if (!obterDataHora(data, hora)) { // [CODER] Pede a data e a hora. "Se NÃO conseguiu..."
        Serial.println("Falha ao obter data/hora. Envio cancelado."); // [CODER] ...avisa o problema no Monitor Serial...
        return false;             // [CODER] ...e desiste deste envio, devolvendo "false".
      }                           // [CODER] Fecha o "se".

      ultimoValor = gerarRuido(); // [CODER] Sorteia o ruído (0 a 120) e guarda na caixinha "ultimoValor".

      // Monta o JSON no formato exato da API
      String json = "{";          // [CODER] Cria um texto chamado "json" começando com "{". JSON é o jeito organizado de escrever um recado para computadores: como um formulário com campos e valores. A chave abre o formulário.
      json += "\"codigo_laboratorio\":\"" + String(codigoLab) + "\","; // [CODER] Acrescenta o campo do laboratório. O "+=" significa "cole isto no final do texto". As barras \" servem para escrever aspas dentro do texto. Fica: "codigo_laboratorio":"F-29",
      json += "\"ruido\":" + String(ultimoValor) + ",";               // [CODER] Acrescenta o campo do ruído, que é número (sem aspas). Fica: "ruido":87,
      json += "\"data\":\"" + String(data) + "\",";                   // [CODER] Acrescenta o campo da data. Fica: "data":"2026-11-18",
      json += "\"hora\":\"" + String(hora) + "\"";                    // [CODER] Acrescenta o campo da hora (o último campo não leva vírgula depois). Fica: "hora":"14:46:00"
      json += "}";                // [CODER] Fecha o formulário com a chave final. O recado está pronto!

      WiFiClientSecure cliente;   // [CODER] Cria o "telefone seguro" (HTTPS) que a placa usa para ligar para o site. [ELETRICO] Os dados saem pelo minirrádio, passam pelo celular e seguem para a internet.
      cliente.setInsecure();      // [CODER] Diz ao telefone para NÃO conferir a identidade do site. Simplifica o teste e a aula; em um produto de verdade o ideal é conferir o certificado.

      HTTPClient http;            // [CODER] Cria o "carteiro de navegador" que sabe entregar recados a sites.
      http.begin(cliente, url);   // [CODER] Diz ao carteiro: "use este telefone seguro para ir neste endereço".
      http.addHeader("Content-Type", "application/json"); // [CODER] Cola uma etiqueta no recado: "o conteúdo é JSON". Assim o site sabe como ler.

      int codigo = http.POST(json); // [CODER] ENTREGA o recado ao site (POST = "estou te mandando um dado novo"). O site responde com um número de status (200 = deu certo, 400 = recado errado, 500 = o site com problema). Esse número fica na caixinha "codigo". Aqui a placa espera a resposta, e por isso o LED fica aceso um tempinho.

      Serial.println("Enviado: " + json); // [CODER] Mostra no Monitor Serial exatamente o que foi enviado.
      Serial.print("HTTP: ");      // [CODER] Escreve "HTTP: " sem pular linha...
      Serial.println(codigo);      // [CODER] ...e em seguida o número de status que o site devolveu.

      if (codigo > 0) {           // [CODER] Se o número for maior que zero, é porque o site respondeu (números negativos indicam falha de conexão).
        Serial.println(http.getString()); // [CODER] Mostra o texto da resposta do site.
      }                           // [CODER] Fecha o "se".

      http.end();                 // [CODER] Encerra a ligação e libera a memória da placa. [ELETRICO] Como desligar o telefone depois da conversa.
      return (codigo >= 200 && codigo < 300); // [CODER] Devolve "true" se o status estiver entre 200 e 299 (família do "deu certo"); caso contrário, "false". O "&&" significa "E": as duas condições precisam ser verdadeiras.
    }                             // [CODER] Fecha a função enviar.

    int getUltimoValor() { return ultimoValor; } // [CODER] Função simples que devolve o último ruído sorteado, para o resto do programa poder consultar se quiser.
};                                // [CODER] Fecha a planta AlimentaApi.

// ============================================================
// OBJETOS (as "casas" construídas a partir das plantas)
// ============================================================

Led meuLed(17);                   // [CODER] Constrói um LED chamado "meuLed" ligado ao pino 17 (GPIO 17). [ELETRICO] GPIO = pino de uso geral da placa. Ligue assim: pino 17 -> resistor (220 a 330 ohms) -> perna LONGA do LED; perna CURTA do LED -> GND. O resistor protege o LED para não queimar com corrente demais. [LED] A perna longa é o positivo (ânodo).

AlimentaApi api(URL_API, COD_LAB); // [CODER] Constrói o carteiro "api", entregando a ele o endereço do site e o código do laboratório.

unsigned long ultimoEnvio = 0;    // [CODER] Caixinha que guarda o MOMENTO (em milissegundos desde que a placa ligou) do último envio. Serve como um cronômetro.

// ============================================================
// SETUP (roda UMA vez, quando a placa liga)
// ============================================================

void setup() {                    // [CODER] A função setup é obrigatória: a placa executa o que está aqui uma única vez, ao ligar ou ao apertar o botão RESET.
  Serial.begin(115200);           // [CODER] Liga a conversa com o computador a 115200 bits por segundo. O Monitor Serial precisa estar na MESMA velocidade, senão aparecem símbolos estranhos. [ELETRICO] É a "velocidade da fala" no cabo USB.
  meuMic.iniciar();
  
  meuLed.iniciar();               // [CODER] Prepara o pino 17 como saída. [LED] A luzinha fica pronta para ser comandada.
  api.iniciar();                  // [CODER] Prepara o carteiro: sorteio, Wi-Fi e relógio. Pode demorar alguns segundos até conectar.
  ultimoEnvio = millis() - INTERVALO_ENVIO; // [CODER] millis() diz quantos milissegundos se passaram desde que a placa ligou. Descontando o intervalo, o cronômetro "já venceu", e o primeiro envio sai imediatamente no loop, sem esperar 5 s.
}                                 // [CODER] Fecha o setup.

// ============================================================
// LOOP (roda para SEMPRE, sem parar)
// ============================================================

void loop() {                     // [CODER] A função loop também é obrigatória: quando termina, a placa a executa de novo, e de novo, infinitamente. [ELETRICO] É o "coração" que bate o tempo todo.
  if (millis() - ultimoEnvio >= INTERVALO_ENVIO) { // [CODER] Pergunta: "já passaram 5 segundos desde o último envio?" Agora menos o momento do último envio = quanto tempo se passou. Se for verdade, entra e executa o que está entre as chaves; se não, pula e volta ao começo do loop.

    meuLed.acender();             // [CODER] Passo 1: acende o LED. [ELETRICO] O pino 17 sobe para 3,3 V e a corrente passa. [LED] A luzinha ACENDE, avisando "estou enviando".
    api.enviar();                 // [CODER] Passo 2: sorteia o ruído, monta o recado e entrega ao site. [LED] Enquanto isso (1 a 3 segundos), o LED continua aceso.
    meuLed.apagar();              // [CODER] Passo 3: o envio terminou, então apaga o LED. [ELETRICO] O pino 17 volta a 0 V. [LED] A luzinha APAGA e fica apagada durante a espera.

    ultimoEnvio = millis();       // [CODER] Passo 4: anota o momento atual como "último envio", e o cronômetro dos 5 segundos recomeça AGORA, depois do envio. Por isso o LED fica realmente 5 segundos apagado.
  }                               // [CODER] Fecha o "se". Se ainda não deu 5 segundos, a placa só repete o loop (muito rápido) até chegar a hora. Não usamos delay() porque ele "congelaria" a placa; com millis() ela continua livre para outras tarefas.

  int ruido = gerarRuido();
  Serial.print("Ruído: ");
  Serial.print(ruido);
  Serial.println(" dB");
  delay(1000);      // Espera 1 segundo

}                                 // [CODER] Fecha o loop. A placa volta ao início dele e repete o ciclo: acende -> envia -> apaga -> espera 5 s -> acende...
