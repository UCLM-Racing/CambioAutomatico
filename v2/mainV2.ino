// ===============================
// CAMBIO SECUENCIAL NEUMÁTICO + DISPLAY 7 SEG (ÁNODO COMÚN)
// ADAPTADO PARA ESP32 REAL Y WOKWI
// PATRÓN: 1ª - N - 2ª - 3ª - 4ª - 5ª - 6ª
// POSICIÓN INICIAL: NEUTRO (N)
// ===============================

// --- Pines de botones ---
const int botonSubir = 2;
const int botonBajar = 4;
const int botonExtra = 13;   // Volver a neutro desde 2ª

// --- Pines de salidas ---
const int embrague = 25;        
const int valvulaSubir = 26;    // Baja físicamente en la palanca
const int valvulaBajar = 27;    // Sube físicamente en la palanca
const int valvulaExtra = 14;    

// --- Pines del display 7 segmentos (ánodo común) ---
// a,b,c,d,e,f,g
const int segmentPins[7] = {23, 22, 21, 19, 18, 5, 17};

// --- Tiempos (ms) ---
const unsigned long tiempoEmbrague = 250;
const unsigned long tiempoMarcha = 300;
const unsigned long tiempoDesembrague = 200;
const unsigned long tiempoEntreCambios = 800;

// --- Variables de control ---
bool secuenciaActiva = false;
unsigned long ultimoCambio = 0;
int marchaActual = 1; // 1 = neutro (posición inicial)

// --- Mapeo de marchas ---
// marchaActual: 0=1ª, 1=N, 2=2ª, 3=3ª, 4=4ª, 5=5ª, 6=6ª

// --- Tabla de segmentos para display ---
const byte numeros[7] = {
  0b00000110, // 0: "1" (1ª marcha)
  0b00111111, // 1: "0" (Neutro - muestra 0)
  0b01011011, // 2: "2" (2ª marcha)
  0b01001111, // 3: "3" (3ª marcha)
  0b01100110, // 4: "4" (4ª marcha)
  0b01101101, // 5: "5" (5ª marcha)
  0b01111101  // 6: "6" (6ª marcha)
};

void setup() {
  pinMode(botonSubir, INPUT_PULLUP);
  pinMode(botonBajar, INPUT_PULLUP);
  pinMode(botonExtra, INPUT_PULLUP);
  
  pinMode(embrague, OUTPUT);
  pinMode(valvulaSubir, OUTPUT);
  pinMode(valvulaBajar, OUTPUT);
  pinMode(valvulaExtra, OUTPUT);
  
  for (int i = 0; i < 7; i++) pinMode(segmentPins[i], OUTPUT);
  
  digitalWrite(embrague, LOW);
  digitalWrite(valvulaSubir, LOW);
  digitalWrite(valvulaBajar, LOW);
  digitalWrite(valvulaExtra, LOW);
  
  mostrarMarcha();
}

void loop() {
  bool subir = !digitalRead(botonSubir);   // Baja en la secuencia
  bool bajar = !digitalRead(botonBajar);   // Sube en la secuencia
  bool extra = !digitalRead(botonExtra);
  
  unsigned long ahora = millis();
  
  if (!secuenciaActiva && (ahora - ultimoCambio > tiempoEntreCambios)) {
    
    // BOTÓN SUBIR: Baja en la secuencia
    if (subir) {
      if (marchaActual == 0) {
        // Desde 1ª salta directamente a 2ª (saltando el neutro)
        ejecutarCambio(valvulaSubir, +2);
      }
      else if (marchaActual >= 1 && marchaActual < 6) {
        // Desde neutro o 2ª-5ª avanza normalmente
        ejecutarCambio(valvulaSubir, +1);
      }
    }
    
    // BOTÓN BAJAR: Sube en la secuencia
    else if (bajar) {
      if (marchaActual == 2) {
        // Desde 2ª salta directamente a 1ª (saltando el neutro)
        ejecutarCambio(valvulaBajar, -2);
      }
      else if (marchaActual > 0) {
        // Desde neutro o 3ª-6ª retrocede normalmente
        ejecutarCambio(valvulaBajar, -1);
      }
    }
      // BOTÓN EXTRA: Volver a neutro desde 2ª marcha
      else if (extra && marchaActual == 2) {  // marchaActual==2 es 2ª marcha
      ejecutarCambio(valvulaExtra, -1);
    }
  }
}

void ejecutarCambio(int valvulaCambio, int direccion) {
  secuenciaActiva = true;
  
  digitalWrite(embrague, HIGH);
  delay(tiempoEmbrague);
  
  digitalWrite(valvulaCambio, HIGH);
  delay(tiempoMarcha);
  digitalWrite(valvulaCambio, LOW);
  
  delay(tiempoDesembrague);
  digitalWrite(embrague, LOW);
  
  marchaActual += direccion;
  if (marchaActual < 0) marchaActual = 0;
  if (marchaActual > 6) marchaActual = 6;
  
  mostrarMarcha();

  secuenciaActiva = false;
  ultimoCambio = millis();
}

void mostrarMarcha() {
  byte valor = numeros[marchaActual];
  
  for (int i = 0; i < 7; i++) {
    bool bit = (valor >> i) & 0x01;
    digitalWrite(segmentPins[i], !bit); // Ánodo común = LOW enciende
  }
}