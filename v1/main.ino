// ===============================
// CAMBIO SECUENCIAL NEUMÁTICO + DISPLAY 7 SEG (ÁNODO COMÚN)
// ADAPTADO PARA ESP32 REAL Y WOKWI
// ===============================

// --- Pines de botones ---
const int botonSubir = 2;    
const int botonBajar = 4;    
const int botonExtra = 13;   

// --- Pines de salidas ---
const int embrague = 25;        
const int valvulaSubir = 26;    
const int valvulaBajar = 27;    
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
int marchaActual = 0; // 0 = neutro

// --- Tabla de segmentos para números 0–6 ---
const byte numeros[7] = {
  0b00111111, // 0 -> Neutral
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101  // 6
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
  bool subir = !digitalRead(botonSubir);
  bool bajar = !digitalRead(botonBajar);
  bool extra = !digitalRead(botonExtra);
  unsigned long ahora = millis();

  if (!secuenciaActiva && (ahora - ultimoCambio > tiempoEntreCambios)) {

    if (subir && marchaActual >= 0 && marchaActual < 6) {
      ejecutarCambio(valvulaSubir, +1);
    }
    else if (bajar) {
      if (marchaActual == 0) {
        ejecutarCambio(valvulaBajar, +1);
      } else if (marchaActual > 1) {
        ejecutarCambio(valvulaBajar, -1);
      }
    }
    else if (extra && marchaActual == 1) {  // Solo pasa a neutra si esta en 1º y pulsa boton nextra
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
    digitalWrite(segmentPins[i], !bit); // ánodo común = LOW enciende
  }
}
