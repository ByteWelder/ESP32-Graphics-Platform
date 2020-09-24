#pragma once
#include "CompositeGraphics.h"
#include "GraphicsProtocol.h"
#include "UartClient.h"

#define ECHO_TEST_TXD  (GPIO_NUM_4)
#define ECHO_TEST_RXD  (GPIO_NUM_5)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)

#define BUFFER_SIZE 4096

class GraphicsServer
{ 
private:
    UartClient uart;
    bool sceneStarted = false;
    void* resources[1024];
    int resourceIndex = -1;
    RenderText renderTextData;

public:

    void init() {
        for (int i = 0; i < 1024; ++i) {
            resources[i] = NULL;
        }
    }

    void update(CompositeGraphics& graphics) {
        static int lastMillis = 0;
        int t = millis();
        int fps = 1000 / (t - lastMillis);
        lastMillis = t;

        uint8_t commandCharacter = 0;
        if (uart.readUint8(&commandCharacter)) {
            if (commandCharacter == (uint8_t) Command::INIT) {
                printf("GraphicsServer: init started\n");
                uint32_t freeMemory = (uint32_t) heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
                bool allWritten = sendInitResponse(uart);
                if (allWritten) {
                    printf("GraphicsServer: init finished\n");
                } else {
                    printf("Error: GraphicsServer: init failed\n");
                }
            } else if (commandCharacter == (uint8_t) Command::SCENE_BEGIN) {
                printf("scene begin\n");
                sceneStarted = true;
                graphics.begin(54);
            } else if (commandCharacter == (uint8_t) Command::SCENE_END) {
                printf("scene end\n");
                graphics.end();
                sendSceneEndVerification(uart);
                sceneStarted = false;
            } else if (commandCharacter == (uint8_t) Command::SET_RESOURCE) {
                printf("set resource\n");
                receiveResource(uart, resources);
            } else if (commandCharacter == (uint8_t) Command::RENDER_TEXT_RESOURCE) { // render text resource
                printf("render text resource\n");
                if (sceneStarted) {
                    if (receiveRenderText(uart, renderTextData)) {
                        graphics.setTextColor(renderTextData.color);
                        graphics.setCursor(renderTextData.positionX, renderTextData.positionY);
                        if (renderTextData.resourceIndex < 1024 && resources[renderTextData.resourceIndex] != NULL) {
                            const char* resource = (const char*) resources[renderTextData.resourceIndex];
                            graphics.print(resource);
                        }
                    }
                }
            } else {
                printf("GraphicsServer command not recognized: '%d'\n", commandCharacter);     
            }
        }
    }
};
