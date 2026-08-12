/*
      Robô Seguidor de Linha - 3 Sensores (Corrigido)
*/

// ================= CONFIGURAÇÃO DE HARDWARE =================
// Se o robô andar para trás ao dar comando de ir para frente, mude para true
const bool INVERTER_MOTORES = false;

// Motor Direita
#define pino_enableDIR   5
#define pino_motorDIR_A  6
#define pino_motorDIR_B  7

// Motor Esquerda
#define pino_enableESQ   10
#define pino_motorESQ_A  9
#define pino_motorESQ_B  8

// Sensores
#define pino_S1 A2   // Esquerdo
#define pino_S2 A1   // Centro
#define pino_S3 A0   // Direito


// ================= VARIÁVEIS DE AJUSTE =================

int S1Val = 0, S2Val = 0, S3Val = 0;

const int velMAX = 200;
const int velMED = 150;
const int velBAIXA = 100;

// Valores calibrados do seu ambiente
const int pisoBranco = 70;
const int pisoPreto = 290;

// Calculamos o ponto médio entre o branco e o preto para maior segurança
const int limiar = (pisoBranco + pisoPreto) / 2; // Ex: (70 + 290) / 2 = 180

int ultimoMov = 0; // 0 = Frente, 1 = Esquerda, 2 = Direita


// ================= SETUP =================

void setup()
{
  pinMode(pino_enableDIR, OUTPUT);
  pinMode(pino_enableESQ, OUTPUT);

  pinMode(pino_motorDIR_A, OUTPUT);
  pinMode(pino_motorDIR_B, OUTPUT);

  pinMode(pino_motorESQ_A, OUTPUT);
  pinMode(pino_motorESQ_B, OUTPUT);

  Serial.begin(9600);
}


// ================= LOOP =================

void loop()
{
  // Leitura analógica dos sensores
  S1Val = analogRead(pino_S1);
  S2Val = analogRead(pino_S2);
  S3Val = analogRead(pino_S3);

  // Conversão para digital (1 = Preto / linha, 0 = Branco / fundo)
  bool S1 = (S1Val >= limiar);
  bool S2 = (S2Val >= limiar);
  bool S3 = (S3Val >= limiar);


  // ================= LÓGICA DE DECISÃO =================

  // [0 0 0] Perdeu a linha -> Busca usando a última direção salva
  if (!S1 && !S2 && !S3)
  {
    switch (ultimoMov)
    {
      case 0:
        moveFrente(velBAIXA);
        break;
      case 1:
        moveEsquerdaMuito(velMED);
        break;
      case 2:
        moveDireitaMuito(velMED);
        break;
    }
  }

  // [0 0 1] Linha à direita -> Curva forte para a direita
  else if (!S1 && !S2 && S3)
  {
    moveDireitaMuito(velMED);
    ultimoMov = 2;
  }

  // [0 1 0] Linha no centro -> Frente total
  else if (!S1 && S2 && !S3)
  {
    moveFrente(velMAX);
    ultimoMov = 0;
  }

  // [0 1 1] Centro + Direita na linha -> Curva suave para a direita
  else if (!S1 && S2 && S3)
  {
    moveDireitaPouco(velMED);
    ultimoMov = 2;
  }

  // [1 0 0] Linha à esquerda -> Curva forte para a esquerda
  else if (S1 && !S2 && !S3)
  {
    moveEsquerdaMuito(velMED);
    ultimoMov = 1;
  }

  // [1 0 1] Esquerda + Direita -> Cruzamento em T ou ruído
  else if (S1 && !S2 && S3)
  {
    moveFrente(velMED);
    ultimoMov = 0;
  }

  // [1 1 0] Esquerda + Centro na linha -> Curva suave para a esquerda
  else if (S1 && S2 && !S3)
  {
    moveEsquerdaPouco(velMED);
    ultimoMov = 1;
  }

  // [1 1 1] Todos na linha -> Cruzamento cheio
  else if (S1 && S2 && S3)
  {
    moveFrente(velMED);
    ultimoMov = 0;
  }

  delay(10); // Resposta rápida do ciclo
}


// =====================================================
//                  FUNÇÕES DOS MOTORES
// =====================================================

// Função auxiliar para controlar o sentido das pontes H
void acionaMotor(int pinoA, int pinoB, int pinoPWM, int velocidade, bool paraFrente)
{
  if (INVERTER_MOTORES) paraFrente = !paraFrente;

  if (velocidade == 0) {
    digitalWrite(pinoA, LOW);
    digitalWrite(pinoB, LOW);
    analogWrite(pinoPWM, 0);
  } else if (paraFrente) {
    digitalWrite(pinoA, HIGH);
    digitalWrite(pinoB, LOW);
    analogWrite(pinoPWM, velocidade);
  } else { // Para trás
    digitalWrite(pinoA, LOW);
    digitalWrite(pinoB, HIGH);
    analogWrite(pinoPWM, velocidade);
  }
}

// ---------- FRENTE ----------
void moveFrente(int vel)
{
  acionaMotor(pino_motorESQ_A, pino_motorESQ_B, pino_enableESQ, vel, true);
  acionaMotor(pino_motorDIR_A, pino_motorDIR_B, pino_enableDIR, vel, true);
}

// ---------- CURVA DIREITA LEVE ----------
void moveDireitaPouco(int vel)
{
  acionaMotor(pino_motorESQ_A, pino_motorESQ_B, pino_enableESQ, vel, true);
  acionaMotor(pino_motorDIR_A, pino_motorDIR_B, pino_enableDIR, vel / 3, true);
}

// ---------- CURVA ESQUERDA LEVE ----------
void moveEsquerdaPouco(int vel)
{
  acionaMotor(pino_motorESQ_A, pino_motorESQ_B, pino_enableESQ, vel / 3, true);
  acionaMotor(pino_motorDIR_A, pino_motorDIR_B, pino_enableDIR, vel, true);
}

// ---------- CURVA DIREITA FORTE (Giro no Próprio Eixo) ----------
void moveDireitaMuito(int vel)
{
  acionaMotor(pino_motorESQ_A, pino_motorESQ_B, pino_enableESQ, vel, true);
  acionaMotor(pino_motorDIR_A, pino_motorDIR_B, pino_enableDIR, vel * 0.8, false); // Gira para trás
}

// ---------- CURVA ESQUERDA FORTE (Giro no Próprio Eixo) ----------
void moveEsquerdaMuito(int vel)
{
  acionaMotor(pino_motorESQ_A, pino_motorESQ_B, pino_enableESQ, vel * 0.8, false); // Gira para trás
  acionaMotor(pino_motorDIR_A, pino_motorDIR_B, pino_enableDIR, vel, true);
}

// ---------- PARAR ----------
void paraCarro()
{
  acionaMotor(pino_motorESQ_A, pino_motorESQ_B, pino_enableESQ, 0, true);
  acionaMotor(pino_motorDIR_A, pino_motorDIR_B, pino_enableDIR, 0, true);
}
