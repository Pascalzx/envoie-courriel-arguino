# 🛠️ Guide d'Installation Détaillé

## Prérequis

- Ordinateur (Windows, Mac ou Linux)
- Câble USB pour Arduino
- Compte email pour recevoir les notifications
- Réseau WiFi 2.4GHz

## Étape 1 : Installation des Logiciels

### Option A : Arduino IDE (Débutants)

1. **Télécharger Arduino IDE**
   - Aller sur https://www.arduino.cc/en/software
   - Choisir votre système d'exploitation
   - Installer le logiciel

2. **Configuration Arduino IDE**
   - Ouvrir Arduino IDE
   - Aller dans `Fichier > Préférences`
   - Dans "URL de gestionnaire de cartes", ajouter :
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```

### Option B : PlatformIO (Avancé)

1. **Installer Visual Studio Code**
   - Télécharger depuis https://code.visualstudio.com/

2. **Installer PlatformIO**
   - Ouvrir VSCode
   - Aller dans Extensions (Ctrl+Shift+X)
   - Rechercher "PlatformIO"
   - Cliquer sur Install

## Étape 2 : Préparation du Projet

### 1. Télécharger le code

```bash
# Option 1 : Git (si installé)
git clone https://github.com/Pascalzx/envoie-courriel-arguino.git
cd envoie-courriel-arguino

# Option 2 : Télécharger ZIP
# Aller sur https://github.com/Pascalzx/envoie-courriel-arguino
# Cliquer sur "Code" > "Download ZIP"
# Extraire le fichier ZIP
```

### 2. Configuration des secrets

```bash
# Copier le fichier exemple
cp secrets.h.example secrets.h

# Ouvrir secrets.h dans un éditeur texte
# Remplacer les valeurs par vos informations
```

## Étape 3 : Installation des Bibliothèques

### Pour Arduino IDE

1. Ouvrir `Outils > Gérer les bibliothèques`
2. Rechercher et installer :
   - **LowPower** par Rocket Scream
   - **ArduinoJson** par Benoit Blanchon (v6.x)

### Pour PlatformIO

Les bibliothèques s'installent automatiquement !

## Étape 4 : Configuration Mailgun

### 1. Créer un compte

1. Aller sur https://signup.mailgun.com/
2. Créer un compte gratuit
3. Confirmer votre email

### 2. Ajouter un domaine

1. Dans le dashboard Mailgun, aller dans "Sending" > "Domains"
2. Cliquer sur "Add New Domain"
3. Entrer votre domaine (ex: mg.votresite.com)
4. Suivre les instructions DNS

### 3. Obtenir les credentials

1. Dans "Domain Settings" > "SMTP Credentials"
2. Noter :
   - Login : `postmaster@mg.votresite.com`
   - Password : Cliquer sur "Reset Password" pour en créer un

## Étape 5 : Assemblage du Circuit

### Matériel nécessaire

#### Liste d'achat suggérée

| Composant | Quantité | Prix approx. | Lien exemple |
|-----------|----------|--------------|--------------|
| Arduino Uno | 1 | 25$ | [Amazon](https://www.amazon.ca/s?k=arduino+uno) |
| ESP8266 ESP-01 | 1 | 5$ | [Amazon](https://www.amazon.ca/s?k=esp8266+esp-01) |
| HC-SR04 | 1 | 5$ | [Amazon](https://www.amazon.ca/s?k=hc-sr04) |
| Reed Switch | 1 | 3$ | [Amazon](https://www.amazon.ca/s?k=reed+switch) |
| Kit résistances | 1 | 10$ | [Amazon](https://www.amazon.ca/s?k=resistor+kit) |
| Breadboard | 1 | 8$ | [Amazon](https://www.amazon.ca/s?k=breadboard) |
| Fils jumper | 1 pack | 8$ | [Amazon](https://www.amazon.ca/s?k=jumper+wires) |

### Schéma de câblage Arduino

```
                    Arduino Uno
                 ________________
                |                |
                |            13 ○|── LED (optionnel)
                |            12 ○|
                |            11 ○|
    Reed ──10kΩ─|─D2        10 ○|
    Switch   └──|─5V         9 ○|
                |             8 ○|
    ESP RST ────|─D7         7 ○|──── ESP8266 Reset
    ESP TX ─────|─D6         6 ○|──── ESP8266 TX
    ESP RX ─3.3k|─D5         5 ○|──┬─ ESP8266 RX
              └─|─GND        4 ○|  └─ 5.6kΩ ── GND
    HC-SR04 ────|─D4         3 ○|──── HC-SR04 Echo
    Trig ───────|─D3         2 ○|──── HC-SR04 Trig
                |             1 ○|
                |             0 ○|
                |                |
                |            A5 ○|
                |            A4 ○|
                |            A3 ○|
                |            A2 ○|
                |            A1 ○|
    Batterie ───|─A0        A0 ○|──┬─ 100kΩ ── VBAT+
              └─|─GND           |  └─ 100kΩ ── GND
                |________________|
```

### Connexions détaillées

1. **Reed Switch**
   - Un côté → Arduino D2
   - Autre côté → GND
   - Résistance 10kΩ entre D2 et 5V

2. **HC-SR04**
   - VCC → Arduino 5V
   - GND → Arduino GND
   - Trig → Arduino D3
   - Echo → Arduino D4

3. **ESP8266**
   - VCC → 3.3V (IMPORTANT!)
   - GND → Arduino GND
   - TX → Arduino D6
   - RX → Arduino D5 (avec diviseur de tension)
   - RST → Arduino D7

## Étape 6 : Upload du Code

### Arduino IDE

1. Ouvrir le fichier `mailbox_arduino.ino`
2. Sélectionner `Outils > Type de carte > Arduino Uno`
3. Sélectionner `Outils > Port > COM3` (ou autre)
4. Cliquer sur le bouton Upload (→)

### PlatformIO

1. Ouvrir le dossier du projet dans VSCode
2. Attendre que PlatformIO charge
3. Cliquer sur l'icône ✓ (Build)
4. Cliquer sur l'icône → (Upload)

## Étape 7 : Test et Vérification

### 1. Ouvrir le moniteur série

- **Arduino IDE** : `Outils > Moniteur série`
- **PlatformIO** : Icône 🔌 dans la barre du bas
- Régler sur 9600 bauds

### 2. Vérifier les messages

Vous devriez voir :
```
Demarrage...
AT+RST
OK
WiFi connecté
```

### 3. Test complet

1. Ouvrir la porte (activer reed switch)
2. LED doit s'allumer
3. Placer un objet devant HC-SR04
4. Attendre l'email (1-2 minutes)

## Dépannage Installation

### Erreurs communes

| Erreur | Solution |
|--------|----------|
| "Port COM non trouvé" | Installer drivers CH340/FTDI |
| "Bibliothèque manquante" | Vérifier installation dans gestionnaire |
| "Upload failed" | Vérifier sélection carte/port |
| "ESP8266 ne répond pas" | Vérifier alimentation 3.3V |

### Drivers USB

Si Arduino non reconnu :

**Windows** :
- Télécharger drivers CH340 : https://sparks.gogo.co.nz/ch340.html

**Mac** :
- Installer drivers FTDI ou CH340 selon votre carte

**Linux** :
- Généralement reconnu automatiquement
- Sinon : `sudo usermod -a -G dialout $USER`

## Support

Besoin d'aide ? 
- Ouvrir une [issue GitHub](https://github.com/Pascalzx/envoie-courriel-arguino/issues)
- Inclure :
  - Votre configuration
  - Messages d'erreur
  - Photos du montage