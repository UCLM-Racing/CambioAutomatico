/*
    VERSION 3
    AUTOR: UCLM RACING TEAM
    CAMBIO SECUENCIAL NEUMÁTICO + DISPLAY 7 SEG (ÁNODO COMÚN)
    ADAPTADO PARA ESP32 REAL Y WOKWI
    PATRÓN: 1ª - N - 2ª - 3ª - 4ª - 5ª - 6ª
    POSICIÓN INICIAL: NEUTRO (N)
    SE PUEDE ACTIVAR UN MODO "SIN EMBRAGUE" AL SUBIR DE MARCHA GRACIAS A UN
    SWITCH ADICIONAL. AL BAJAR DE MARCHA SIEMPRE SE USA EMBRAGUE.
*/

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

// --- Pin de control del modo sin embrague ---
const int modoSinEmbrague = 16;
const int ledModo = 4; // LED indicador del modo sin embrague
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

// --- Tabla de segmentos para números 0–6 ---
// HE TENIDO PROBLEMAS CON EL WOKWI; REVISAR ESTO
// Formato: gfedcba (bit 0 = a, bit 6 = g) -> ÁNODO COMÚN
const byte numeros[7] = {
  0b0000110, // 0: "1" (1ª marcha) -> segmentos b,c
  0b0111111, // 1: "0" (Neutro) -> segmentos a,b,c,d,e,f
  0b1011011, // 2: "2" (2ª marcha) -> segmentos a,b,d,e,g
  0b1001111, // 3: "3" (3ª marcha) -> segmentos a,b,c,d,g
  0b1100110, // 4: "4" (4ª marcha) -> segmentos b,c,f,g
  0b1101101, // 5: "5" (5ª marcha) -> segmentos a,c,d,f,g
  0b1111101  // 6: "6" (6ª marcha) -> segmentos a,c,d,e,f,g
};

void ejecutarCambio(int valvulaCambio, int direccion);
void ejecutarCambioSinEmbrague(int valvulaCambio, int direccion);
void mostrarMarcha();

void setup() {
  pinMode(botonSubir, INPUT_PULLUP);
  pinMode(botonBajar, INPUT_PULLUP);
  pinMode(botonExtra, INPUT_PULLUP);
  pinMode(modoSinEmbrague, INPUT_PULLUP);
  
  pinMode(embrague, OUTPUT);
  pinMode(valvulaSubir, OUTPUT);
  pinMode(valvulaBajar, OUTPUT);
  pinMode(valvulaExtra, OUTPUT);
  pinMode(ledModo, OUTPUT);
  
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
  bool sinEmbrague = !digitalRead(modoSinEmbrague);
  digitalWrite(ledModo, sinEmbrague ? HIGH : LOW); // Indicador modo sin embrague -> Feedback con un LED
  
  unsigned long ahora = millis();
  
  // Esto es una barrera para evitar múltiples cambios en una misma pulsación 
  // Si ahora - ultimoCambio > tiempoEntreCambios, se permite un nuevo cambio => ahora (x) - ultimoCambio (y) > tiempoEntreCambios (800) => x - y > 800
  // En caso contrario se ignora la pulsación
  if (!secuenciaActiva && (ahora - ultimoCambio > tiempoEntreCambios)) {
    
    // BOTÓN SUBIR: Sube de manera secuencial las marchas (1ª-6ª)
    if (subir) {
      // NOTA: 0 ES PRIMERA
      if (marchaActual == 0) {
        if(sinEmbrague){
          // Desde 1ª salta directamente a 2ª (saltando el neutro) sin embrague
          ejecutarCambioSinEmbrague(valvulaSubir, +2);
        } else {
          // Desde 1ª salta directamente a 2ª (saltando el neutro)
          ejecutarCambio(valvulaSubir, +2);
        }
      }
      else if(marchaActual == 1){
        // Desde neutro a 1ª marcha
        ejecutarCambio(valvulaBajar, -1);
      }
      else if (marchaActual > 1 && marchaActual < 6) {
        if(sinEmbrague){
          // Desde neutro o 2ª-5ª avanza normalmente sin embrague
          ejecutarCambioSinEmbrague(valvulaSubir, +1);
        } else {
          // Desde neutro o 2ª-5ª avanza normalmente
          ejecutarCambio(valvulaSubir, +1);
        }
      }
    }
    
    // BOTÓN BAJAR: Baja de manera secuencial las marchas (6ª-1ª)
    else if (bajar) {
      if(marchaActual == 1){
        // Si estamos en neutral no podemos "bajar" a primera (aunque realmente se haga así)
        return;
      }
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

/*
  Función para ejecutar el cambio de marcha sin usar el embrague al subir de marcha.
  Al bajar de marcha, siempre se usa el embrague por seguridad.
*/
void ejecutarCambioSinEmbrague(int valvulaCambio, int direccion)
{
  /*
  En caso de bajar de marcha sin quitar el modo sin embrague,
  se llama a la función normal para que baje de marcha con el embrague.
  Posteriormente se sale de la función para que no se ejecute el resto del código.
  */
  if(direccion <= 0){
    ejecutarCambio(valvulaCambio, direccion);
    return;
  }

  secuenciaActiva = true;

  digitalWrite(valvulaCambio, HIGH);
  delay(tiempoMarcha);
  digitalWrite(valvulaCambio, LOW);

  marchaActual += direccion;
  if (marchaActual < 0) marchaActual = 0;
  if (marchaActual > 6) marchaActual = 6;

  mostrarMarcha();

  secuenciaActiva = false;
  ultimoCambio = millis();
}

/*
Cambiar el ánodo común o cátodo en función del tipo de display
*/
void mostrarMarcha() {
  byte valor = numeros[marchaActual];
  
  for (int i = 0; i < 7; i++) {
    bool bit = (valor >> i) & 0x01; // Lee bit i para segmento i
    digitalWrite(segmentPins[i], bit); // ánodo común = LOW enciende (!bit) / cátodo común = HIGH enciende (bit)
  }
}