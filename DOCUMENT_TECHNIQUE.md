# Document Technique - Système de Notification Boîte Postale

## 1. Architecture Système

### 1.1 Vue d'ensemble
```
┌─────────────┐     ┌──────────┐     ┌─────────┐
│ Reed Switch │────▶│  Arduino │────▶│ ESP8266 │
└─────────────┘     │   Uno    │     └────┬────┘
                    │          │           │
┌─────────────┐     │ ATmega328│           ▼
│   HC-SR04   │────▶│          │     ┌─────────┐
└─────────────┘     └──────────┘     │  WiFi   │
                                      │ Router  │
                                      └────┬────┘
                                           │
                                           ▼
                                    ┌────────────┐
                                    │  Mailgun   │
                                    │SMTP Server │
                                    └────────────┘
```

### 1.2 Composants principaux
| Composant | Modèle | Rôle | Consommation |
|-----------|---------|------|--------------|
| MCU | ATmega328P | Contrôleur principal | 20µA (veille) / 5mA (actif) |
| WiFi | ESP8266-01 | Communication réseau | 15µA (veille) / 80mA (TX) |
| Capteur distance | HC-SR04 | Détection colis | 2mA (actif) |
| Capteur porte | Reed Switch | Réveil système | 0µA |

## 2. Spécifications Électriques

### 2.1 Alimentation
- **Tension nominale**: 5V (Arduino) / 3.3V (ESP8266)
- **Source**: 2x piles AA (3V) + boost 5V ou batterie Li-Po 3.7V
- **Régulation**: LDO 3.3V pour ESP8266 (MCP1700)
- **Protection**: Diode inverse + fusible resetable 500mA

### 2.2 Consommation détaillée
| Mode | Courant | Durée | Énergie |
|------|---------|--------|---------|
| Deep Sleep | 20µA | 99.9% | ~0.02mAh/jour |
| Mesure distance | 7mA | 200ms | 0.0004mAh |
| Envoi email | 100mA | 10s | 0.28mAh |
| **Total journalier** | - | - | **~0.3mAh** |

### 2.3 Autonomie calculée
- Piles AA (2000mAh): **6-12 mois**
- Li-Po 1000mAh: **3-6 mois**
- Facteurs: température, qualité WiFi, fréquence ouvertures

## 3. Protocoles Communication

### 3.1 Interface ESP8266
- **Baudrate**: 9600 bps (fiable)
- **Protocol**: AT Commands
- **Buffer**: 64 bytes
- **Timeout**: 5s (connexion), 30s (SMTP)

### 3.2 Séquence SMTP
```
1. TCP Connect → smtp.mailgun.org:587
2. EHLO arduino
3. STARTTLS (upgrade TLS)
4. AUTH LOGIN (base64)
5. MAIL FROM / RCPT TO
6. DATA → message
7. QUIT
```

### 3.3 Format données
```json
{
  "timestamp": "2025-01-28 14:30:00",
  "distance_cm": 12.5,
  "battery_mv": 3750,
  "event": "PACKAGE_DETECTED"
}
```

## 4. Gestion Mémoire

### 4.1 Répartition RAM (2KB)
- Stack: 256 bytes
- Variables globales: 512 bytes
- Buffers série: 128 bytes
- String temporaires: 256 bytes
- **Libre**: ~900 bytes

### 4.2 Organisation EEPROM (1KB)
```
Addr 0-3:    Header (magic + index)
Addr 4-1023: Logs circulaires (255 entrées)
Format log:  [Type:1][Battery:2][Distance:1] = 4 bytes
```

### 4.3 Optimisations Flash
- Strings en PROGMEM (F() macro)
- Fonctions inline critiques
- Suppression Serial en production
- **Taille finale**: <10KB

## 5. Algorithmes Clés

### 5.1 Filtrage distance
```c
// Médiane de 5 mesures pour robustesse
float measurements[5];
sort(measurements, 5);
return measurements[2]; // médiane
```

### 5.2 Détection état
```
États possibles:
- IDLE: Attente interruption
- MEASURE: Acquisition capteurs
- DETECT: Analyse présence
- NOTIFY: Envoi notification
- SLEEP: Retour veille

Transitions:
- INT0 → MEASURE
- Distance < seuil → DETECT → NOTIFY
- Timeout → SLEEP
```

### 5.3 Gestion énergie
```c
// Configuration registres AVR
PRR = 0xFF;        // Power Reduction
ADCSRA &= ~(1<<ADEN); // ADC off
ACSR |= (1<<ACD);  // Comparateur off
```

## 6. Sécurité

### 6.1 Protection données
- Credentials en constantes Flash
- Pas de stockage mots de passe
- TLS pour SMTP (STARTTLS)
- Timeout connexions

### 6.2 Fiabilité système
- Watchdog 8s
- Brownout detection 2.7V
- Pull-ups internes activés
- Debounce reed switch 50ms

## 7. Calibration & Tests

### 7.1 Procédure calibration HC-SR04
1. Mesurer distance boîte vide (D_ref)
2. Tester avec colis types
3. Définir seuil = D_ref - 10cm
4. Valider angles détection

### 7.2 Tests validation
| Test | Procédure | Critère succès |
|------|-----------|----------------|
| Portée WiFi | Mesurer RSSI positions | > -80dBm |
| Précision distance | 50 mesures statiques | ±2cm |
| Consommation veille | Multimètre µA | < 50µA |
| Temps réveil | Oscilloscope INT0 | < 100ms |

### 7.3 Mode debug
```c
#define DEBUG 1  // Activer logs série
#define TEST_MODE 1  // Envoi email au démarrage
```

## 8. Évolutions Futures

### 8.1 Améliorations hardware
- LoRa pour zones sans WiFi
- Panneau solaire intégré
- Capteur température/humidité
- Camera basse conso

### 8.2 Améliorations software
- OTA updates
- Machine learning détection
- Multi-utilisateurs
- API REST locale

## 9. Normes & Certifications

- **EMC**: EN 55032 Class B (résidentiel)
- **Safety**: IEC 60950-1
- **Radio**: EN 300 328 (2.4GHz)
- **Étanchéité**: IP44 minimum

## 10. Références

- ATmega328P Datasheet
- ESP8266 AT Command Set
- HC-SR04 Ultrasonic Sensor Datasheet
- Mailgun API Documentation
- Arduino LowPower Library