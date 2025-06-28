# Notificateur de Boîte Postale Arduino

Système de détection de colis pour boîte postale avec notification email.

## Matériel
- Arduino Uno/Nano
- Module WiFi ESP8266 (ESP-01)
- Capteur HC-SR04 (ultrason)
- Reed switch + résistance 10kΩ
- Pont diviseur pour batterie (2x 10kΩ)
- LED indicateur

## Connexions

```
Arduino D2 (INT0) → Reed Switch → GND
Arduino D3 → HC-SR04 Trig
Arduino D4 → HC-SR04 Echo  
Arduino D5 → ESP8266 RX
Arduino D6 → ESP8266 TX
Arduino D7 → ESP8266 Reset
Arduino A0 → Pont diviseur batterie
```

## Installation

1. **Librairies requises**
   - LowPower
   - SoftwareSerial (incluse)

2. **Configuration WiFi**
   ```cpp
   const char* WIFI_SSID = "VotreWiFi";
   const char* WIFI_PASS = "MotDePasse";
   ```

3. **Configuration Email**
   - Créer compte Mailgun
   - Remplacer SMTP_USER et SMTP_PASS

## Utilisation

1. **Calibration distance**
   - Mesurer distance boîte vide
   - Ajuster DISTANCE_SEUIL (défaut 15cm)

2. **Test**
   - Ouvrir porte → LED clignote
   - Si colis détecté → email envoyé

3. **Logs**
   - Stockés dans EEPROM
   - Format: Type|Batterie|Distance

## Consommation
- Veille: ~20µA (LowPower)
- Actif: ~50mA (WiFi)
- Autonomie: 3-6 mois (2xAA)

## Dépannage
- **WiFi timeout**: Vérifier alimentation ESP8266 (3.3V)
- **Distance erronée**: Nettoyer capteur HC-SR04
- **Pas de réveil**: Vérifier pull-up sur D2

## Optimisations
- Utiliser watchdog pour reset auto
- Désactiver ADC/BOD en veille
- Limiter transmissions WiFi