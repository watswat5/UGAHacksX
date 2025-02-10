# 🎵 ESP32 Polyphonic Synthesizer

## 🎹 Project Overview
This project is a **polyphonic 16-voice synthesizer** powered by an **ESP32-WROVER module**, running the **Mozzi** synthesizer library. Designed for expressive music creation, this device features an intuitive button layout for note selection and **thumbstick-controlled pitch shifting**.

---

## 👥 Group Members
- **Christopher Antepenko**
- **Caleb Barnett**
- **Adithya Lakshmikanth**
- **Krishna Mohan**

---

## 🎼 Usage
### **🎛️ Controls**
- **4 left-hand buttons** select the **octave section**.
- **4 right-hand buttons** select the **note within that section**.
- **Thumbsticks** allow for **pitch shifting** and expressive modulation.

### **🎵 Note Mapping**
| Left Hand | Right Index | Right Middle | Right Ring | Right Pinky |
|-----------|------------|--------------|------------|------------|
| **Left Index** | C4 | D4 | E4 | F4 |
| **Left Middle** | G4 | A4 | B4 | C5 |
| **Left Ring** | D5 | E5 | F5 | G5 |
| **Left Pinky** | A5 | B5 | C6 | D6 |

- **Play any combination of these 8 buttons simultaneously** for full polyphony.
- **Thumbsticks for pitch shifting**:
  - 🎚 **Left thumbstick** controls **Left Index & Left Middle octaves**.
  - 🎚 **Right thumbstick** controls **Left Ring & Left Pinky octaves**.

This setup enables effects like **glissandos**, **vibratos**, and **microtonal adjustments**.

---

## 🛠️ Building & Setup
### **🔧 Hardware Requirements**
- **ESP32-WROVER or ESP32-S3** *(pin configuration may vary by model)*
- **Mechanical keyboard switches** *(snap-fit holes in the 3D model)*
- **Thumbstick modules** *(commonly used for Arduino projects)*
- **Amplifier & speaker / 3.5mm jack for external audio**
- **Custom 3D-printed enclosure** *(CAD files included in `cad/` folder)*
- **Liberal use of hot glue!** 😆

### **💾 Software Requirements**
1. **Install Required Libraries** from the **Arduino Library Manager**:
   - `Mozzi`
   - `FixMath`
2. **Use the correct ESP32 board package**:
   - Install **ESP32 package v2.0.14** from Espressif (⚠️ Do NOT use v3.11, as it conflicts with the Wire library and will prevent sound output.)

### **📡 Audio Output**
- **Line-out connection** for external speakers or amplifiers.
- **UGA Hacks X version** features an **internal speaker & amplifier**.

---

## 📂 Resources & Files
📁 **Schematic & Wiring** – Included in the repository for proper assembly.
📁 **3D Models & CAD Files** – Found in the `cad/` folder for custom enclosures.

---

## 🚀 Future Improvements
- Enhance thumbstick calibration for **smoother pitch control**.
- Modify case ergonomics to improve handling.
- Add a display and menu to allow the user to modify synth effects.
- Improve enclosure design to avoid **thumbstick collision issues**.
- Experiment with **additional synthesis techniques** using the Mozzi library.

🎶 *Unleash your inner musician and start creating unique sounds with this ESP32 Synth!* 🎶

