# 📮 Système de Notification pour Boîte Postale

Un système IoT qui détecte automatiquement l'arrivée de colis dans votre boîte postale et vous envoie une notification par email.

## 🎯 Fonctionnalités

- ✅ Détection automatique de l'ouverture de la boîte postale
- ✅ Mesure de distance pour détecter la présence de colis
- ✅ Notification par email avec les détails (distance, batterie, heure)
- ✅ Ultra basse consommation (<50µA en veille)
- ✅ Journalisation des événements
- ✅ Support ESP32 et Arduino Uno

## 📋 Matériel Requis

### Version Arduino (Recommandée pour débuter)
- Arduino Uno ou Nano
- Module WiFi ESP8266 (ESP-01)
- Capteur ultrason HC-SR04
- Reed switch (capteur magnétique)
- Résistances : 10kΩ (x2), 100kΩ (x2)
- LED + résistance 220Ω
- Alimentation 5V (piles ou batterie)

### Version ESP32 (Plus de fonctionnalités)
- ESP32 DevKit
- Capteur ultrason HC-SR04
- Reed switch + aimant
- Batterie Li-Po 3.7V
- Boîtier étanche

## 🚀 Installation Rapide

### 1. Cloner le projet
```bash
git clone https://github.com/Pascalzx/envoie-courriel-arguino.git
cd envoie-courriel-arguino
```

### 2. Installer l'IDE Arduino
Télécharger depuis : https://www.arduino.cc/en/software

### 3. Installer les bibliothèques requises

#### Pour Arduino :
- Dans l'IDE Arduino : `Outils > Gérer les bibliothèques`
- Rechercher et installer :
  - `LowPower` by Rocket Scream
  - `ArduinoJson` by Benoit Blanchon

#### Pour ESP32 :
- Ajouter l'URL dans `Fichier > Préférences > URL de gestionnaire`:
  ```
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  ```
- Dans `Outils > Type de carte > Gestionnaire`, installer "ESP32"

### 4. Configuration Email (Mailgun)

1. **Créer un compte Mailgun** : https://signup.mailgun.com/
2. **Ajouter votre domaine** dans le dashboard Mailgun
3. **Obtenir vos credentials SMTP** :
   - Serveur : `smtp.mailgun.org`
   - Port : `587`
   - Username : `postmaster@votredomaine.mailgun.org`
   - Password : Votre clé API

### 5. Configurer le projet

1. **Copier le fichier de configuration** :
   ```bash
   cp secrets.h.example secrets.h
   ```

2. **Éditer `secrets.h`** avec vos informations :
   ```cpp
   #define WIFI_SSID "VotreWiFi"
   #define WIFI_PASSWORD "VotreMotDePasse"
   #define SMTP_USER "postmaster@votredomaine.mailgun.org"
   #define SMTP_PASS "votre-cle-api-mailgun"
   #define EMAIL_TO "votre-email@example.com"
   ```

### 6. Câblage

#### Arduino
```
Arduino D2 → Reed Switch → GND (avec pull-up 10kΩ vers 5V)
Arduino D3 → HC-SR04 Trig
Arduino D4 → HC-SR04 Echo
Arduino D5 → ESP8266 RX (avec diviseur de tension)
Arduino D6 → ESP8266 TX
Arduino D7 → ESP8266 Reset
Arduino A0 → Point milieu pont diviseur batterie
Arduino 5V → HC-SR04 VCC
Arduino GND → Commun
```

#### ESP32
```
ESP32 GPIO25 → Reed Switch → GND (pull-up interne)
ESP32 GPIO26 → HC-SR04 Trig
ESP32 GPIO27 → HC-SR04 Echo
ESP32 GPIO35 → Point milieu pont diviseur batterie
ESP32 5V → HC-SR04 VCC
ESP32 GND → Commun
```

### 7. Téléverser le code

1. Sélectionner la bonne carte dans `Outils > Type de carte`
2. Sélectionner le bon port dans `Outils > Port`
3. Cliquer sur le bouton Téléverser (→)

## 🔧 Configuration et Calibration

### Calibration de la distance

1. **Ouvrir le moniteur série** (115200 bauds)
2. **Mesurer la distance** boîte vide
3. **Ajuster le seuil** dans le code :
   ```cpp
   #define DISTANCE_THRESHOLD 15  // Ajuster selon vos mesures
   ```

### Test du système

1. **Test WiFi** : Le système doit se connecter au démarrage
2. **Test capteur** : Ouvrir/fermer la porte, vérifier la LED
3. **Test email** : Placer un objet, vérifier la réception

## 📊 Utilisation

### Fonctionnement normal

1. Le système est en veille profonde (consommation minimale)
2. L'ouverture de la porte réveille le système
3. Mesure de distance pour détecter un colis
4. Si colis détecté : connexion WiFi et envoi email
5. Retour en veille après notification

### Indicateurs LED

- **Clignotement rapide** : Mesure en cours
- **Allumée fixe** : Envoi email
- **Éteinte** : Système en veille

## 🔍 Dépannage

### Problèmes fréquents

| Problème | Solution |
|----------|----------|
| WiFi ne se connecte pas | Vérifier SSID/password, portée signal |
| Email non reçu | Vérifier spam, credentials Mailgun |
| Distance incorrecte | Nettoyer capteur, vérifier alimentation |
| Pas de réveil | Vérifier connexion reed switch |
| Consommation élevée | Vérifier mode sleep, désactiver LED |

### Mode debug

Activer les logs détaillés :
```cpp
#define DEBUG 1  // Au début du code
```

### Vérification Mailgun

Tester l'envoi avec curl :
```bash
curl -s --user 'api:YOUR_API_KEY' \
    https://api.mailgun.net/v3/YOUR_DOMAIN/messages \
    -F from='Mailbox <mailbox@YOUR_DOMAIN>' \
    -F to='you@example.com' \
    -F subject='Test' \
    -F text='Test email'
```

## 📁 Structure du Projet

```
envoie-courriel-arguino/
├── mailbox_arduino.ino      # Code pour Arduino Uno
├── mailbox_notifier.ino     # Code pour ESP32
├── secrets.h.example        # Template configuration
├── README.md               # Ce fichier
├── README_ARDUINO.md       # Guide spécifique Arduino
├── DOCUMENT_TECHNIQUE.md   # Documentation technique détaillée
├── platformio.ini          # Configuration PlatformIO
└── .gitignore             # Fichiers à ignorer
```

## 🛠️ Options Avancées

### Utiliser PlatformIO (Recommandé pour développeurs)

1. Installer [VSCode](https://code.visualstudio.com/)
2. Installer l'extension PlatformIO
3. Ouvrir le projet
4. Build : `pio run`
5. Upload : `pio run -t upload`

### Monitoring à distance

Ajouter des métriques :
- Nombre d'ouvertures
- Historique des détections
- Niveau batterie dans le temps

### Intégrations possibles

- IFTTT pour autres notifications
- Home Assistant pour domotique
- ThingSpeak pour graphiques

## 🤝 Contribution

Les contributions sont bienvenues ! N'hésitez pas à :
- Signaler des bugs
- Proposer des améliorations
- Partager vos modifications

## 📄 Licence

Ce projet est sous licence MIT. Voir le fichier LICENSE pour plus de détails.

## 🙏 Remerciements

- Communauté Arduino
- Contributeurs ESP32
- Mailgun pour le service SMTP

---

**Besoin d'aide ?** Ouvrez une [issue](https://github.com/Pascalzx/envoie-courriel-arguino/issues) sur GitHub.