#include <MD5.h>
#include <Adafruit_NFCShield_I2C.h>
#include <Wire.h>
#include <EEPROM.h>

#define IRQ   (2)
#define RESET (3)
#define SRX (8)
#define STX (9)

#define INIT_NORMAL 100
#define NORMAL 101
#define INIT_LEARNING 200
#define LEARNING 201
#define INIT_MASTER 400
#define MASTER 401
#define INIT_CLEAR 500
#define CLEAR 501

/*****************************************************************************
* Librerías externas
******************************************************************************/
Adafruit_NFCShield_I2C nfc(IRQ, RESET);

/*****************************************************************************
* Master TAG
******************************************************************************/
char MASTER_TAG[] = "9fafcd5cf37c6dfb25b4fa30895eb594";
char ALLOWED[16][33];
int ALLOWED_NUM;

/*****************************************************************************
* Variables globales
******************************************************************************/
int buzzer = 11;
int greenLed = 12;
int redLed = 13;
long previousMillis = 0;
long interval = 2000;
char uid[12];
char uid_add[12];
char uid_conf[12];
char *puid = uid;
char *puid_add = uid_add;
char *puid_conf = uid_conf;
int pr = 0;   // Puntero a la EEPROM para leer
int pw = 0;   // Puntero a la EEPROM para escribir
int i = 0;    // Índice de bucle
int j = 0;    // Índice de bucle
uint8_t id_count;
unsigned char *hash;
char *md5str;
long learningMillis = 0;
uint8_t *buff;
char *uid_byte;
char *tmp_uid;

int mode = 0;

void setup() {

  Serial.begin(115200);
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("No se encuentra el lector NFC");
    while (1); // halt
  }

  // Encontrado el lector NFC, se imprimen los datos
  Serial.print("Encontrado PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  // Se ajusta el número máximo de reintentos de lectura
  nfc.setPassiveActivationRetries(1);

  // Se configura para leer TAGs RFID
  nfc.SAMConfig();

  // Buzzer, LED verde, LED rojo
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
}

void loop() {
  state();
}

void state() {
  switch (mode) {
  case INIT_NORMAL: // Inicia normal
    init_normal();
    mode = NORMAL;
    break;
  case NORMAL: // Normal
    mode_normal();
    break;
  case INIT_LEARNING: // Inicia aprendizaje
    init_learning();
    mode = LEARNING;
    break;
  case LEARNING: // Aprendizaje 1
    mode_learning_feedback();
    mode_learning();
    break;
  case INIT_MASTER:
    init_master();
    mode = MASTER;
    break;
  case MASTER:
    mode_master_feedback();
    mode_master();
    break;
  case INIT_CLEAR:
    init_clear();
    mode = CLEAR;
    break;
  case CLEAR:
    mode_clear_feedback();
    mode_clear();
    break;
  default:
    mode = INIT_NORMAL;
  }
}

void init_learning() {}

void init_normal() {
  int pr = 0;   // Puntero a la EEPROM para leer
  int i = 0;
  int j = 0;
  ALLOWED_NUM = 0;

  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, HIGH);

  Serial.println("Actualizando IDs...");

  ALLOWED_NUM = EEPROM.read(pr); pr++;

  Serial.print("Hay "); Serial.print(ALLOWED_NUM); Serial.print(" IDs autorizadas.");
  Serial.println("");

  for (i = 0 ; i < ALLOWED_NUM ; i++) {
    for (j = 0 ; j < 32 ; j++) {
      ALLOWED[i][j] = EEPROM.read(pr); pr++;
    }
    ALLOWED[i][32] = '\0';
  }
}

void mode_normal() {

  if (readID(&puid) > 0) {
    Serial.print("UID: "); Serial.println(uid);
    checkID(uid);
  }
}

uint8_t readID(char **r_uid) {

  boolean success;
  uint8_t uidLength;
  int ret = 0;
  int i = 0;

  if (millis() - previousMillis > interval) {

    buff = (uint8_t *) calloc(12, sizeof(uint8_t));

    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &buff[0], &uidLength);

    if (success && uidLength == 4) {    // Si se ha leído correctamente una UID de 4 bytes
      uid_byte = (char *) calloc(4, sizeof(char));
      tmp_uid = (char *) calloc(12, sizeof(char));
      for (i = 0 ; i < 4 ; i++) {     // Pasamos los 4 bytes a una cadena en HEX

        snprintf(uid_byte, 3 , "%02X", buff[i]);
        strncat(tmp_uid, uid_byte, 2);
      }

      strncpy(*r_uid, tmp_uid, 8);
      free(uid_byte);
      free(tmp_uid);
      ret = 1;
      previousMillis = millis();
    } else {
      ret = 0;
    }

    free(buff);
  }

  return ret;
}

void checkID(char *c_uid) {

  int found = 0;
  int i = 0;

  unsigned char *hash = MD5::make_hash(c_uid);
  char *md5str = MD5::make_digest(hash, 16);

  if (!strncmp(md5str, MASTER_TAG, 32)) {
    found = 100;
  } else {
    for (i = 0 ; i < ALLOWED_NUM ; i++) {

      if (!strncmp(md5str, ALLOWED[i], 32)) {
        found = 200;
      }
    }
  }
  free(hash);
  free(md5str);

  switch (found) {
  case 200:
    success();
    break;
  case 100:
    mode = INIT_MASTER;
    break;
  default:
    fail();
  }
}

void success() {

  Serial.println("ACCESO PERMITIDO");

  digitalWrite(greenLed, LOW);
  digitalWrite(buzzer, LOW);
  delay(750);
  digitalWrite(greenLed, HIGH);
  digitalWrite(buzzer, HIGH);
}

