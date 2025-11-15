// ===============================
// CAMBIO SECUENCIAL NEUMÁTICO + DISPLAY 7 SEG (ÁNODO COMÚN)
// Versión: 2 botones originales + 1 botón extra (solo funciona en 2ª)
// El botón extra usa una válvula/LED distinto como "segundo led"
// ===============================

// --- Pines de botones ---
const int botonSubir = 2;    // Subir marcha
const int botonBajar = 3;    // Bajar marcha
const int botonExtra = 12;   // Nuevo: solo funciona si marchaActual == 2

// --- Pines de salidas ---
const int embrague = 8;        // Embrague (común)
const int valvulaSubir = 9;    // Válvula/LED para subir marcha (segundo led original)
const int valvulaBajar = 10;   // Válvula/LED para bajar marcha
const int valvulaExtra = 11;   // Nueva válvula/LED para el tercer botón (segundo led distinto)

// --- Pines del display 7 segmentos (ánodo común) ---
// a,b,c,d,e,f,g
const int segmentPins[7] = {4, 5, 6, 7, A0, A1, A2};

// --- Tiempos (ms) ---
const unsigned long tiempoEmbrague = 250;
const unsigned long tiempoMarcha = 300;
const unsigned long tiempoDesembrague = 200;
const unsigned long tiempoEntreCambios = 800;

// --- Variables de control ---
bool secuenciaActiva = false;
unsigned long ultimoCambio = 0;
int marchaActual = 0; // 0 = neutro

// --- Tabla de segmentos para números 0–6 (bits: a,b,c,d,e,f,g) ---
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
  // Botones (pull-up interno)
  pinMode(botonSubir, INPUT_PULLUP);
  pinMode(botonBajar, INPUT_PULLUP);
  pinMode(botonExtra, INPUT_PULLUP); 

  // Salidas (válvulas / LEDs simulados)
  pinMode(embrague, OUTPUT);
  pinMode(valvulaSubir, OUTPUT);
  pinMode(valvulaBajar, OUTPUT);
  pinMode(valvulaExtra, OUTPUT);

  // Display
  for (int i = 0; i < 7; i++) pinMode(segmentPins[i], OUTPUT);

  // Inicialización salidas en LOW (válvulas apagadas)
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

    // --- Subir marcha: solo si ya estás en >=1 y <6 ---
    if (subir && marchaActual >= 1 && marchaActual < 6) {
      ejecutarCambio(valvulaSubir, +1); // usa valvulaSubir
    }

    // --- Bajar marcha ---
    else if (bajar) {
      if (marchaActual == 0) {
        // De neutro a 1ª
        ejecutarCambio(valvulaBajar, +1);
      } else if (marchaActual > 1) {
        // Bajar 1 marcha (2->1, 3->2, ...)
        ejecutarCambio(valvulaBajar, -1);
      }
    }

    // --- Botón extra: SOLO FUNCIONA SI ESTÁS EN 2ª ---
    else if (extra && marchaActual == 2) {
      // Ejecuta la secuencia usando la válvulaExtra (segundo LED distinto)
      // y actualiza la marcha como -2 (2 -> 0). Si quieres otro comportamiento,
      // cámbialo a -1 o 0 según necesidades.
      ejecutarCambio(valvulaExtra, -2);
    }
  }
}

// --- Secuencia de cambio ---
// valvulaCambio: pin de la válvula/LED que actuará como "segundo led"
// direccion: cómo cambia la marcha (ej. +1, -1, -2)
void ejecutarCambio(int valvulaCambio, int direccion) {
  secuenciaActiva = true;

  // 1) Activar embrague
  digitalWrite(embrague, HIGH);
  delay(tiempoEmbrague);

  // 2) Activar la válvula/LED secundaria (puede ser valvulaSubir, valvulaBajar o valvulaExtra)
  digitalWrite(valvulaCambio, HIGH);
  delay(tiempoMarcha);
  digitalWrite(valvulaCambio, LOW);

  // 3) Desembrague
  delay(tiempoDesembrague);
  digitalWrite(embrague, LOW);

  // 4) Actualizar marcha y display
  marchaActual += direccion;
  if (marchaActual < 0) marchaActual = 0;
  if (marchaActual > 6) marchaActual = 6;

  mostrarMarcha();

  secuenciaActiva = false;
  ultimoCambio = millis();
}

// --- Mostrar número en display (ánodo común: LOW = encendido) ---
void mostrarMarcha() {
  byte valor = numeros[marchaActual];
  for (int i = 0; i < 7; i++) {
    bool bit = (valor >> i) & 0x01;
    digitalWrite(segmentPins[i], !bit); // invertir por ánodo común
  }
}