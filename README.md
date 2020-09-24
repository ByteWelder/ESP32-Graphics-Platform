# Introduction

The aim of this project is to combine two [ESP32](https://www.espressif.com/en/products/socs/esp32) SOCs to create a platform for
rendering 2D and 3D graphics.

One ESP32 serves as a graphics `host`: it takes care of rendering and composite video out.
The other ESP32 serves as the graphics `client`: it sends data and tells the `host` how to render it. 

This project is inspired by Bitluni's composite video [article](https://bitluni.net/esp32-composite-video) and uses his
[code](https://github.com/bitluni/ESP32CompositeVideo) as the foundation for the 3D graphics `host`.

# Hardware

The `host` should run on a standard (dual core) ESP32.

The `client` can in theory be any ESP32 with an available hardware UART.

# Wiring

```
 [Client]     [Server]    [Composite]
  
 GPIO 4  <->  GPIO 5
 GPIO 5  <->  GPIO 4
 GND     <->  GND    <->  outside jack
              PIN 25 <->  inside jack pin
```

# Getting Started

More information will follow.

