# Smart-Remote-Switching-of-Industrial-Machines-Using-LoRa-GSM-and-Android-Application

# Smart Remote Switching of Industrial Machines Using LoRa, GSM and Android Application

This project enables controlling industrial machines from:

✔ **LoRa-based long range remote**  
✔ **GSM/SMS-based switching**  
✔ **Android Application**  
✔ **Web IoT Dashboard backend**

The system supports:

- Switching ON/OFF multiple machines
- Status monitoring (Online/Offline)
- GPRS-based synchronization
- SMS-based fallback control when Internet fails
- LoRa-based local control when GSM is unavailable

---

# 🚀 System Architecture

1. **LoRa Remote → ESP32 Receiver**  
2. **Android App → Server → ESP32**  
3. **SMS Commands → SIM800L → ESP32 Relay Controller**  
4. **Server synchronized using HTTP (PHP + MySQL)**  
5. **Real-time relay state updates**

---

# 🧰 Features

### Communication Methods  
| Method | Description |
|--------|-------------|
| **LoRa** | Long distance, low-power switching |
| **GSM SMS** | Direct SMS control, no internet needed |
| **GPRS/HTTP** | Cloud dashboard synchronization |
| **Android App** | Manual control anywhere |

### Relay Control  
✔ Supports DC relay  
✔ Supports AC SSR  
✔ Supports industrial loads (with isolation)

---

# 🧾 SMS Commands

| SMS | Description |
|-----|-------------|
| DC-ON  | Turns DC relay ON |
| DC-OFF | Turns DC relay OFF |
| AC-ON  | Turns SSR ON |
| AC-OFF | Turns SSR OFF |
| STATUS | Sends back device status |

---

# 📡 LoRa Commands

| Command | Action |
|--------|--------|
| "1ON"  | Load 1 ON |
| "1OFF" | Load 1 OFF |
| "ALLON" | All ON |
| "ALLOFF" | All OFF |

---

# 📱 Android App

- Bluetooth/HTTP/LoRa hybrid controls
- ON/OFF switches
- Status monitor
- Device heartbeat online/offline detection
- Logs screen

---

# 🌐 Server API

| Endpoint | Description |
|----------|-------------|
| `/iot/update.php` | ESP32 updates states |
| `/iot/read.php`   | ESP32 reads commands |
| `/iot/status.php` | heartbeat/status |

---

# 🖧 Hardware Used

- ESP32 Devkit
- SIM800L GSM module
- SX1276 LoRa module
- 5V Relay
- AC SSR
- 5V/3A SIM800L stable power supply
- 433/868/915 MHz antenna
