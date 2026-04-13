// UART_Port.hpp
// #pragma once
#include "CH59x_common.h"

class UartSlavePort
{
    uint32_t TX_PIN = 0;
    uint32_t RX_PIN = 0;
    // --------------------------------
    enum State
    {
        STATE_IDLE, 
        STATE_WAIT,
        STATE_TX_COMMAND,
        STATE_RX_COMMAND, 
        STATE_TX_DATA,
        STATE_RX_DATA
    };
    volatile State state = STATE_IDLE;
    //---------------------------------
    volatile uint8_t* tx_buffer = nullptr;
    volatile uint8_t* rx_buffer = nullptr;  

    volatile uint16_t tx_length = 0;
    volatile uint16_t rx_length = 0; 
    volatile uint16_t echo_count = 0;

    uint8_t trig_point = 1; 
    //---------------------------------
    volatile uint8_t tx_command = 0;
    volatile uint8_t rx_command = 0; 
    static constexpr uint8_t HELLO = 0x55;
    //---------------------------------
    volatile bool connection = false; 

public: 
    UartSlavePort() {}
    void init(uint32_t tx_pin, uint32_t rx_pin) 
    {
        TX_PIN = tx_pin; 
        RX_PIN = rx_pin; 

        // §ß§Ñ§ã§ä§â§à§Û§Ü§Ñ §á§Ú§ß§à§Ó
        GPIOA_SetBits(TX_PIN);
        GPIOA_ModeCfg(RX_PIN, GPIO_ModeIN_PU);
        GPIOA_ModeCfg(TX_PIN, GPIO_ModeIN_PU);
        
        // §¯§Ñ§ã§ä§â§à§Û§Ü§Ñ §ã§Ñ§Þ§à§Ô§à UART (§ã§Ü§à§â§à§ã§ä§î, §æ§à§â§Þ§Ñ§ä)
        UART1_DefInit();

        // §¬§à§ß§æ§Ú§Ô§å§â§Ñ§è§Ú§ñ §á§â§Ö§â§í§Ó§Ñ§ß§Ú§Û
        UART1_ByteTrigCfg(UART_1BYTE_TRIG); 
        UART1_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
        
        // §£§Ü§Ý§ð§é§Ñ§Ö§Þ §Ó§Ö§Ü§ä§à§â §Ó §Ü§à§ß§ä§â§à§Ý§Ý§Ö§â§Ö §á§â§Ö§â§í§Ó§Ñ§ß§Ú§Û PFIC
        PFIC_EnableIRQ(UART1_IRQn);
    }

    void sendData(uint8_t command, uint8_t *buf, uint16_t length)
    {
        tx_command = command; 
        if (buf != nullptr) 
        {
            tx_length = length; 
            tx_buffer = buf+length; 
        }
        else
        {
            tx_length = 0; 
            tx_buffer = nullptr;
        }
    }
    
    void receiveData(uint8_t *buf, uint16_t length)
    {
        rx_buffer = buf+length; 
        rx_length = length; 

        if (state == STATE_WAIT) 
        {
            setTxCommandState(); 
            commandTransmit();            
        } 
    }

    void poll()
    {
        switch (state) {
            case STATE_IDLE: break; 
            case STATE_WAIT: break; 
            case STATE_TX_COMMAND: break; 
            case STATE_TX_DATA: break; 
            
            case STATE_RX_DATA: break; 
            case STATE_RX_COMMAND: break; 
        }
    }

    void interruptReceived() 
    {   
        switch (state) {
            case STATE_IDLE: 
                commandReceived(); 
                if (rx_command == HELLO || rx_length)
                {
                    setTxCommandState(); 
                    commandTransmit();
                }
                else state = STATE_WAIT; 
            break; 
            case STATE_WAIT: 
            break; 
            
            case STATE_TX_COMMAND:
                echoReceived(); 
                if (rx_length) setRxDataState(); 
                else if (tx_length) setRxCommandState();
                else setIdleState();  
            break;
            
            case STATE_RX_COMMAND: 
                commandReceived(); 
                setTxDataState(); 
                dataTransmit();
                
                break; 
            
            case STATE_RX_DATA: 
                dataReceived(); 
                if (!rx_length)
                {
                    if (tx_length) 
                    {setTxDataState(); 
                    dataTransmit();}
                    else setIdleState(); 
                }
                break; 
            case STATE_TX_DATA: 
                echoReceived();
                if (!echo_count) setIdleState();
                break; 
        }
    }

    bool isConnected()
    {
        return connection;
    }

    bool isTxReady()
    {
        return (state == STATE_RX_COMMAND);
    }
    
