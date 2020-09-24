#pragma once

#include "driver/uart.h"
#include "GraphicsProtocol.h"
#include "UartClient.h"

class GraphicsClient
{ 
private:

    UartClient uart;
    bool connected = false;
    RenderText renderTextData;

public:

    GraphicsClient() {
        connected = false;
    }

    bool tryConnect() {
        printf("GraphicsClient connect\n");
        uart.flushRxBuffer();
        if (!uart.writeUint8((uint8_t) Command::INIT)) {
            printf("failed to send init\n");
            return false;
        }
        
        if (!uart.waitUntilTxDone()) {
            printf("failed to wait for tx_done\n");
            return false;
        }
        
        InitResponse response;
        connected = receiveInitResponse(uart, response);
        return connected;
    }

    bool isConnected() {
        return connected;
    }

    void disconnect() {
        printf("GraphicsClient disconnect\n");
        uart.flushRxBuffer();
        connected = false;
    }
    
    void sceneBegin() {
        sendSceneBegin(uart);
    }

    void setResource(uint8_t index, const void* data, uint8_t dataLength) {
        sendResourceSet(uart, index, data, dataLength);
    }

    void setResourceText(uint8_t resourceIndex, const char* text) {
        return setResource(resourceIndex, (void*) text, strlen(text) + 1);
    }

    void renderText(int resourceIndex, uint8_t posX, uint8_t posY, uint8_t color) {
        renderTextData.resourceIndex = resourceIndex;
        renderTextData.color = color;
        renderTextData.positionX = posX;
        renderTextData.positionY = posY;
        sendRenderText(uart, renderTextData);
    }

    void sceneEnd() {
        if (!sendSceneEnd(uart)) {
            disconnect();
        }
    }
};