/*
* Notify a fail
*/
void fail() {

  Serial.println("ACCESO DENEGADO");

  digitalWrite(redLed, LOW);
  digitalWrite(buzzer, LOW);
  delay(100);
  digitalWrite(redLed, HIGH);
  digitalWrite(buzzer, HIGH);

  delay (75);

  digitalWrite(redLed, LOW);
  digitalWrite(buzzer, LOW);
  delay(100);
  digitalWrite(redLed, HIGH);
  digitalWrite(buzzer, HIGH);

  delay (75);

  digitalWrite(redLed, LOW);
  digitalWrite(buzzer, LOW);
  delay(100);
  digitalWrite(redLed, HIGH);
  digitalWrite(buzzer, HIGH);

}

void mode_learning_feedback() {

  if (millis () - learningMillis > 0 && millis() - learningMillis < 250) {
    digitalWrite(greenLed, LOW);
    digitalWrite(redLed, HIGH);
  }

  if (millis () - learningMillis > 250 && millis() - learningMillis < 500) {
    digitalWrite(greenLed, LOW);
    digitalWrite(redLed, LOW);
  }

  if (millis() - learningMillis > 500) {
    learningMillis = millis();
  }
}

void mode_learning() {

  pw = 0;   // Puntero a la EEPROM para escribir
  i = 0;    // Índice de bucle

  if (readID(&puid_conf) > 0) { // Se vuelve a leer para confirmar
    if (!strncmp(uid_add, uid_conf, 8)) { // Si coincide se calcula el MD5

      pw = (id_count * 32 ) + 1 ;
      id_count++;
      EEPROM.write(0, id_count);

      for (i = 0 ; i < 32 ; i++) {    // Se almacena el MD5 en EEPROM
        EEPROM.write(pw, md5str[i]); pw++;
      }

      Serial.println("Añadido TAG:");
      Serial.print("UID: "); Serial.print(uid_conf);
      Serial.println("");
      Serial.print("MD5: "); Serial.print(md5str);
      Serial.println("");
      digitalWrite(redLed, HIGH);
      digitalWrite(greenLed, LOW);
      digitalWrite(buzzer, LOW);
      delay(500);
    } else { // Si no coincide se cancela la operación
      Serial.println("Operación cancelada");
      digitalWrite(redLed, LOW);
      digitalWrite(greenLed, HIGH);
      digitalWrite(buzzer, LOW);
      delay(500);
    }
    mode = INIT_NORMAL;
  }
}

void init_master() {

  pr = 0;   // Puntero a la EEPROM para leer
  pw = 0;   // Puntero a la EEPROM para escribir
  i = 0;    // Índice de bucle
  j = 0;    // Índice de bucle

  char *id = (char *) calloc(33, sizeof(char));     // MD5 del TAG a almacenar

  Serial.println("MODO MASTER");
  Serial.println("");

  // El primer byte de la EEPROM indica el número de claves autorizadas.
  id_count = EEPROM.read(pr); pr++;

  Serial.print("Hay "); Serial.print(id_count); Serial.print(" IDs autorizadas.");
  Serial.println("");

  for (j = 0 ; j < id_count ; j++) {
    for (i = 0 ; i < 32 ; i++) {
      id[i] = EEPROM.read(pr); pr++;
    }
    id[32] = '\0';
    Serial.print("MD5: "); Serial.print(id);
    Serial.println("");
  }

}

void mode_master_feedback() {

  if (millis () - learningMillis > 0 && millis() - learningMillis < 250) {
    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, HIGH);
  }

  if (millis () - learningMillis > 250 && millis() - learningMillis < 500) {
    digitalWrite(greenLed, LOW);
    digitalWrite(redLed, LOW);
  }

  if (millis() - learningMillis > 500) {
    learningMillis = millis();
  }
}

void mode_master() {

  if (readID(&puid_add) > 0) {  // Se lee el TAG por primera vez

    hash = MD5::make_hash(uid_add);
    md5str = MD5::make_digest(hash, 16);

    if (!strcmp(md5str, MASTER_TAG)) { // Si es el MASTER TAG se prepara para borrar EEPROM

      // Master TAG: Limpieza de EEPROM
      mode = INIT_CLEAR;

    } else {

      // TAG normal, se añade a la lista de TAGs autorizados

      if (id_count < 16) {
        Serial.print("Leído: "); Serial.println(uid_add);
        Serial.println("Vuelve a colocarlo para añadir este TAG.");
        mode = INIT_LEARNING; // Modo aprendizaje
      } else {
        Serial.println("Ḿáximo número de IDs alcanzado.");
        mode = INIT_NORMAL; // Vuelve a modo normal
      }
    }
  }
}

void init_clear() {
  Serial.println("Vuelve a colocar el MASTER TAG para borrar todos los TAGs");
}

void mode_clear_feedback() {

  if (millis () - learningMillis > 0 && millis() - learningMillis < 500) {
    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, HIGH);
  }

  if (millis () - learningMillis > 500 && millis() - learningMillis < 1000) {
    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, LOW);
  }

  if (millis() - learningMillis > 1000) {
    learningMillis = millis();
  }
}

void mode_clear() {

  if (readID(&puid_conf) > 0) { // Se vuelve a leer para confirmar
    if (!strncmp(uid_add, uid_conf, 8)) { // Limpiamos EEPROM
      Serial.println("Limpiando EEPROM...");

      for (int i = 0; i < 512; i++)
        EEPROM.write(i, 0);

      Serial.println("¡EEPROM Borrada!");
      mode = INIT_NORMAL;
    } else {
      Serial.println("Operación cancelada");
      mode = INIT_NORMAL;
    }
  }
}