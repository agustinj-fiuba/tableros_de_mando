// DEFINICIONES PINES

// Entradas Pulsadores
const int pulsador_marcha = 4; 
const int pulsador_parada = 16;
const int pulsador_emergencia = 5;
// Entradas Llaves selectoras
const int selectora_automatico = 17;
// Entradas Potenciometros
const int pot_prensa = 34;
const int pot_motor = 35;

// Salidas Testigos
const int testigo_modo = 18;

// Salidas Potencia
const int valvula_prensa =  21;
const int testigo_y_motor = 19;

// TIEMPOS (en ms)
int tiempo_motor = 1000;
int tiempo_prensa = 1000;
unsigned long startTime = 0;

// VARIABLES ESTADO
bool marcha = false; 
bool automatico = false;
volatile bool emergencia = false;
int pasoActual = 0; 

// Lógica de Interrupción por Emergencia
void IRAM_ATTR CambioEmergencia() {
  if (digitalRead(pulsador_emergencia) == HIGH) {
    emergencia = true;
  } else {
    emergencia = false; 
  }
}


void setup() {
  // Comunicacion
  Serial.begin(115200);

  // Entradas
  pinMode(pulsador_marcha, INPUT);
  pinMode(pulsador_parada, INPUT);
  pinMode(pulsador_emergencia, INPUT);
  pinMode(selectora_automatico, INPUT);
  pinMode(pot_prensa, INPUT);
  pinMode(pot_motor, INPUT);

  //Salidas
  pinMode(testigo_modo, OUTPUT);
  pinMode(testigo_marcha, OUTPUT); 
  pinMode(valvula_prensa, OUTPUT);

  // Adjuntar Interrupción por Emergencia
  attachInterrupt(digitalPinToInterrupt(pulsador_emergencia), CambioEmergencia, CHANGE);
}

void loop() {
  // Lectura Estados de los Botones
  bool current_selectora_automatico = digitalRead(selectora_automatico);
  bool current_pulsador_marcha = digitalRead(pulsador_marcha);
  bool current_pulsador_parada = digitalRead(pulsador_parada);
  bool current_pulsador_emergencia = digitalRead(pulsador_emergencia);
  bool current_pot_prensa = digitalRead(pot_prensa);
  bool current_pot_motor = digitalRead(pot_motor);

  if (emergencia) { 
    Serial.println("Controles bloqueados.");
    delay(500); 
    return; 
  }

  // Etapas Proceso


}
