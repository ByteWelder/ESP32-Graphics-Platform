#pragma once

#include "UartClient.h"

#define GRAPHICS_PROTOCOL_VERSION_MAJOR 1
#define GRAPHICS_PROTOCOL_VERSION_MINOR 0
#define GRAPHICS_PROTOCOL_VERSION_PATCH 0

#define UART_CLIENT_TXD  (GPIO_NUM_4)
#define UART_CLIENT_RXD  (GPIO_NUM_5)
#define UART_CLIENT_RTS  (UART_PIN_NO_CHANGE)
#define UART_CLIENT_CTS  (UART_PIN_NO_CHANGE)

enum class Command {
  INIT = 1,
  SCENE_BEGIN,
  SCENE_END,
  SET_RESOURCE,
  SET_TEXT_COLOR,
  RENDER_TEXT_RESOURCE
};

struct InitResponse {
    uint8_t echo = 0;
    uint8_t versionMajor = 0;
    uint8_t versionMinor = 0;
    uint8_t versionPatch = 0;
    uint32_t memoryAvailable = 0;
};

struct RenderText {
    uint8_t resourceIndex = 0;
    uint8_t color = 5;
    uint8_t positionX = 0;
    uint8_t positionY = 0;
};

//
// HOST
//

inline bool sendInitResponse(UartClient& uart) {
    uint32_t freeMemory = (uint32_t) heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    return uart.writeUint8((uint8_t) Command::INIT)
        && uart.writeUint8(GRAPHICS_PROTOCOL_VERSION_MAJOR)
        && uart.writeUint8(GRAPHICS_PROTOCOL_VERSION_MINOR)
        && uart.writeUint8(GRAPHICS_PROTOCOL_VERSION_PATCH)
        && uart.writeUint32(freeMemory)
        && uart.waitUntilTxDone();
}

inline bool sendSceneEndVerification(UartClient& uart) {
    uart.writeUint8((uint8_t) Command::SCENE_END);
    return uart.waitUntilTxDone();
}

inline bool receiveResource(UartClient& uart, void* resources[]) {
    int receivedBytes = 0;
    uint8_t resId = 255;
    uint8_t length = 0;
    receivedBytes = uart_read_bytes(UART_NUM_1, &resId, 1, 20 / portTICK_RATE_MS);
    receivedBytes = uart_read_bytes(UART_NUM_1, &length, 1, 20 / portTICK_RATE_MS);
    if (resources[resId] != NULL) {
        free(resources[resId]);
    }
    resources[resId] = malloc(length);
    return uart_read_bytes(UART_NUM_1, (unsigned char *) resources[resId], length, 20 / portTICK_RATE_MS) == length;
}

inline bool receiveRenderText(UartClient& uart, uint8_t* resourceIndex) {
    int receivedBytes = 0;
    receivedBytes = uart_read_bytes(UART_NUM_1, resourceIndex, 1, 20 / portTICK_RATE_MS);
}

inline bool receiveRenderText(UartClient& uart, RenderText& data) {
    return uart.readUint8(&(data.resourceIndex)) == 1
        && uart.readUint8(&(data.color)) == 1
        && uart.readUint8(&(data.positionX)) == 1
        && uart.readUint8(&(data.positionY)) == 1;
}

//
// CLIENT
//

inline bool receiveInitResponse(UartClient& uart, InitResponse& response) {
    bool hasPingResponse = uart.readUint8(&(response.echo));
    uart.readUint8(&(response.versionMajor));
    uart.readUint8(&(response.versionMinor));
    uart.readUint8(&(response.versionPatch));
    uart.readUint32(&(response.memoryAvailable));
    
    if (!hasPingResponse) {
        printf("error: no ping response\n");
        return false;
    } else if (response.echo != (uint8_t) Command::INIT) {
        printf("error: init ping incorrect\n");
        return false;
    } else if (response.versionMajor != GRAPHICS_PROTOCOL_VERSION_MAJOR) {
        printf("error: init protocol major version mismatch (%d vs %d)\n", response.versionMajor, GRAPHICS_PROTOCOL_VERSION_MAJOR);
        return false;
    } else if (response.versionMinor < GRAPHICS_PROTOCOL_VERSION_MINOR) {
        printf("error: init protocol minor version mismatch (%d vs %d)\n", response.versionMajor, GRAPHICS_PROTOCOL_VERSION_MINOR);
        return false;
    } else if (response.memoryAvailable < 1000) {
        printf("error: init memory insufficient\n");
        return false;
    } else {
        const char* printFormat =
          "init response:\n"
          " - remote protocol version: %d.%d.%d\n"
          " - memory available: %d\n";
        printf(printFormat, response.versionMajor, response.versionMinor, response.versionPatch, response.memoryAvailable);
        return true;
    }
}

inline void sendSceneBegin(UartClient& uart) {
    uart.writeUint8((uint8_t) Command::SCENE_BEGIN);
}

inline bool sendSceneEnd(UartClient& uart) {
    uart.writeUint8((uint8_t) Command::SCENE_END);
    if (!uart.waitUntilTxDone()) {
        return false;
    }
    uint8_t echo = 0;
    if (!uart.readUint8(&echo)) {
        printf("GraphicsClient sceneEnd echo missing\n");
        return false;
    }

    if (echo != (uint8_t) Command::SCENE_END) {
        printf("GraphicsClient sceneEnd echo mismatch\n");
        return false;
    }

    return true;
}

inline void sendResourceSet(UartClient& uart, uint8_t resourceIndex, const void* data, uint8_t dataLength) {
    uart.writeUint8((uint8_t) Command::SET_RESOURCE);
    uart.writeUint8(resourceIndex);
    uart.writeUint8(dataLength);
    uart.write(data, dataLength);
}

inline void sendRenderText(UartClient& uart, const RenderText& data) {
    uart.writeUint8((uint8_t) Command::RENDER_TEXT_RESOURCE);
    uart.writeUint8(data.resourceIndex);
    uart.writeUint8(data.color);
    uart.writeUint8(data.positionX);
    uart.writeUint8(data.positionY);
}
