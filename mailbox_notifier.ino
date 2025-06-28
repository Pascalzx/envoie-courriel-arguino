/*
 * Système de notification pour boîte postale
 * Détecte l'ouverture et la présence de colis
 * Envoie notification par email via SMTP TLS
 */

// === INCLUSIONS ET BIBLIOTHÈQUES ===
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "secrets.h"

// === CONSTANTES GPIO ===
#define REED_SWITCH_PIN 25      // Capteur d'ouverture
#define TRIG_PIN 26             // HC-SR04 Trigger
#define ECHO_PIN 27             // HC-SR04 Echo
#define BATTERY_PIN 35          // ADC batterie

// === CONSTANTES SYSTÈME ===
#define DISTANCE_THRESHOLD 15   // Seuil détection colis (cm)
#define MEASURE_COUNT 5         // Nombre de mesures pour moyenne
#define DEEP_SLEEP_TIME 300     // Réveil périodique (5 min)
#define WIFI_RETRY_MAX 10       // Tentatives connexion Wi-Fi
#define WIFI_RETRY_DELAY 1000   // Délai entre tentatives (ms)

// === STRUCTURES DE DONNÉES ===
enum SystemState {
  IDLE,
  DETECTED,
  SENT,
  SLEEP
};

struct SensorData {
  float distance;
  int batteryMv;
  time_t timestamp;
};

// === VARIABLES GLOBALES ===
SystemState currentState = IDLE;
SensorData lastReading;
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR bool packageDetected = false;

// === CONFIGURATION INITIALE ===
void setup() {
  Serial.begin(115200);
  bootCount++;
  
  // Configuration des GPIO
  pinMode(REED_SWITCH_PIN, INPUT_PULLDOWN);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  
  // Initialisation SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Erreur montage SPIFFS");
  }
  
  // Vérifier raison du réveil
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Réveil par ouverture porte");
      currentState = DETECTED;
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Réveil périodique");
      // Vérifier si colis présent
      if (measureDistance() < DISTANCE_THRESHOLD) {
        currentState = DETECTED;
      } else {
        currentState = SLEEP;
      }
      break;
    default:
      Serial.println("Premier démarrage");
      currentState = IDLE;
      break;
  }
}

// === CONNEXION WI-FI AVEC RETRY ===
bool connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < WIFI_RETRY_MAX) {
    delay(WIFI_RETRY_DELAY);
    Serial.print(".");
    retries++;
    
    // Back-off exponentiel
    if (retries > 5) {
      delay(WIFI_RETRY_DELAY * (retries - 5));
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connecté");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  
  Serial.println("\nÉchec connexion Wi-Fi");
  return false;
}

// === SYNCHRONISATION NTP ===
void syncTime() {
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  
  struct tm timeinfo;
  int retry = 0;
  while (!getLocalTime(&timeinfo) && retry < 10) {
    Serial.println("Attente NTP...");
    delay(1000);
    retry++;
  }
  
  if (retry < 10) {
    Serial.println("Heure synchronisée");
    char timeStr[50];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.println(timeStr);
  }
}

// === MESURE DISTANCE HC-SR04 ===
float measureDistance() {
  float measurements[MEASURE_COUNT];
  
  for (int i = 0; i < MEASURE_COUNT; i++) {
    // Pulse trigger
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    // Lecture echo
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    
    if (duration == 0) {
      measurements[i] = 999.0;  // Timeout
    } else {
      measurements[i] = duration * 0.034 / 2.0;  // Conversion cm
    }
    
    delay(50);  // Petit délai entre mesures
  }
  
  // Calcul moyenne
  float sum = 0;
  for (int i = 0; i < MEASURE_COUNT; i++) {
    sum += measurements[i];
  }
  
  return sum / MEASURE_COUNT;
}

// === LECTURE NIVEAU BATTERIE ===
int readBatteryLevel() {
  // Moyenne de plusieurs lectures ADC
  int readings = 0;
  for (int i = 0; i < 10; i++) {
    readings += analogRead(BATTERY_PIN);
    delay(10);
  }
  
  int adcValue = readings / 10;
  
  // Conversion en mV (pont diviseur 2:1, référence 3.3V)
  int batteryMv = (adcValue * 3300 * 2) / 4095;
  
  return batteryMv;
}

// === ENVOI EMAIL SMTP TLS ===
bool sendEmail(String subject, String body) {
  WiFiClientSecure client;
  client.setInsecure();  // Pour dev - utiliser certificat en prod
  
  Serial.println("Connexion SMTP...");
  if (!client.connect(SMTP_HOST, SMTP_PORT)) {
    Serial.println("Échec connexion SMTP");
    return false;
  }
  
  // Handshake SMTP
  if (!waitForResponse(client, "220")) return false;
  
  client.println("EHLO " + String(MAILGUN_DOMAIN));
  if (!waitForResponse(client, "250")) return false;
  
  // STARTTLS
  client.println("STARTTLS");
  if (!waitForResponse(client, "220")) return false;
  
  // Authentification
  client.println("AUTH LOGIN");
  if (!waitForResponse(client, "334")) return false;
  
  // Username encodé base64
  client.println(base64Encode(SMTP_USER));
  if (!waitForResponse(client, "334")) return false;
  
  // Password encodé base64
  client.println(base64Encode(SMTP_PASS));
  if (!waitForResponse(client, "235")) return false;
  
  // Envoi email
  client.println("MAIL FROM:<" + String(EMAIL_FROM) + ">");
  if (!waitForResponse(client, "250")) return false;
  
  client.println("RCPT TO:<" + String(EMAIL_TO) + ">");
  if (!waitForResponse(client, "250")) return false;
  
  client.println("DATA");
  if (!waitForResponse(client, "354")) return false;
  
  // En-têtes
  client.println("From: " + String(EMAIL_FROM));
  client.println("To: " + String(EMAIL_TO));
  client.println("Subject: " + subject);
  client.println("Content-Type: text/plain; charset=UTF-8");
  client.println();
  
  // Corps
  client.println(body);
  client.println(".");
  
  if (!waitForResponse(client, "250")) return false;
  
  client.println("QUIT");
  client.stop();
  
  Serial.println("Email envoyé avec succès");
  return true;
}

// === UTILITAIRES SMTP ===
bool waitForResponse(WiFiClientSecure& client, String code) {
  unsigned long timeout = millis() + 5000;
  String response = "";
  
  while (millis() < timeout) {
    if (client.available()) {
      response = client.readStringUntil('\n');
      Serial.println("SMTP: " + response);
      if (response.startsWith(code)) {
        return true;
      }
    }
  }
  return false;
}

String base64Encode(String input) {
  const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  String output = "";
  int val = 0, valb = -6;
  
  for (unsigned int i = 0; i < input.length(); i++) {
    val = (val << 8) + input[i];
    valb += 8;
    while (valb >= 0) {
      output += chars[(val >> valb) & 0x3F];
      valb -= 6;
    }
  }
  
  if (valb > -6) output += chars[((val << 8) >> (valb + 8)) & 0x3F];
  while (output.length() % 4) output += '=';
  
  return output;
}

// === JOURNALISATION SPIFFS ===
void logEvent(String event, float distance, int battery) {
  File logFile = SPIFFS.open("/log.csv", FILE_APPEND);
  if (!logFile) {
    // Créer fichier avec en-têtes si n'existe pas
    logFile = SPIFFS.open("/log.csv", FILE_WRITE);
    if (logFile) {
      logFile.println("timestamp,event,distance_cm,battery_mv");
    }
  }
  
  if (logFile) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timeStr[30];
      strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
      
      String logLine = String(timeStr) + "," + event + "," + 
                      String(distance, 1) + "," + String(battery);
      logFile.println(logLine);
    }
    logFile.close();
  }
}

