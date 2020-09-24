// Graphics code originally made by Bitluni (send me a high five if you like the code)
// and modified by Ken Van Hoeylandt 
// Communications protocol and other code by Ken Van Hoeylandt

/**
 * Connections:
 * 
 * [Client]     [Server]    [Composite]
 * 
 * GPIO 4  <->  GPIO 5
 * GPIO 5  <->  GPIO 4
 * GND     <->  GND    <->  outside jack
 *              PIN 25 <->  inside jack pin
 */
 
#include "esp_pm.h"
#include "Matrix.h"
#include "CompositeGraphics.h"
#include "Mesh.h"
#include "Image.h"
#include "CompositeOutput.h"
#include "font6x8.h"
#include "driver/uart.h"
#include "GraphicsServer.h"

//PAL MAX, half: 324x268 full: 648x536
//NTSC MAX, half: 324x224 full: 648x448
const int XRES = 320;
const int YRES = 200;

//Graphics using the defined resolution for the backbuffer and fixed buffer for triangles
CompositeGraphics graphics(XRES, YRES, 1337);

//Composite output using the desired mode (PAL/NTSC) and twice the resolution. 
//It will center the displayed image automatically
CompositeOutput composite(CompositeOutput::NTSC, XRES * 2, YRES * 2);

Font<CompositeGraphics> font(6, 8, font6x8::pixels);

GraphicsServer server;

#include <soc/rtc.h>

void setup()
{
  //highest clockspeed needed
  esp_pm_lock_handle_t powerManagementLock;
  esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "compositeCorePerformanceLock", &powerManagementLock);
  esp_pm_lock_acquire(powerManagementLock);
  
  // Initializing DMA buffers and DAC
  composite.init();
  // Initializing graphics double buffer
  graphics.init();
  // Select font
  graphics.setFont(font);
  // Start listening for incoming data
  server.init();
  
  // Composite output is pinned to core 0
  xTaskCreatePinnedToCore(compositeCore, "compositeCoreTask", 1024, NULL, 1, NULL, 0);
}

void compositeCore(void *data)
{
  while (true)
  {
    //just send the graphics frontbuffer whithout any interruption 
    composite.sendFrameHalfResolution(&graphics.frame);
  }
}

void loop()
{
  server.update(graphics);
}
