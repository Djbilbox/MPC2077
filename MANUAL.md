# MPC2077 — Manuel d'Utilisation

**MPC2077 by DJBILBOX BEATS** — drum machine VST3/Standalone cyberpunk minimaliste.

---

## Démarrage rapide

### Installation (Windows)
1. Télécharge `MPC2077-Windows.zip`
2. Extrait le dossier
3. Lance `MPC2077.exe` (Standalone) ou copie le VST3 vers `C:\Program Files\Common Files\VST3\`

### Installation (Mac)
1. Télécharge `MPC2077-macOS.dmg`
2. Double-clic pour monter le volume
3. Drag le plugin AU vers `/Library/Audio/Plug-Ins/Components/`
4. Relance ton DAW

---

## Interface utiliselle

### Haut (TopPanel)
- **LCD Preset Display** : affiche le numéro et le nom du preset courant (PRESET XX/50)
- **< / >** : navigue entre les presets
- **SAVE / LOAD** : sauvegarde/charge tes presets utilisateur personnalisés
- **INIT** : réinitialise à l'état par défaut
- **SEQ** : bascule entre la grille de pads et l'éditeur de séquenceur
- **Logo camel** : badge neon cyan (décoratif)
- **VOL** : volume master (0.0 — 2.0)

### Mixer Strip (ligne de 16 potards)
- **KK, 808, SN, CP, CH, OH, LT, MT, HT, RM, CR, RD, CV, CB, CG, SP**
- Chaque potard = niveau de volume du pad correspondant (0.0 — 1.5)
- **Clique et tire** sur un potard pour sélectionner ce pad

### Grille de pads (4×4)
- **16 pads** avec les vrais samples 808/909/707
- **Clique** = déclenche le pad
- **Double-clic** = ouvre un navigateur pour charger un sample WAV custom
- **Drag & Drop** = colle un WAV sur le pad
- Chaque pad affiche son nom (KICK, 808, SNARE, etc.)
- **Point jaune** en haut à droite = indica sample utilisateur chargé

### Colonne droite (Knob Sections)

#### ENVELOPE (A/D/S/R)
- **ATTACK** : temps d'attaque (0 — 2s)
- **DECAY** : temps de descente (0 — 2s)
- **SUSTAIN** : niveau de sustain (0.0 — 1.0)
- **RELEASE** : temps de relâchement (0 — 3s)

#### TONE
- **PITCH** : transposition en semitones (−24 — +24)
- **FILTER** : type de filtre (OFF / LP / HP / BP)
- **RESO** : résonance du filtre (0.0 — 1.0)
- **DRIVE** : saturation (0.0 — 1.0)

#### FX (master effects)
- **REVERB** : wet/dry (0.0 — 1.0)
- **DELAY** : feedback (0.0 — 1.0)
- **CRUSH** : bitcrush quantization (0.0 — 1.0)
- **GLITCH** : beat-repeat glitch (0.0 — 1.0)
- **CYBER** : auto-wah cyber (0.0 — 1.0)

---

## Mode Séquenceur (SEQ)

Clique **SEQ** pour basculer vers l'éditeur de pas-à-pas :

- **Grille 16 pads × 32 steps** : clique/tire pour peindre les steps
- **Playhead jaune** : montre le step courant pendant la lecture
- **16 / 32** toggle : change la résolution (16 steps ou 32 steps)
- **SWING** slider : applique une swing groove (0.0 — 1.0)
- **HUMANIZE** slider : ajoute du jitter aléatoire (0.0 — 1.0)
- **CLR** button : efface tous les steps

La lecture est **syncronisée au transport de ton host** (FL Studio, Reaper, etc.) ou utilise l'horloge interne si le host ne fournit pas de sync.

---

## 50 Presets Factory

Chaque preset a ses propres **patterns de drums** (kicks/snares/claps distincts) et **paramètres FX** :

### Top 5 à essayer
1. **Night City 808** (boom-bap 808 classique)
2. **Cyber Monk Slice** (cowbell accent + distortion)
3. **Blade Runner Trap** (trap hats rapides)
4. **Neon Rain Bounce** (swing groove bounce)
5. **Zero Day Glitch** (glitch FX + glitch beat-repeat)

Les 45 autres offrent une variété de grooves cyberpunk (Tokyo Neon Dream, Digital Heat, Future Shock, etc.).

---

## Tips & Tricks

### Changer les sounds
- Double-clic sur un pad → sélectionne un WAV custom
- Tous les samples restent dans le preset quand tu sauvegardes
- Les samples sont **resamplés en temps réel** à la hauteur du pad

### Automation
- Tous les paramètres master (VOL, FX, SWING, HUMANIZE, PITCH WHEEL, MOD WHEEL) sont **automatable dans le DAW**
- Les paramètres per-pad (ENVELOPE, TONE) ne sont pas automatable (par design — ils changent quand tu sélectionnes un pad dans l'UI)

### Performance
- **24 voix de polyphonie** : jusqu'à 24 notes simultanées
- VST3 + Standalone supportés (Windows) ; AU (Mac)
- CPU usage léger (~2—5% per instance à 44.1kHz)

### Save/Load
- **SAVE** : exporte ton preset courant en `.mpcpreset` (ValueTree XML)
- **LOAD** : importe un preset depuis un fichier `.mpcpreset`
- Les presets sauvegardés incluent les samples WAV **par chemin de fichier** (ils seront relinkés si tu déplaces les WAVs)

---

## Spécifications techniques

| Paramètre | Valeur |
|-----------|--------|
| Pads | 16 (4×4 MPC grid) |
| Polyphonie | 24 voix |
| Samples intégrés | 16 × 808/909/707 one-shots |
| Presets factory | 50 |
| Séquenceur | 16 / 32 steps, host-synced |
| Filtres | SVF (LP/HP/BP) par pad |
| Master FX | Reverb, Delay, Bitcrush, Glitch, Cyber Wah |
| Format (Windows) | VST3, Standalone .exe |
| Format (Mac) | AU (Audio Unit), Standalone |
| C++ Standard | C++17 |
| Framework | JUCE 8.0 |

---

## Support

Pour les bugs ou questions, contacte : **djbilbox@gmail.com**

**MPC2077** © 2026 DJBILBOX BEATS. Tous droits réservés.
