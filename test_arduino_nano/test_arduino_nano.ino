// --- DEFINICIONES DE PINES ---

// Entradas Pulsadores
const int pulsador_marcha = 4; 
const int pulsador_parada = 16;

// Entradas Llaves selectoras
const int selectora_automatico = 17;

// Entradas Potenciómetros
const int pot_prensa = 34; 
const int pot_motor = 35;

// Salidas Testigos
const int testigo_modo = 18;

// Salidas Potencia
const int valvula_prensa = 21;
const int motor_y_testigo = 19; // <-- Pin 19 unificado para motor y luz de marcha

// --- VARIABLES MÁQUINA DE ESTADOS ---
int pasoActual = 0; // 0=Reposo, 1=Espera, 2=Prensa, 3=Motor
unsigned long tiempoAnterior = 0;
volatile bool parada = true;
bool modoAutomatico = false;
unsigned long tiempoParpadeo = 0;
bool estadoModo = false;

// Variables para los tiempos
unsigned long tiempo_espera =  1000; // 5 segundos fijos
unsigned long tiempo_prensa = 1000;
unsigned long tiempo_motor = 1000;

void setup() {
  Serial.begin(115200);

  // Configuración de Entradas
  pinMode(pulsador_marcha, INPUT);
  pinMode(pulsador_parada, INPUT);
  pinMode(selectora_automatico, INPUT);
  pinMode(pot_prensa, INPUT);
  pinMode(pot_motor, INPUT);

  // Configuración de Salidas
  pinMode(testigo_modo, OUTPUT);
  pinMode(valvula_prensa, OUTPUT);
  pinMode(motor_y_testigo, OUTPUT);

  // Iniciar apagado
  digitalWrite(testigo_modo, HIGH);
  digitalWrite(valvula_prensa, HIGH);
  digitalWrite(motor_y_testigo, HIGH);
}

void loop() {
  Serial.print("Marcha:");
  Serial.print(digitalRead(pulsador_marcha));
  Serial.print(",");
  Serial.print("Parada:");
  Serial.print(digitalRead(pulsador_parada));
  Serial.print(",");
  Serial.print("Selectora:");
  Serial.print(digitalRead(selectora_automatico));
  Serial.print(",");
  Serial.print(",");
  Serial.print("Paso:");
  Serial.println(pasoActual);

  unsigned long tiempoActual = millis();

  // --- 2. LECTURA DE MODO AUTOMÁTICO ---
  if (digitalRead(selectora_automatico) != modoAutomatico) {
    parada = true;
    pasoActual = 0;
    digitalWrite(valvula_prensa, HIGH);
    digitalWrite(motor_y_testigo, HIGH);
    modoAutomatico = digitalRead(selectora_automatico);
    digitalWrite(testigo_modo, modoAutomatico ? LOW : HIGH);
  }

  if (pasoActual != 0 && modoAutomatico) {
    if (tiempoActual - tiempoParpadeo >= 200) {
      tiempoParpadeo = tiempoActual;
      estadoModo = !estadoModo;
      analogWrite(testigo_modo, estadoModo ? 255: 0);
    }
  }
  else {
    analogWrite(testigo_modo, modoAutomatico ? 255 : 0);
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
    if (tiempoActual - tiempoAnterior >= tiempo_espera) {
      digitalWrite(valvula_prensa, LOW); 
      
      pasoActual = 2;
      tiempoAnterior = tiempoActual; 
    }
  }

  // ESTADO 2: Avance de Prensa
  else if (pasoActual == 2) {
    if (tiempoActual - tiempoAnterior >= tiempo_prensa) {
      digitalWrite(valvula_prensa, HIGH); 
      digitalWrite(motor_y_testigo, LOW); // Encendemos el motor y la luz al mismo tiempo
      
      pasoActual = 3;
      tiempoAnterior = tiempoActual; 
    }
  }

  // ESTADO 3: Avance del Motor
  else if (pasoActual == 3) {
    if (tiempoActual - tiempoAnterior >= tiempo_motor) {
      digitalWrite(motor_y_testigo, HIGH); // Apagamos el motor y la luz
      
      pasoActual = 0; 
    }
  }
}