    uint8_t hasCommand()
    {
        if (state == STATE_WAIT)
            return rx_command; 
        else return 0; 
    }

private:
    void setTxCommandState()
    {
        state = STATE_TX_COMMAND; 
        GPIOA_ModeCfg(TX_PIN, GPIO_ModeOut_PP_5mA);
        trig_point = 1;
        UART1_ByteTrigCfg(UART_1BYTE_TRIG);
        echo_count = 1;          
    }
    void setRxCommandState()
    {
        state = STATE_RX_COMMAND; 
        GPIOA_ModeCfg(TX_PIN, GPIO_ModeIN_PU);
        trig_point = 1;
        UART1_ByteTrigCfg(UART_1BYTE_TRIG);  
    }
    
    void setTxDataState()
    {
        state = STATE_TX_DATA; 
        GPIOA_ModeCfg(TX_PIN, GPIO_ModeOut_PP_5mA);
        setTriggerPoint(tx_length); 
        echo_count = tx_length;
    }
    void setRxDataState()
    {
        state = STATE_RX_DATA; 
        GPIOA_ModeCfg(TX_PIN, GPIO_ModeIN_PU);
        setTriggerPoint(rx_length); 
    }
    void setIdleState()
    {
        state = STATE_IDLE; 
        GPIOA_ModeCfg(TX_PIN, GPIO_ModeIN_PU);
        tx_command = 0;
        rx_command = 0;
        tx_length = 0; 
        rx_length = 0;
        tx_buffer = nullptr; 
        rx_buffer = nullptr; 

        echo_count = 0; 
        trig_point = 1;
        UART1_ByteTrigCfg(UART_1BYTE_TRIG); 
    }
//------------------------------------------------------------------------------------
    void setTriggerPoint(uint16_t lenght)
    {
        if (lenght >= 7)
        {
            trig_point = 7; 
            UART1_ByteTrigCfg(UART_7BYTE_TRIG);        
        }
        else if (lenght >= 4)
        {
            trig_point = 4; 
            UART1_ByteTrigCfg(UART_4BYTE_TRIG);
        } 
        else if (lenght >= 2)
        {
            trig_point = 2; 
            UART1_ByteTrigCfg(UART_2BYTE_TRIG);
        }
        else
        {
            trig_point = 1; 
            UART1_ByteTrigCfg(UART_1BYTE_TRIG);
        }
    }
// ----------------------------------------------------------------------

    void commandTransmit()
    {
        if (tx_command > 0) R8_UART1_THR = tx_command;
        else R8_UART1_THR = HELLO;
        tx_command = 0; 
    }
    void dataTransmit()
    {
        // §à§ä§á§â§Ó§Ý§ñ§Ö§Þ §Ü§å§ã§à§Ü §Õ§Ñ§ß§ß§í§ç, §á§à§Þ§Ö§ë§Ñ§ð§ë§Ú§Û§ã§ñ §Ó FIFO 
        uint16_t frame_length = (tx_length > 8)? 8 : tx_length; 
        for (int i=0; i<frame_length; i++)
        {
            R8_UART1_THR = *(tx_buffer - tx_length); 
            tx_length--; 
        }  
    }
    void echoReceived()
    {
        for (int i=0; i<trig_point; i++)
        {
            // §ä§å§ä §Ö§ë§Ö §á§â§à§Ó§Ö§â§Ü§å §ß§Ñ §á§å§ã§ä§à§Û §Ò§å§æ§Ö§â 
            // §Ú §Ó§à§Ù§Þ§à§Ø§ß§à §ã§â§Ñ§Ó§ß§Ö§ß§Ú§Ö §ã §ä§Ö§Þ, §é§ä§à §à§ä§á§â§Ñ§Ó§Ý§ñ§Ý§Ú §Õ§Ý§ñ §à§Ò§ß§Ñ§â§å§Ø§Ö§ß§Ú§ñ §à§ê§Ú§Ò§à§Ü 
            R8_UART1_RBR; 
            echo_count--; 
        }
        if (tx_length > 0 && state == STATE_TX_DATA)
        {
            setTriggerPoint(tx_length);
            dataTransmit();
        } 
        else if (echo_count > 0) setTriggerPoint(echo_count); 
    }

    void commandReceived()
    {
        rx_command = R8_UART1_RBR;

    }
    void dataReceived()
    {
        if (rx_buffer == nullptr) return; 

        for (int i=0; i<trig_point; i++)
        {
            *(rx_buffer - rx_length) = R8_UART1_RBR; 
            rx_length--; 
        }
        setTriggerPoint(rx_length); 
        
    }
};