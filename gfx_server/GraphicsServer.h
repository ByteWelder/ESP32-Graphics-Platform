#pragma once
#include "CompositeGraphics.h"
#include "GraphicsProtocol.h"
#include "UartClient.h"

//#define DEBUG_GRAPHICS

#ifdef DEBUG_GRAPHICS
#define logDebug(x) printf(x)
#define logDebug2(x, y) printf(x, y)
#define delayForDebug() delay(1000)
#else
#define logDebug(x)
#define logDebug2(x, y)
#define delayForDebug()
#endif
#define logInfo(x) printf(x)

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
        uint8_t commandCharacter = 0;
        if (uart.readUint8(&commandCharacter)) {
            if (commandCharacter == (uint8_t) Command::INIT) {
                logInfo("GraphicsServer: init started\n");
                uint32_t freeMemory = (uint32_t) heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
                bool allWritten = sendInitResponse(uart);
                if (allWritten) {
                    logInfo("GraphicsServer: init finished\n");
                } else {
                    logInfo("Error: GraphicsServer: init failed\n");
                }
            } else if (commandCharacter == (uint8_t) Command::SCENE_BEGIN) {
                logDebug("scene begin\n");
                sceneStarted = true;
                graphics.begin(54);
            } else if (commandCharacter == (uint8_t) Command::SCENE_END) {
                logDebug("scene end\n");
                graphics.end();
                sendSceneEndVerification(uart);
                sceneStarted = false;
            } else if (commandCharacter == (uint8_t) Command::SET_RESOURCE) {
                logDebug("set resource\n");
                receiveResource(uart, resources);
            } else if (commandCharacter == (uint8_t) Command::RENDER_TEXT_RESOURCE) { // render text resource
                logDebug("render text resource\n");
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
                logDebug2("GraphicsServer command not recognized: '%d'\n", commandCharacter);     
            }
        }
    }
};
