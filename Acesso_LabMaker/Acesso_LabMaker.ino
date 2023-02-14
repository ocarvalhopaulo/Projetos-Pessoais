#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <ESP32Servo.h>
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_AS5 932

#define SS_PIN        5  // ESP32 pin GIOP5 
#define RST_PIN      17 // ESP32 pin GIOP27 
#define RELAY_PIN    22 // ESP32 pin GIOP22
#define linhas        4 
#define colunas       4 
#define btn_saida    16 
#define led_verde    21 
#define led_vermelho  4
#define buzzer       15
#define speaker       2
#define sino          3

const char* ssid = "FabLab";
const char* password = "fablab082";
byte tag_1[4] = {0xEE, 0x04, 0xB6, 0xAB}; // Tag - Paulo
byte tag_2[4] = {0xDD, 0xA4, 0x24, 0x20}; // Tag - Carlos  
byte pinos_linhas[linhas]      = {26, 25, 33, 32}; 
byte pinos_colunas[colunas] = {13, 12, 14, 27};  
const String senha = "7890"; 
String input_senha;
int lastState = LOW;  
int currentState;  
int lastState2 = LOW;  
int currentState2; 

char keys[linhas][colunas] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

Keypad keypad = Keypad( makeKeymap(keys), pinos_linhas, pinos_colunas, linhas, colunas );
WiFiServer server(80);
MFRC522 rfid(SS_PIN, RST_PIN);
 
void setup() {
  Serial.begin(115200);  
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi conectada.");
  Serial.println("Endereço de IP: ");
  Serial.println(WiFi.localIP());
  server.begin();

  SPI.begin(); 
  rfid.PCD_Init(); // inicia o modulo RFID
  pinMode(RELAY_PIN, OUTPUT); 
  pinMode(led_verde, OUTPUT); 
  pinMode(led_vermelho, OUTPUT); 
  pinMode(buzzer, OUTPUT); 
  pinMode(btn_saida, INPUT_PULLUP);
  pinMode(speaker, OUTPUT); 
  pinMode(sino, INPUT_PULLUP);
  input_senha.reserve(32);
}
 
void loop() {

saida();
servidor();
teclado();
campanhia();

 if (rfid.PICC_IsNewCardPresent()) { // nova tag disponivel
    if (rfid.PICC_ReadCardSerial()) { // ID lido
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

      if (rfid.uid.uidByte[0] == tag_1[0] &&
          rfid.uid.uidByte[1] == tag_1[1] &&
          rfid.uid.uidByte[2] == tag_1[2] &&
          rfid.uid.uidByte[3] == tag_1[3] ) {
        Serial.println("Bem vindo Paulo!");
        abertura(); 
      }
      else
      if (rfid.uid.uidByte[0] == tag_2[0] &&
          rfid.uid.uidByte[1] == tag_2[1] &&
          rfid.uid.uidByte[2] == tag_2[2] &&
          rfid.uid.uidByte[3] == tag_2[3] ) {
        Serial.println("Bem vindo Carlos!");
        abertura(); 
      }
      else
      {
        alerta();
        Serial.print("Acesso negado para:");
        for (int i = 0; i < rfid.uid.size; i++) {
          Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
          Serial.print(rfid.uid.uidByte[i], HEX);
        }
        Serial.println();
      }

      rfid.PICC_HaltA(); 
      rfid.PCD_StopCrypto1(); 
    }
  }
} 

void abertura(){

digitalWrite(RELAY_PIN, HIGH); 
digitalWrite(led_verde, HIGH);
tone(buzzer, NOTE_G5);
delay(5000);
digitalWrite(RELAY_PIN, LOW);
digitalWrite(led_verde, LOW);
noTone(buzzer);
}

void saida(){

currentState = digitalRead(btn_saida);

  if (lastState == HIGH && currentState == LOW){
    digitalWrite(RELAY_PIN, HIGH); 
  }else if (lastState == LOW && currentState == HIGH){
    digitalWrite(RELAY_PIN, LOW);
  }
  lastState = currentState;
}

void alerta(){
   
  digitalWrite(led_vermelho, HIGH);
  tone(buzzer, NOTE_F5);
  delay(500);
  tone(buzzer, NOTE_E5);
  delay(500);  
  noTone(buzzer);
  delay(2000);  
  digitalWrite(led_vermelho, LOW);
  
}

void campanhia(){

currentState2 = digitalRead(sino);

  if (lastState2 == HIGH && currentState2 == LOW){
    tone(speaker, NOTE_AS5);
    delay(500);
    tone(speaker, NOTE_G5);
    delay(500);     
  }else if (lastState2 == LOW && currentState2 == HIGH){
    noTone(speaker);
  }
  lastState2 = currentState2;
  }

void teclado(){

 char key = keypad.getKey();

  if (key) {
    if (key == '*') {
      input_senha = ""; // clear input password
    } else if (key == 'D') {
      if (senha == input_senha) {
        Serial.println("Acesso Liberado!");
        abertura();
      } else {
        alerta();
        Serial.println("Acesso Negado!");
      }

      input_senha = "";
    } else {
      input_senha += key; 
    }
  }  
}

void servidor(){

WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print("Clique <a href=\"/H\">here</a> para abrir a porta. <br>");            
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c; 
        }
        if (currentLine.endsWith("GET /H")) {
           abertura(); 
        }
      }
    }
    client.stop();
    Serial.println("Client Disconnected.");
  }  
}
 
