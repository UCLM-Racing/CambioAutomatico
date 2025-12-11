// ===============================
// CAMBIO SECUENCIAL NEUMÁTICO + DISPLAY 7 SEG (ÁNODO COMÚN)
// ADAPTADO PARA ESP32 REAL Y WOKWI
// PATRÓN: 1ª - N - 2ª - 3ª - 4ª - 5ª - 6ª
// POSICIÓN INICIAL: NEUTRO (N)
// ===============================

// --- Pines de botones ---
const int botonSubir = 32;
const int botonBajar = 33;
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
  bool subir = !digitalRead(botonSubir);   // Secuencia hacia adelante
  bool bajar = !digitalRead(botonBajar);   // Secuencia hacia atrás
  bool extra = !digitalRead(botonExtra);

  unsigned long ahora = millis();

  if (!secuenciaActiva && (ahora - ultimoCambio > tiempoEntreCambios)) {

    // ===========================
    //   BOTÓN SUBIR
    // ===========================
    if (subir) {

      // Desde NEUTRA (1) siempre va a 1ª (0)
      if (marchaActual == 1) {
        ejecutarCambio(valvulaBajar, -1);  // Neutra → 1ª
      }

      // Desde 1ª va a Neutra
      else if (marchaActual == 0) {
        ejecutarCambio(valvulaSubir, +2);  // 1ª → N
      }

      // De 2ª en adelante sube normal hasta 6ª
      else if (marchaActual >= 2 && marchaActual < 6) {
        ejecutarCambio(valvulaSubir, +1);  // 2ª→3ª→4ª→5ª→6ª
      }
    }

    // ===========================
    //   BOTÓN BAJAR
    // ===========================
    else if (bajar) {

      // Solo permitir NEUTRA desde 2ª
      if (marchaActual == 2) {
        ejecutarCambio(valvulaBajar, -2);   // 2ª → N
      }

      // De 3ª-6ª retrocede normal (pero sin pasar por N)
      else if (marchaActual > 2) {
        ejecutarCambio(valvulaBajar, -1);   // 6ª→5ª→4ª→3ª→2ª
      }

      // Desde NEUTRA NO SE BAJA
      // Desde 1ª NO SE BAJA (evita N → ?)
    }

    // ===========================
    // BOTÓN EXTRA: Devolver a NEUTRO desde 2ª
    // ===========================
    else if (extra && marchaActual == 2) {
      ejecutarCambio(valvulaExtra, -1);     // 2ª → N
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
    digitalWrite(segmentPins[i], bit); // Ánodo común = LOW enciende
  }
}