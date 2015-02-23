// #include <SoftwareSerial.h>

// /*****************************************************************************
// * Librerías externas
// ******************************************************************************/
// SoftwareSerial mySerial(10, 11); // RX, TX

// /*****************************************************************************
// * Tabla hardcodeada de personas con acceso
// ******************************************************************************/
// String DIEGO = "400002596448";
// String JORGE = "410003617264";
// String FRAN = "410003639582";
// String DANI = "410003620225";

// /*****************************************************************************
// * Temporizador
// ******************************************************************************/
// long interval = 1500;
// unsigned long currentMillis = 0;
// long previousMillis = 0;

// /*****************************************************************************
// * Variables
// ******************************************************************************/
// char code[14];
// int val;
// int bytesread = 0;

// void setup() {

//  // Conexión con lector RFID
//  Serial.begin(9600);

//  // Conexión con XBEE
//  mySerial.begin(9600);

//  // Pin REST del lector RFID
//  pinMode(2, OUTPUT);
//  digitalWrite(2, HIGH);

//  // Buzzer, LED verde, LED rojo
//  pinMode(11, OUTPUT);
//  pinMode(12, OUTPUT);
//  pinMode(13, OUTPUT);
// }

// void loop() {

//  // Se actualiza el temporizador
//  currentMillis = millis();

//  // Fin del temporizador
//  if (currentMillis - previousMillis > interval) {
//      digitalWrite(2, HIGH);              // Activate the RFID reader


//      if (Serial.available() > 0) {           // if data available from reader

//          if ((val = Serial.read()) == 10) {    // check for header

//              bytesread = 0;

//              while (bytesread < 14) {            // read 14 digit code
//                  if ( Serial.available() > 0) {
//                      val = Serial.read();
//                      if ((val == 10) || (val == 13)) { // if header or stop bytes before the 10 digit reading
//                          break;                        // stop reading
//                      }
//                      code[bytesread] = val;          // add the digit
//                      bytesread++;                    // ready to read next digit
//                  }
//              }
//              if (bytesread == 14) {              // if 14 digit read is complete
//                  Serial.print("TAG code is: ");    // possibly a good TAG
//                  Serial.println(code);             // print the TAG code
//              }
//              bytesread = 0;
//              digitalWrite(2, LOW);               // deactivate the RFID reader for a moment so it will not flood
//          }
//      }
//      previousMillis = millis();
//  }
// }

// // /*
// //  * Deal with a full message and determine function to call
// //  */
// // void process() {
// //  index = 0;

// //  strncpy(cmd, messageBuffer, 2);
// //  cmd[2] = '\0';
// //  strncpy(pin, messageBuffer + 2, 2);
// //  pin[2] = '\0';

// //  if (debug) {
// //      Serial.println(messageBuffer);
// //  }
// //  int cmdid = atoi(cmd);

// //  if (cmdid == 96) {
// //      strncpy(val, messageBuffer + 4, 12);
// //      val[12] = '\0';
// //  } else if (cmdid > 90) {
// //      strncpy(val, messageBuffer + 4, 2);
// //      val[2] = '\0';
// //      strncpy(aux, messageBuffer + 6, 3);
// //      aux[3] = '\0';
// //  } else {
// //      strncpy(val, messageBuffer + 4, 3);
// //      val[4] = '\0';
// //      strncpy(aux, messageBuffer + 7, 3);
// //      aux[4] = '\0';
// //  }

// //  // Serial.println(cmd);
// //  // Serial.println(pin);
// //  // Serial.println(val);
// //  // Serial.println(aux);

// //  switch (cmdid) {
// //  case 0:  sm(pin, val);              break;
// //  case 1:  dw(pin, val);              break;
// //  case 2:  dr(pin, val);              break;
// //  case 3:  aw(pin, val);              break;
// //  case 4:  ar(pin, val);              break;
// //  case 91: decide(aux);              break;
// //  case 92: heartbeat(1);             break;
// //  case 99: toggleDebug(val);         break;
// //  default:                           break;
// //  }
// // }

// // /*
// //  * Toggle debug mode
// //  */
// // void toggleDebug(char *val) {
// //  if (atoi(val) == 0) {
// //      debug = false;
// //      Serial.println("goodbye");
// //  } else {
// //      debug = true;
// //      Serial.println("hello");
// //  }
// // }

// // /*
// //  * Set pin mode
// //  */
// // void sm(char *pin, char *val) {
// //  if (debug) Serial.println("sm");
// //  int p = getPin(pin);
// //  if (p == -1) {
// //      if (debug) Serial.println("badpin");
// //      return;
// //  }
// //  if (atoi(val) == 0) {
// //      pinMode(p, OUTPUT);
// //  } else {
// //      pinMode(p, INPUT);
// //  }
// // }

