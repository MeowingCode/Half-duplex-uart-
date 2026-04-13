// UART_Port.hpp
#pragma once
#include "CH59x_common.h"

class UART_Port
{
public:
    UART_Port(uint32_t tx_pin, uint32_t rx_pin, bool is_master = false)
        : TX_PIN(tx_pin), RX_PIN(rx_pin), master(is_master)
    {
        GPIOA_SetBits(TX_PIN);
        GPIOA_ModeCfg(RX_PIN, GPIO_ModeIN_PU);
        GPIOA_ModeCfg(TX_PIN, GPIO_ModeOut_PP_5mA);

        UART_DefInit();

        state = master ? STATE_TX_IDLE : STATE_RX_IDLE;
    }

    // вызывается по таймеру
    void poll()
    {
        if(master)
        {
            masterProcess();
        }
        else
        {
            slaveProcess();
        }
    }

    bool isConnected() const
    {
        return connection;
    }

private:
    // ---------------- CONFIG ----------------
    uint32_t TX_PIN;
    uint32_t RX_PIN;
    bool master;

    // ---------------- STATE ----------------
    enum State
    {
        STATE_TX_IDLE,
        STATE_SENDING,
        STATE_WAIT_RX,
        STATE_RX_IDLE
    };

    State state;
    bool connection = false;

    static constexpr uint8_t HELLO = 0x55;

    // ---------------- MASTER ----------------
    void masterProcess()
    {
        switch(state)
        {
            case STATE_TX_IDLE:
                sendHello();
                connection = false;
                state = STATE_WAIT_RX;
                break;

            case STATE_WAIT_RX:
                if(isByteReceived())
                {
                    uint8_t b = readByte();
                    if(b == HELLO)
                    {
                        connection = true;
                    }
                }
                else {connection = false;}
                state = STATE_TX_IDLE;
                break;

            default:
                break;
        }
    }

    // ---------------- SLAVE ----------------
    void slaveProcess()
    {
        switch(state)
        {
            case STATE_RX_IDLE:
                if(isByteReceived())
                {
                    uint8_t b = readByte();
                    if(b == HELLO)
                    {
                        connection = true;
                        sendHello(); // ответ
                    }
                }
                break;

            default:
                break;
        }
    }

    // ---------------- UART ----------------

    void sendHello()
    {
        // кладём байт в FIFO
        // R8_UART1_THR Transmit hold reg (FIFO)
        R8_UART1_THR = HELLO;

        // ждём ПОЛНОГО окончания передачи
        // R8_UART1_LSR - регистр состояния
        // while(!(R8_UART1_LSR & RB_LSR_TX_ALL_EMP));
    }

    bool isByteReceived()
    {
        return (R8_UART1_LSR & RB_LSR_DATA_RDY);
    }

    uint8_t readByte()
    {
        return R8_UART1_RBR; // автоматически вытаскивает из FIFO
    }

    // ---------------- INIT ----------------

    void UART_DefInit(void)
    {
        UART_BaudRateCfg(115200);
        R8_UART1_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        R8_UART1_LCR = RB_LCR_WORD_SZ;
        R8_UART1_IER = RB_IER_TXD_EN;
        R8_UART1_DIV = 1;
    }

    void UART_BaudRateCfg(uint32_t baudrate)
    {
        uint32_t x;
        x = 10 * GetSysClock() / 8 / baudrate;
        x = (x + 5) / 10;
        R16_UART1_DL = (uint16_t)x;
    }
};