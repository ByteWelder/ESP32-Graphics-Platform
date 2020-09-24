#pragma once

#define BUFFER_SIZE 1024

#define UART_CLIENT_TXD  (GPIO_NUM_4)
#define UART_CLIENT_RXD  (GPIO_NUM_5)
#define UART_CLIENT_RTS  (UART_PIN_NO_CHANGE)
#define UART_CLIENT_CTS  (UART_PIN_NO_CHANGE)

class UartClient {

private:

    uart_port_t uartNum;
    
public:

    UartClient(uart_port_t uartNum) {
        this->uartNum = uartNum;
        init();
    }

    UartClient() {
        this->uartNum = UART_NUM_1;
        init();
    }

    bool writeUint8(uint8_t data) {
        return uart_write_bytes(uartNum, (const char*) &data, 1) == 1;
    }
    
    bool writeUint16(uint16_t data) {
        return uart_write_bytes(uartNum, (const char*) &data, 2) == 2;
    }
    
    bool writeUint32(uint32_t data) {
        return uart_write_bytes(uartNum, (const char*) &data, 4) == 4;
    }

    bool write(const void* data, size_t byteCount) {
        return uart_write_bytes(uartNum, (const char*) data, byteCount) == byteCount;
    }

    bool read(void* bytes, size_t bytecount) {
        return uart_read_bytes(uartNum, (uint8_t*) bytes, bytecount, 10 / portTICK_RATE_MS) == bytecount;
    }

    bool readUint8(uint8_t* number) {
        return read(number, 1) == 1;
    }

    bool readUint16(uint16_t* number) {
        return read(number, 2) == 2;
    }
    
    bool readUint32(uint32_t* number) {
        return read(number, 4) == 4;
    }
    
    bool waitUntilTxDone() {
        return uart_wait_tx_done(uartNum, 200 / portTICK_RATE_MS) == ESP_OK;
    }

    bool flushRxBuffer() {
        return uart_flush_input(uartNum) == ESP_OK;
    }

private:

    void init() {
        uart_config_t uart_config = {
            .baud_rate = 4000000,
            .data_bits = UART_DATA_8_BITS,
            .parity    = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
        };
        
        uart_param_config(uartNum, &uart_config);
        uart_set_pin(uartNum, UART_CLIENT_TXD, UART_CLIENT_RXD, UART_CLIENT_RTS, UART_CLIENT_CTS);
        uart_driver_install(uartNum, BUFFER_SIZE * 2, 0, 0, NULL, 0);
    }

};
