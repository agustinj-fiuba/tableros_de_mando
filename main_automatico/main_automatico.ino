// PROGRAMA - MODO AUTOMATICO

// --- DEFINICIONES DE PINES ---

// Entradas Pulsadores
const int pulsador_marcha = 4; 
const int pulsador_parada = 16;
const int pulsador_emergencia = 5;

// Entradas Llave Selectora
const int selectora_automatico = 17;

// Entradas Potenciómetros
const int pot_prensa = 34; 
const int pot_motor = 35;

// Salidas Testigos
const int testigo_modo = 18;

// Salidas Potencia
const int valvula_prensa = 21;
const int motor_y_testigo = 19;

// --- VARIABLES ---

// Variables Máquina de Estados
volatile bool emergenciaActivada = false;
int pasoActual = 0; // 0=Reposo, 1=Espera, 2=Prensa, 3=Espera, 4=Motor
unsigned long tiempoAnterior = 0;
volatile bool parada = true;
bool modoAutomatico = false;
unsigned long tiempoParpadeo = 0;
bool estadoModo = false;

// Variables para los tiempos
unsigned long tiempo_espera_1 =  2000; // ajustar en código
unsigned long tiempo_prensa = 1000; // ajustar con potenciometro
unsigned long tiempo_motor = 1000; // ajustar con potenciometro
unsigned long tiempo_espera_2 = 2000; // ajustar en código
unsigned long tiempo_luz = 1000; // ajustar en código

// Variables del proceso
unsigned long estampas_completadas = 0; 

// --- INTERRUPCIÓN DE EMERGENCIA ---
void IRAM_ATTR manejarEmergencia() {
  if (digitalRead(pulsador_emergencia) == LOW) {
    emergenciaActivada = true; 
    
    // APAGADO CRÍTICO
    digitalWrite(valvula_prensa, HIGH); // HIGH es False, LOW es True
    digitalWrite(motor_y_testigo, HIGH);
  } else {
    emergenciaActivada = false; 
  }
}

// Setup de los Pines
void setup() {

  // Inicializar
  Serial.begin(115200);

  // Configuración de Entradas
  pinMode(pulsador_marcha, INPUT);
  pinMode(pulsador_parada, INPUT);
  pinMode(pulsador_emergencia, INPUT);
  pinMode(selectora_automatico, INPUT);
  pinMode(pot_prensa, INPUT);
  pinMode(pot_motor, INPUT);

  // Configuración de Salidas
  pinMode(testigo_modo, OUTPUT);
  pinMode(valvula_prensa, OUTPUT);
  pinMode(motor_y_testigo, OUTPUT);

  // Iniciar todo en False
  digitalWrite(testigo_modo, HIGH);
  digitalWrite(valvula_prensa, HIGH);
  digitalWrite(motor_y_testigo, HIGH);

  // Adjuntar interrupción
  attachInterrupt(digitalPinToInterrupt(pulsador_emergencia), manejarEmergencia, CHANGE);
}

// Loop
void loop() {
  
  // Monitoreo Serial
  Serial.print("Marcha:");
  Serial.print(digitalRead(pulsador_marcha));
  Serial.print(",");
  Serial.print("Parada:");
  Serial.print(digitalRead(pulsador_parada));
  Serial.print(",");
  Serial.print("Selectora:");
  Serial.print(digitalRead(selectora_automatico));
  Serial.print(",");
  Serial.print("Emergencia:");
  Serial.print(digitalRead(pulsador_emergencia));
  Serial.print(",");
  Serial.print("Paso:");
  Serial.print(pasoActual);
  Serial.print(",");
  Serial.print("t_motor:");
  Serial.print(tiempo_motor);
  Serial.print(",");
  Serial.print("t_prensa:");
  Serial.print(tiempo_prensa);
  Serial.print(",");
  Serial.print("Estampas:");
  Serial.println(estampas_completadas);

  // Determino el tiempo
  unsigned long tiempoActual = millis();

  // --- 1. GESTIÓN DE EMERGENCIA ---
  if (emergenciaActivada) {
    digitalWrite(valvula_prensa, HIGH);
    digitalWrite(motor_y_testigo, HIGH);
    pasoActual = 0; 
    parada = true;
    delay(500); 
    return; 
  }

  // --- 2. LECTURA DE MODO AUTOMÁTICO ---
  if (digitalRead(selectora_automatico) == HIGH) {
    parada = true;
    pasoActual = 0;
    digitalWrite(valvula_prensa, HIGH);
    digitalWrite(motor_y_testigo, HIGH);
    modoAutomatico = false;
    analogWrite(testigo_modo, modoAutomatico ? 0 : 255);
  }
  else if (pasoActual == 0) {
    modoAutomatico = true;
    analogWrite(testigo_modo, modoAutomatico ? 0 : 255);
  }

  // Gestionar parpadeo
  if (pasoActual != 0 && modoAutomatico) {
    if (tiempoActual - tiempoParpadeo >= tiempo_luz) {
      tiempoParpadeo = tiempoActual;
      estadoModo = !estadoModo;
      analogWrite(testigo_modo, estadoModo ? 255: 0);
    }
  }
  else {
    analogWrite(testigo_modo, modoAutomatico ? 0 : 255);
  }

  // --- 3. LECTURA DE PARADA NORMAL ---
  if (digitalRead(pulsador_parada) == HIGH) {
    parada = true;
    delay(200);
  }

  // --- 4. MÁQUINA DE ESTADOS ---
  
  // ESTADO 0: Reposo
  if (pasoActual == 0) {
    if ((digitalRead(pulsador_marcha) == LOW && modoAutomatico) || (parada == false && modoAutomatico)) {
      parada = false;
      tiempo_prensa = map(analogRead(pot_prensa), 0, 4095, 1000, 5000);
      tiempo_motor = map(analogRead(pot_motor), 0, 4095, 1000, 5000);
      
      pasoActual = 1;              
      tiempoAnterior = tiempoActual;   
      delay(200); 
    }
  }

  // ESTADO 1: Tiempo de Carga/Espera
  else if (pasoActual == 1) {
    if (tiempoActual - tiempoAnterior >= tiempo_espera_1) {
      digitalWrite(valvula_prensa, LOW); 
      
      pasoActual = 2;
      tiempoAnterior = tiempoActual; 
    }
  }

  // ESTADO 2: Avance de Prensa
  else if (pasoActual == 2) {
    if (tiempoActual - tiempoAnterior >= tiempo_prensa) {
      digitalWrite(valvula_prensa, HIGH); 
      
      pasoActual = 3;
      tiempoAnterior = tiempoActual; 
    }
  }

  // ESTADO 3: Espera prensa/motor
  else if (pasoActual == 3) {
    if (tiempoActual - tiempoAnterior >= tiempo_espera_2) {
      digitalWrite(motor_y_testigo, LOW); // Encendemos el motor y la luz al mismo tiempo
      
      pasoActual = 4;
      tiempoAnterior = tiempoActual; 
    }
  }

  // ESTADO 4: Avance del Motor
  else if (pasoActual == 4) {
    if (tiempoActual - tiempoAnterior >= tiempo_motor) {
      digitalWrite(motor_y_testigo, HIGH); // Apagamos el motor y la luz
      estampas_completadas += 1;
      pasoActual = 0; 
    }
  }
}