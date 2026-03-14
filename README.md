# FPGA-TDR-System: High-Resolution Cable Fault Detection via Multi-Phase TDC

An end-to-end, hardware-accelerated Time Domain Reflectometer (TDR) built from scratch on a **Cyclone IV E FPGA**. This system detects, locates, and diagnoses physical faults (opens/shorts) in transmission lines using sub-cycle digital signal processing. The raw hardware logic is bridged to the cloud via an **ESP8266**, featuring a live web dashboard and predictive cable maintenance powered by the **Gemini API**.

---

## 📖 Table of Contents
- [Project Overview](#project-overview)
- [Core Technologies & Concepts](#core-technologies--concepts)
  - [What is TDR?](#what-is-tdr)
  - [Why FPGA?](#why-fpga)
  - [Sub-Cycle Resolution via PLL](#sub-cycle-resolution-via-pll)
- [Hardware Architecture](#hardware-architecture)
  - [The Analog Front-End](#the-analog-front-end)
  - [FPGA Digital Logic (Verilog)](#fpga-digital-logic-verilog)
- [IoT & AI Integration](#iot--ai-integration)
- [Wiring & Setup Guide](#wiring--setup-guide)
- [Author](#author)

---

## 🚀 Project Overview

[cite_start]This project implements a Time-to-Digital Converter (TDC) architecture to measure the "time of flight" of electrical reflections in a Cat-5 twisted pair cable[cite: 2]. By tracking the exact nanosecond a transmitted pulse echoes back from a cut or short, the system computes the exact distance to the fault. 

The distance is transmitted via a custom-built hardware UART to an ESP8266, which hosts an asynchronous webserver for real-time monitoring and queries an AI model to predict cable degradation based on signal velocity and cable age.

---

## 🧠 Core Technologies & Concepts

### What is TDR?
Time Domain Reflectometry (TDR) acts like radar for cables. It injects a sharp voltage pulse down a transmission line. Whenever the pulse hits an impedance mismatch (like a severed wire or a short circuit), a portion of that energy reflects back to the source. By measuring the time ($\Delta t$) it takes for the echo to return, and knowing the velocity of propagation ($v$) in the cable, the distance ($d$) to the fault is calculated:
$$d = \frac{v \cdot \Delta t}{2}$$

### Why FPGA?
Microcontrollers process instructions sequentially, making it impossible to guarantee exact nanosecond timing for high-speed signal capture. An FPGA (Field Programmable Gate Array) allows us to design custom, parallel silicon circuits. This guarantees absolute determinism—our counters never miss a clock cycle while waiting for an interrupt, allowing us to catch reflections traveling at $200,000,000 \text{ meters/second}$ ($20 \text{ cm/ns}$).

### Sub-Cycle Resolution via PLL (Phase-Locked Loop)
A standard $50\text{ MHz}$ clock has a period of $20\text{ ns}$—far too slow for accurate TDR, as $20\text{ ns}$ equals $2\text{ meters}$ of blind spot. To achieve high-resolution timing without requiring a multi-GHz processor, this project exploits the Cyclone IV's hardware **ALTPLL**. 

The PLL generates **four phase-shifted clocks** from the base $50\text{ MHz}$ signal:
* `clk0` (0°)
* `clk90` (90°)
* `clk180` (180°)
* `clk270` (270°)

By running four parallel counters driven by these offset clocks, we effectively divide the $20\text{ ns}$ period by 4. This creates a multi-phase Time-to-Digital Converter (TDC) with an effective sampling rate of **200 MHz**, giving us a precise **$5\text{ ns}$ temporal resolution**.

---

## ⚙️ Hardware Architecture

### The Analog Front-End (The Voltage Divider)
Standard Cat-5 cable has a characteristic impedance of $\approx 100 \Omega$[cite: 2]. The FPGA's `tx` pin drives the cable through a physical $100 \Omega$ series termination resistor to prevent secondary reflections and match the impedance.

### FPGA Digital Logic (Verilog)
The RTL is modularized into dedicated silicon blocks:
1. **Pulse Generator:** Periodically fires a $100\text{ ns}$ pulse into the transmission line.
2. **Asynchronous Strobe Logic:** A combinational gate (`strobe = rx_raw & ~tx`) that isolates the reflection window, bypassing standard flip-flop metastability delays to preserve the $5\text{ ns}$ resolution.
3. **TDC Counters:** Four parallel accumulators that sum the active strobe duration across the phase-shifted clocks.
4. **Hardware Multiplier (Distance Calc):** Converts the raw nanosecond counts into physical centimeters in real-time.
5. **UART Transmitter:** A custom state machine running at 115200 baud that frames the 16-bit distance calculation into a 3-byte packet `[High Byte][Low Byte][\n]` for the ESP8266.

---

## 🌐 IoT & AI Integration

The system isn't just an edge sensor; it's a fully connected diagnostic hub.
* **ESP8266 Serial Parsing:** The microcontroller continuously buffers the raw UART stream from the FPGA, reconstructing the High/Low bytes into integers.
* **Live Webserver:** Hosts a dynamic web interface accessible via any local browser, displaying the live cable distance metrics.
* **Gemini API Predictive Maintenance:** Users can input the cable's installation age and baseline signal velocity. The ESP8266 routes this data to the Gemini LLM API, which predicts dielectric degradation, velocity factor drift, and estimates the remaining operational lifespan of the line.

---

## 🔌 Wiring & Setup Guide

### 1. Physical Connections
| Component | FPGA Pin (Cyclone IV E) | Target |
| :--- | :--- | :--- |
| **TX Pulse** | Assign to GPIO | Connects to `100Ω Resistor` -> `Cat-5 Wire A` |
| **RX Sense** | Assign to GPIO | Connects to junction between `Resistor` & `Wire A` |
| **Ground** | GND | Connects to `Cat-5 Wire B` (Twisted Pair Return) |
| **UART TX** | Assign to GPIO | Connects to `ESP8266 RX` Pin |
| **Reset** | Assign to Push Button | (Enable Weak Pull-Up in Quartus) |

### 2. Quartus Setup
1. Open the project in Intel Quartus Prime.
2. Generate the ALTPLL IP (`50MHz` input -> Four `50MHz` outputs shifted at 0°, 90°, 180°, 270°).
3. Apply timing constraints via the included `timing.sdc` file to ensure proper routing of the 5ns TDC paths.
4. Compile and flash the `.sof` file to the Cyclone IV E via a USB-Blaster.

### 3. ESP8266 Setup
1. Open the provided `.ino` sketch in the Arduino IDE.
2. Insert your Wi-Fi credentials and Gemini API Key.
3. Flash to the ESP8266. 
4. Open the Serial Monitor at `115200 baud` to view the local IP address for the web dashboard.

---

## 👨‍💻 Authors
**Garv Kapoor** B.Tech in Electronics and Communication Engineering (ECE)  
NIT Hamirpur

**Kritika Bhandari** B.Tech in Electronics and Communication Engineering (ECE)  
NIT Hamirpur

**Shranya Thakur** B.Tech in Electronics and Communication Engineering (ECE)  
NIT Hamirpur

**Shubham Pathak** B.Tech in Electronics and Communication Engineering (ECE)  
NIT Hamirpur

