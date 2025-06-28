/*
 * Notificateur de boîte postale - Version Arduino
 * Compatible Arduino Uno/Nano + ESP8266 WiFi
 */

#include <SoftwareSerial.h>
#include <LowPower.h>
#include <EEPROM.h>
#include <avr/wdt.h>

// === CONFIGURATION PINS ===
#define REED_PIN 2          // INT0 pour réveil
#define TRIG_PIN 3         
#define ECHO_PIN 4         
#define BATTERY_PIN A0     
#define ESP_RX 5           
#define ESP_TX 6           
#define ESP_RESET 7        
#define LED_PIN 13         

// === CONSTANTES ===
#define DISTANCE_SEUIL 15
#define WIFI_TIMEOUT 30000
#define MEASURE_COUNT 5

// === CONFIGURATION (à adapter) ===
const char* WIFI_SSID = "VotreWiFi";
const char* WIFI_PASS = "VotrePassword";
const char* EMAIL_TO = "pascal@example.com";
const char* SMTP_SERVER = "smtp.mailgun.org";
const char* SMTP_USER = "postmaster@domain.mailgun.org";
const char* SMTP_PASS = "votrecle";

// === VARIABLES ===
SoftwareSerial esp(ESP_RX, ESP_TX);
volatile bool porteOuverte = false;
float derniereDistance = 0;
int batterieMv = 0;
bool colisDetecte = false;

// === INTERRUPTION ===
void wakeUp() {
  porteOuverte = true;
}

void setup() {
  wdt_disable();
  
  Serial.begin(9600);
  esp.begin(9600);
  
  pinMode(REED_PIN, INPUT_PULLUP);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(ESP_RESET, OUTPUT);
  
  digitalWrite(ESP_RESET, HIGH);
  attachInterrupt(0, wakeUp, RISING);
  
  Serial.println(F("Demarrage..."));
  resetESP();
}

void loop() {
  if (porteOuverte) {
    porteOuverte = false;
    digitalWrite(LED_PIN, HIGH);
    
    // Mesurer distance
    derniereDistance = measureDistance();
    batterieMv = readBattery();
    
    Serial.print(F("Distance: "));
    Serial.print(derniereDistance);
    Serial.println(F(" cm"));
    
    if (derniereDistance < DISTANCE_SEUIL && !colisDetecte) {
      colisDetecte = true;
      
      if (connectWiFi()) {
        String msg = "Colis detecte!\n";
        msg += "Distance: " + String(derniereDistance, 1) + " cm\n";
        msg += "Batterie: " + String(batterieMv) + " mV\n";
        
        if (sendEmail("Colis dans boite postale", msg)) {
          Serial.println(F("Email envoye"));
          logEvent('D');  // Detecte
        } else {
          logEvent('E');  // Erreur
        }
        disconnectWiFi();
      }
    } else if (derniereDistance >= DISTANCE_SEUIL) {
      colisDetecte = false;
      logEvent('O');  // Ouvert sans colis
    }
    
    digitalWrite(LED_PIN, LOW);
  }
  
  // Deep sleep 8s
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}

// === MESURE DISTANCE ===
float measureDistance() {
  float total = 0;
  
  for (int i = 0; i < MEASURE_COUNT; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    float distance = duration * 0.034 / 2.0;
    
    if (distance > 0 && distance < 400) {
      total += distance;
    } else {
      total += 999;
    }
    delay(50);
  }
  
  return total / MEASURE_COUNT;
}

// === LECTURE BATTERIE ===
int readBattery() {
  long total = 0;
  for (int i = 0; i < 10; i++) {
    total += analogRead(BATTERY_PIN);
    delay(10);
  }
  // Conversion mV (pont diviseur, ref 5V)
  return (total / 10) * 5000L / 1024L * 2;
}

// === GESTION ESP8266 ===
void resetESP() {
  digitalWrite(ESP_RESET, LOW);
  delay(100);
  digitalWrite(ESP_RESET, HIGH);
  delay(3000);
  
  sendATCommand("AT+RST", 3000);
  sendATCommand("AT+CWMODE=1", 1000);
  sendATCommand("AT+CIPMUX=0", 1000);
}

bool connectWiFi() {
  String cmd = "AT+CWJAP=\"";
  cmd += WIFI_SSID;
  cmd += "\",\"";
  cmd += WIFI_PASS;
  cmd += "\"";
  
  if (sendATCommand(cmd, 10000)) {
    return sendATCommand("AT+CIFSR", 2000);
  }
  return false;
}

void disconnectWiFi() {
  sendATCommand("AT+CWQAP", 1000);
}

bool sendEmail(String subject, String body) {
  // Connexion SMTP
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += SMTP_SERVER;
  cmd += "\",587";
  
  if (!sendATCommand(cmd, 5000)) return false;
  
  // Commandes SMTP simplifiées
  if (!sendSMTPData("EHLO arduino")) return false;
  if (!sendSMTPData("STARTTLS")) return false;
  
  // Auth (nécessite base64)
  if (!sendSMTPData("AUTH LOGIN")) return false;
  if (!sendSMTPData(SMTP_USER)) return false;  // À encoder
  if (!sendSMTPData(SMTP_PASS)) return false;  // À encoder
  
  // Email
  cmd = "MAIL FROM:<arduino@mailbox.local>";
  if (!sendSMTPData(cmd)) return false;
  
  cmd = "RCPT TO:<";
  cmd += EMAIL_TO;
  cmd += ">";
  if (!sendSMTPData(cmd)) return false;
  
  if (!sendSMTPData("DATA")) return false;
  
  // Corps
  sendSMTPData("Subject: " + subject);
  sendSMTPData("");
  sendSMTPData(body);
  sendSMTPData(".");
  
  sendSMTPData("QUIT");
  sendATCommand("AT+CIPCLOSE", 1000);
  
  return true;
}

bool sendSMTPData(String data) {
  String cmd = "AT+CIPSEND=";
  cmd += String(data.length() + 2);
  
  esp.println(cmd);
  delay(100);
  
  if (esp.find(">")) {
    esp.println(data);
    delay(500);
    return true;
  }
  return false;
}

bool sendATCommand(String cmd, int timeout) {
  esp.println(cmd);
  
  long start = millis();
  while (millis() - start < timeout) {
    if (esp.find("OK")) {
      return true;
    }
  }
  return false;
}

// === JOURNALISATION EEPROM ===
void logEvent(char type) {
  static int addr = 0;
  
  // Structure: [Type][Battery_H][Battery_L][Distance]
  EEPROM.write(addr++, type);
  EEPROM.write(addr++, batterieMv >> 8);
  EEPROM.write(addr++, batterieMv & 0xFF);
  EEPROM.write(addr++, (byte)derniereDistance);
  
  if (addr >= 1020) addr = 0;  // Boucler
}