// // /*
// //  * Digital write
// //  */
// // void dw(char *pin, char *val) {
// //  if (debug) Serial
// //      .println("dw");
// //  int p = getPin(pin);
// //  if (p == -1) {
// //      if (debug) Serial.println("badpin");
// //      return;
// //  }
// //  pinMode(p, OUTPUT);
// //  if (atoi(val) == 0) {
// //      digitalWrite(p, LOW);
// //  } else {
// //      digitalWrite(p, HIGH);
// //  }
// // }

// // /*
// //  * Digital read
// //  */
// // void dr(char *pin, char *val) {
// //  if (debug) Serial.println("dr");
// //  int p = getPin(pin);
// //  if (p == -1) {
// //      if (debug) Serial.println("badpin");
// //      return;
// //  }
// //  pinMode(p, INPUT);
// //  int oraw = digitalRead(p);
// //  char m[7];
// //  sprintf(m, "%02d::%02d", p, oraw);
// //  Serial.println(m);
// // }

// // /*
// //  * Analog read
// //  */
// // void ar(char *pin, char *val) {
// //  if (debug) Serial.println("ar");
// //  int p = getPin(pin);
// //  if (p == -1) {
// //      if (debug) Serial.println("badpin");
// //      return;
// //  }
// //  pinMode(p, INPUT); // don't want to sw
// //  int rval = analogRead(p);
// //  char m[8];
// //  sprintf(m, "%s::%03d", pin, rval);
// //  Serial.println(m);
// // }

// // void aw(char *pin, char *val) {
// //  if (debug) Serial.println("aw");
// //  int p = getPin(pin);
// //  pinMode(p, OUTPUT);
// //  if (p == -1) {
// //      if (debug) Serial.println("badpin");
// //      return;
// //  }
// //  analogWrite(p, atoi(val));
// // }

// // int getPin(char *pin) { //Converts to A0-A5, and returns -1 on error
// //  int ret = -1;
// //  if (pin[0] == 'A' || pin[0] == 'a') {
// //      switch (pin[1]) {
// //      case '0':  ret = A0; break;
// //      case '1':  ret = A1; break;
// //      case '2':  ret = A2; break;
// //      case '3':  ret = A3; break;
// //      case '4':  ret = A4; break;
// //      case '5':  ret = A5; break;
// //      default:             break;
// //      }
// //  } else {
// //      ret = atoi(pin);
// //      if (ret == 0 && (pin[0] != '0' || pin[1] != '0')) {
// //          ret = -1;
// //      }
// //  }
// //  return ret;
// // }

// void checkID(String id_code) {

//  bool encontrado = false;

//  String id(id_code);

//  if ( id_code == FRAN ) {
//      encontrado = true;
//  }
//  if ( id_code == JORGE ) {
//      encontrado = true;
//  }
//  if ( id_code == DANI ) {
//      encontrado = true;
//  }

//  if (encontrado) {
//      success();
//  }
//  else {
//      fail();
//  }
// }

// // /*
// // * Check if the server authorised the ID.
// // */
// // void decide(char *aux) {

// //  int id = atoi(aux);

// //  if (id == 0) success();
// //  else fail();

// // }

// /*
// * Unlock the door and notify a success
// */
// void success() {

//  int greenLed = 12;
//  int buzzer = 11;

//  Serial.println("ACCESO PERMITIDO");
//  //Unlock the door

//  //TODO

//  //LED and buzzer feedback
//  digitalWrite(greenLed, LOW);
//  digitalWrite(greenLed, HIGH);
//  digitalWrite(buzzer, HIGH);
//  delay(750);
//  digitalWrite(greenLed, LOW);
//  digitalWrite(buzzer, LOW);
// }

// /*
// * Notify a fail
// */
// void fail() {

//  int redLed = 13;
//  int buzzer = 11;

//  Serial.println("ACCESO DENEGADO");

//  //LED and buzzer feedback
//  digitalWrite(redLed, HIGH);
//  digitalWrite(buzzer, HIGH);
//  delay(100);
//  digitalWrite(redLed, LOW);
//  digitalWrite(buzzer, LOW);
//  delay(100);
//  digitalWrite(redLed, HIGH);
//  digitalWrite(buzzer, HIGH);
//  delay(100);
//  digitalWrite(redLed, LOW);
//  digitalWrite(buzzer, LOW);
//  delay(100);
//  digitalWrite(redLed, HIGH);
//  digitalWrite(buzzer, HIGH);
//  delay(100);
//  digitalWrite(redLed, LOW);
//  digitalWrite(buzzer, LOW);
//  delay(100);

// }

// // void heartbeat(int new_hb) {
// //  hb = new_hb;

// //  int redLed = 13;
// //  int greenLed = 12;

// //  if (hb) {
// //      previousMillis2 = millis();
// //      digitalWrite(redLed, LOW);
// //      digitalWrite(greenLed, LOW);
// //      Serial.println("HEARTBEAT!");
// //  }
// //  else {
// //      digitalWrite(redLed, HIGH);
// //      digitalWrite(greenLed, HIGH);
// //  }
// // }

