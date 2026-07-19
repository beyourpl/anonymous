# M5Stick Cyber Awareness

Atelier de **sensibilisation à la cybersécurité** avec le **M5Stick jaune** (original M5Stack).

Deux approches complémentaires :

| Mode | Matériel | Description |
|------|----------|-------------|
| **Autonome** | M5Stick seul | Faux Wi-Fi, portail captif, brute force simulé ou réel (labo) |
| **PC + M5Stick** | USB + PC | Hashcat (ou simulation) sur PC, progression affichée sur l'écran du stick |

---

## Menu du M5Stick

| # | Mode | Boutons |
|---|------|---------|
| 1 | **Faux Wi-Fi** | Diffuse `Starbucks_Free` + portail captif |
| 2 | **Brute force sim** | Animation réaliste, mot de passe faible trouvé à la fin |
| 3 | **Brute force réel** | Teste une wordlist contre **votre** AP de labo |
| 4 | **Mode PC** | Affiche la progression envoyée par le script Python |
| 5 | **Quiz phishing** | 6 scénarios : arnaque ou légitime ? |
| 6 | **DDoS sim** | Animation pédagogique (aucun paquet réseau) |
| 7 | **DDoS lab** | Démo coopérative avec PC victime (sans flood) |
| 8 | **Sensibilisation** | Rotation de bonnes pratiques |

- **A** : descendre dans le menu / pause / conseil suivant  
- **B** : valider / quitter  

---

## Matériel

- M5Stick jaune (original)
- Câble USB
- *(Optionnel)* Routeur de labo avec WPA2 et mot de passe faible
- *(Optionnel)* PC avec hashcat pour le mode avancé

---

## Installation firmware

### PlatformIO (recommandé)

```bash
pip install platformio
cd m5stick-cyber-awareness   # ou racine du repo
pio run -t upload
pio device monitor -b 115200
```

### Configuration

Éditer `include/config.h` :

```cpp
#define LAB_SSID         "LABO_CYBER_DEMO"   // votre AP de test
#define LAB_PASSWORD     "12345678"          // mot de passe volontairement faible
#define FAKE_AP_SSID     "Starbucks_Free"    // nom du faux réseau
#define LAB_VICTIM_HOST  "192.168.1.50"      // IP du PC (serveur victime)
```

---

## Déroulé d'atelier suggéré

### Partie 1 — M5Stick autonome (~15 min)

1. **Faux Wi-Fi** : montrer le SSID sur un téléphone, se connecter, ouvrir le portail captif.
2. Afficher sur l'écran ce qui a été saisi → *« Un attaquant récupère ça en quelques secondes »*.
3. **Brute force sim** : lancer la simulation, laisser le suspense, mot de passe trouvé.
4. **Quiz phishing** : les participants votent arnaque / légitime sur 6 scénarios.
5. **DDoS sim** : montrer le principe d'une saturation (simulation pure).
6. **Sensibilisation** : parcourir les conseils à l'écran.

### Partie 2 — DDoS lab (sans flood) (~10 min)

1. PC et M5Stick sur le **même Wi-Fi de labo**.
2. Sur le PC :

```bash
python3 pc-tools/ddos_victim_server.py
# noter l'IP du PC, l'ouvrir dans un navigateur : http://IP:8080
```

3. Mettre cette IP dans `LAB_VICTIM_HOST` (`include/config.h`), reflasher.
4. Sur le M5Stick : mode **7 DDoS lab** → **A** pour lancer.
5. Le navigateur montre la jauge monter (saturation **volontaire** côté serveur).
6. Débrief : un vrai DDoS est illégal ; ici aucun flood n'a lieu.

### Partie 3 — M5Stick + PC hashcat (~15 min)

1. Configurer un routeur de labo `LABO_CYBER_DEMO` / `12345678`.
2. Capturer le handshake WPA2 (ex. `hcxdumptool` + `hcxpcapngtool`) → fichier `.hc22000`.
3. Sur le M5Stick : mode **4 Mode PC**.
4. Sur le PC :

```bash
pip install pyserial
python3 pc-tools/crack_display.py --simulate
# ou avec hashcat :
python3 pc-tools/crack_display.py capture.hc22000 pc-tools/wordlist-demo.txt
```

5. Le public voit la progression sur le petit écran jaune pendant que le PC crack.
6. Débrief oral : VPN, mots de passe longs, pas de Wi-Fi inconnu.

---

## Protocole série (mode PC)

Le script envoie des lignes à 115200 baud :

```
STATUS Hashcat en cours...
PROGRESS 47 100 password123
FOUND 12345678
RESET
```

---

## Cadre légal et éthique

**Obligatoire** pour toute démonstration :

- Réseau **isolé** que vous contrôlez (pas le Wi-Fi du lieu)
- Panneau visible : *« Atelier cybersécurité — ne pas saisir de vrais identifiants »*
- **Consentement** des participants
- Ne jamais cibler un réseau tiers (illégal en France, art. 323-1 et s. du Code pénal)
- Le mode **DDoS sim** est une **animation uniquement** : aucun paquet n'est envoyé.
- Le mode **DDoS lab** envoie au plus ~2 req/s (plafond 40) vers une **IP privée** de labo ; le serveur augmente sa jauge volontairement. Ce n'est **pas** un flood. Un vrai DDoS est illégal (art. 323-2).

Le portail captif affiche un avertissement pédagogique. Utilisez uniquement des identifiants fictifs en démo.

---

## Structure du projet

```
├── platformio.ini
├── include/config.h          # SSID labo, faux AP
├── src/
│   ├── main.cpp
│   ├── fake_wifi.cpp         # AP + portail captif
│   ├── bruteforce_sim.cpp    # simulation écran
│   ├── bruteforce_real.cpp   # wordlist → AP labo
│   ├── pc_bridge.cpp         # affichage série
│   ├── phishing_quiz.cpp     # quiz arnaque / légitime
│   ├── ddos_sim.cpp          # démo DDoS (simulation, 0 trafic)
│   ├── ddos_lab.cpp          # pulses HTTP vers PC victime (sans flood)
│   ├── awareness.cpp         # conseils
│   └── menu.cpp
└── pc-tools/
    ├── crack_display.py      # bridge PC → M5Stick
    ├── ddos_victim_server.py # dashboard victime labo
    └── wordlist-demo.txt
```

---

## Limites techniques

- L'ESP32 ne remplace pas hashcat pour un vrai audit (trop lent).
- Le brute force **réel** sur le M5Stick fonctionne uniquement contre un AP de labo avec mot de passe dans la petite wordlist embarquée.
- Le mode **simulation** est idéal pour écoles et salons sans infrastructure réseau.
- **DDoS lab** refuse les IP publiques ; plafond de pulses côté stick.

---

## Licence

Projet pédagogique — utilisation responsable uniquement.
