# Système de notification pour boîte postale

## Description
Système IoT basé sur ESP32 qui détecte l'ouverture de la boîte postale et la présence de colis, puis envoie une notification par email.

## Matériel requis
- ESP32 DevKit
- Capteur reed switch (ouverture porte)
- Capteur ultrason HC-SR04
- Résistances : 10kΩ (pull-down), 2x 100kΩ (pont diviseur batterie)
- Alimentation batterie 3.7V Li-Po

## Schéma de câblage

```
ESP32 GPIO 25 ----[10kΩ]----GND
                |
                └---- Reed Switch ---- 3.3V

ESP32 GPIO 26 ---- HC-SR04 Trig
ESP32 GPIO 27 ---- HC-SR04 Echo
ESP32 GND ---- HC-SR04 GND
ESP32 5V ---- HC-SR04 VCC

ESP32 GPIO 35 ----[100kΩ]----[100kΩ]---- VBAT+
                |
               GND
```

## Configuration Mailgun

1. Créer compte sur [Mailgun](https://www.mailgun.com)
2. Ajouter et vérifier votre domaine
3. Obtenir les credentials SMTP :
   - Serveur : smtp.mailgun.org
   - Port : 587
   - Username : postmaster@votre-domaine.mailgun.org
   - Password : Clé API Mailgun

## Installation

1. Installer les bibliothèques Arduino :
   - ArduinoJson
   - ESP32 Core

2. Copier `secrets.h` et remplir vos informations :
   ```cpp
   #define WIFI_SSID "votre-ssid"
   #define WIFI_PASSWORD "votre-password"
   #define SMTP_USER "postmaster@domaine.mailgun.org"
   #define SMTP_PASS "votre-cle-api"
   ```

3. Téléverser le code sur l'ESP32

## Calibration HC-SR04

1. Placer le capteur au fond de la boîte postale
2. Mesurer distance typique sans colis
3. Ajuster `DISTANCE_THRESHOLD` (défaut 15 cm)
4. Tester avec différents types de colis

## Test SMTP

Pour tester l'envoi d'email sans attendre une détection :
1. Décommenter la ligne de test dans `setup()`
2. Vérifier les logs série pour debug
3. Confirmer réception dans votre boîte email

## Déploiement OTA

Pour mise à jour sans fil :
1. Utiliser ArduinoOTA ou ESP Web Tools
2. Configurer mdns : `mailbox.local`
3. Upload via réseau après première installation USB

## Consommation

- Mode veille : < 150 µA
- Mode actif (envoi email) : ~80 mA pendant 5-10s
- Autonomie estimée : 6-12 mois avec batterie 2000mAh

## Logs

Les événements sont journalisés dans `/log.csv` sur SPIFFS :
- Format : `timestamp,event,distance_cm,battery_mv`
- Accès via moniteur série ou extraction SPIFFS

## Dépannage

- **Wi-Fi ne se connecte pas** : Vérifier SSID/password, portée signal
- **Email non reçu** : Vérifier spam, credentials Mailgun, logs SMTP
- **Fausses détections** : Ajuster seuil distance, stabiliser montage
- **Batterie faible** : Seuil critique < 3200mV