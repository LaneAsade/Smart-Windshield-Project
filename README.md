# Smart Glare-Reducing Windshield

This is a smart windshield system developed as part of the Hardware Workshop at ABV-IIITM. It uses an ESP32-CAM module and PDLC (Polymer Dispersed Liquid Crystal) films to dynamically reduce glare from intense light sources such as sunlight or headlights.

## How It Works

- The ESP32-CAM continuously captures live video frames of the external environment.
- Each frame is divided into six zones, mapped to corresponding PDLC sections on the windshield.
- A grayscale-based brightness detection algorithm analyzes light intensity in each zone.
- Zones experiencing high glare are selectively dimmed via relay-controlled PDLC films.

This zone-wise dimming helps preserve visibility while reducing driver discomfort due to glare.

## Features

- Real-time image processing using ESP32-CAM
- Six-zone brightness detection and PDLC control
- Serial communication with Arduino Mega for GPIO control
- Simulated and tested circuit design using Fritzing

## Tech Stack

- ESP32-CAM (C++)
- Arduino Mega (Relay control)
- PDLC films
- Web server (ESPAsyncWebServer)
- Grayscale analysis for brightness detection
- Fritzing for circuit design

## Future Improvements

- AI-based glare detection
- Voice or gesture control integration
- Solar-powered system for self-sustainability

## Demo Video

https://github.com/user-attachments/assets/23d1ef1b-6c5d-45da-a017-03d93b5efbbe

Developed by Sisira Asapu, Adarsh Gupta, Hiya Chhawchharia and Nael Saade
---