// === PRÉPARATION DEEP SLEEP ===
void prepareDeepSleep() {
  // Désactiver Wi-Fi et Bluetooth
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();
  
  // Configurer GPIO en entrée pour économie
  pinMode(TRIG_PIN, INPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Réveil sur ouverture porte (niveau haut)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_25, 1);
  
  // Réveil périodique (secondes)
  esp_sleep_enable_timer_wakeup(DEEP_SLEEP_TIME * 1000000ULL);
  
  Serial.println("Entrée en deep sleep...");
  esp_deep_sleep_start();
}

// === BOUCLE PRINCIPALE ===
void loop() {
  switch (currentState) {
    case IDLE:
      // Vérifier ouverture porte
      if (digitalRead(REED_SWITCH_PIN) == HIGH) {
        currentState = DETECTED;
      } else {
        currentState = SLEEP;
      }
      break;
      
    case DETECTED:
      // Mesurer distance et batterie
      lastReading.distance = measureDistance();
      lastReading.batteryMv = readBatteryLevel();
      lastReading.timestamp = time(nullptr);
      
      Serial.print("Distance: ");
      Serial.print(lastReading.distance);
      Serial.println(" cm");
      
      Serial.print("Batterie: ");
      Serial.print(lastReading.batteryMv);
      Serial.println(" mV");
      
      // Vérifier présence colis
      if (lastReading.distance < DISTANCE_THRESHOLD) {
        if (!packageDetected) {
          packageDetected = true;
          
          // Connexion Wi-Fi et envoi email
          if (connectWiFi()) {
            syncTime();
            
            // Préparer corps email avec JSON
            StaticJsonDocument<256> doc;
            doc["distance_cm"] = lastReading.distance;
            doc["battery_mv"] = lastReading.batteryMv;
            
            struct tm timeinfo;
            if (getLocalTime(&timeinfo)) {
              char timeStr[30];
              strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S EST", &timeinfo);
              doc["timestamp"] = timeStr;
            }
            
            String jsonBody;
            serializeJsonPretty(doc, jsonBody);
            
            String emailBody = "Un colis a été détecté dans votre boîte postale.\n\n";
            emailBody += "Détails:\n" + jsonBody;
            
            if (sendEmail("Colis détecté", emailBody)) {
              currentState = SENT;
              logEvent("PACKAGE_DETECTED", lastReading.distance, lastReading.batteryMv);
            } else {
              logEvent("EMAIL_FAILED", lastReading.distance, lastReading.batteryMv);
            }
          }
        }
      } else {
        packageDetected = false;
        logEvent("DOOR_OPENED_NO_PACKAGE", lastReading.distance, lastReading.batteryMv);
      }
      
      currentState = SLEEP;
      break;
      
    case SENT:
      Serial.println("Notification envoyée");
      currentState = SLEEP;
      break;
      
    case SLEEP:
      prepareDeepSleep();
      break;
  }
  
  // Petit délai pour éviter boucle trop rapide
  delay(50);
}