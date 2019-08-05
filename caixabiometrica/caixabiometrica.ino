
#include <Servo.h>
#include <Adafruit_Fingerprint.h>
#if ARDUINO >= 100
 #include <SoftwareSerial.h>
#else
 #include <NewSoftSerial.h>
#endif
 
int getFingerprintIDez();
 
Servo myservo;
int pos = 0;            // Armazena a posição servo
int servoLock = 0;      // Se o servo está atualmente na posição travada ou aberta
int mytimer = 0;        // Um temporizador que é incrementado em cada loop.Desliga o dispositivo após inatividade.
 
int speakerOut = 6; // Alarme
int printID = -1;   // Variável para armazenar a impressão digital atual identificada (indexada de zero)
 
// pin #2 is IN from sensor (Fio verde)
// pin #3 is OUT from arduino  (fio branco)
#if ARDUINO >= 100
SoftwareSerial mySerial(2, 3);
#else
NewSoftSerial mySerial(2, 3);
#endif
 
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
 
void setup()
{
  Serial.begin(9600);
  Serial.println("TESTE DE IMPRESSÃO");
 
  myservo.attach(10);
  myservo.write(82);
 
  pinMode(8, INPUT);    // Controle lógico do interruptor de energia
  digitalWrite(8, LOW); // Set Low para ligar, High para desligar
 
  pinMode(speakerOut, OUTPUT);
 
  finger.begin(57600); // definir a taxa de dados para a porta serial do sensor
 
  if (finger.verifyPassword()) {
    Serial.println("Sensor de impressão digital encontrado!");
  } else {
    Serial.println("Não encontrou o sensor de impressão digital :(");
    while (1);
  }
  Serial.println("Esperando por uma impressão digital válida...");
 
  delay(500);
  myservo.detach();   // Retire o servo para economizar desgaste de energia
}
 
void loop()
{
  printID = getFingerprintIDez();
  if(printID >= 0) {
    //Impressão digital válida, agora mude a posição do servo
    myservo.attach(10);
    if(servoLock == 0) {
      myservo.write(10);
      servoLock = 1;
    } else {
      myservo.write(82);
      servoLock = 0;
    }
    delay(1000);
    myservo.detach();
    mytimer = 0;        // Reseta o temporizador, dispositivo ainda em uso
  } else if (printID == -2) {
    // Varredura ruim
    analogWrite(speakerOut,100);    // ALARME!!!!
    delay(500);
    analogWrite(speakerOut,0);
  } else if(printID == -3) {
    // Impressão inválida!
    analogWrite(speakerOut,200);    // ALARME DIFERENTE !!!
    delay(1000);
    analogWrite(speakerOut,0);
  }
 
  delay(100);
  if(mytimer >= 100) {
    // O dispositivo ficou inativo por 100 loops, desative o ajuste do interruptor Pololu.
    digitalWrite(8, HIGH);
    Serial.println("Desligado");
  } else {
    mytimer++;
  }
}
 
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Imagem tirada");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("Impressão não detectada");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("erro de comunicação");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("erro de imagem");
      return p;
    default:
      Serial.println("Erro deconhecido");
      return p;
  }
 
  // OK successo!
 
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Imagem convertida");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Imagem muito bagunçada");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Erro de comunicação");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Não foi possível encontrar recursos de impressão digital");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Não foi possível encontrar recursos de impressão digital");
      return p;
    default:
      Serial.println("Erro desconhecido");
      return p;
  }
 
  // OK convertido!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Encontrou uma correspondência de impressão!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Erro de comunicação");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Não encontrou uma correspondência");
    return p;
  } else {
    Serial.println("Erro desconhecido");
    return p;
  }
 
  // Encontrou correspondência!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
 
}
 
// retorna -1 se falhar, caso contrário, retorna ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;
 
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -2;
 
  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -3;
 
  // Encontrou correspondência!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;}
