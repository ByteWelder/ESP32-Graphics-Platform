#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "GraphicsClient.h"

#define DEBUG_GRAPHICS

#ifdef DEBUG_GRAPHICS
#define logDebug(x) printf(x)
#else
#define logDebug(x)
#endif
#define logInfo(x) printf(x)

GraphicsClient client;

static void sendInitialResources() {
    logInfo("Connected: sending initial resources...\n");
    client.setResourceText(0, "Hello, world!");
    logInfo("Connected: done\n");
}

static void waitForConnection() {
    logInfo("Waiting for connection...\n");
    do {
        if (client.tryConnect()) { // TODO: this always returns true - fix it!
            sendInitialResources();
        } else {
            delay(500);
        }
    } while (!client.isConnected());
}

static void sender_task(void* args)
{
    while (1) {
        if (!client.isConnected()) {
            waitForConnection();
        } else {
            logDebug("begin\n");
            client.sceneBegin();
            logDebug("render\n");
            client.renderText(0, 0, 0, 5);
            logDebug("end\n");
            client.sceneEnd();

#ifdef DEBUG_GRAPHICS
            delay(1000);
#endif
        }
    }
}

void setup()
{
    Serial.begin(115200);
    printf("Starting master task\n");
    xTaskCreate(sender_task, "sender_task", 2048, NULL, 10, NULL);
}

void loop() {
    vTaskDelay(1000);
}
