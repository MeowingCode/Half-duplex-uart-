// #pragma once
#include "CH59x_common.h"

class UartPort
{
public:
        enum State
    {
        STATE_IDLE,
        STATE_WAIT,
        STATE_TX_COMMAND,
        STATE_RX_COMMAND, 
        STATE_TX_DATA,
        STATE_RX_DATA
    };
protected:
    uint32_t TX_PIN = 0;
    uint32_t RX_PIN = 0;
    // --------------------------------
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

public: ////////////////////////////////////////////////////////////////
    UartPort() {}
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
// ------------------------------------------------------------------
    virtual void poll() = 0; 
    virtual void interruptReceived() = 0; 
// -------------------------------------------------------------------
    void sendData(uint8_t command, uint8_t* buf, uint16_t length)
    {
        tx_command = command;
        tx_length  = buf ? length : 0;
        tx_buffer  = buf ? buf + length : nullptr;
    }
    
    void receiveData(uint8_t *buf, uint16_t length)
    {
        rx_length = buf ? length : 0;
        rx_buffer = buf ? buf + length : nullptr; 
        if (state == STATE_WAIT) proceed(); 
    }
// ----------------------------------------------------------------------
    bool isConnected()
    {
        return connection;
    }

    bool isTxReady()
    {
        return (state == STATE_IDLE && !tx_command);
    }    
    
    uint8_t hasCommand()
    {
        if (state == STATE_WAIT)
            return rx_command; 
        else return 0; 
    }

protected: ////////////////////////////////////////////////////////
    
    virtual void proceed() = 0;

    virtual void setIdleState() = 0; 

    void reset()
    {
        tx_command = 0;
        tx_length = 0; 
        tx_buffer = nullptr; 
        
        rx_command = 0;
        rx_length = 0;
        rx_buffer = nullptr; 

        echo_count = 0; 
    }

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
// --------------------------------------------------------------
    void commandTransmit()
    {
        if (tx_command > 0) R8_UART1_THR = tx_command;
        else R8_UART1_THR = HELLO;
    }

    void dataTransmit()
    {
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
        for (int i=0; i<trig_point; i++)
        {
            *(rx_buffer - rx_length) = R8_UART1_RBR; 
            rx_length--; 
        }
        setTriggerPoint(rx_length);  
    }
};

/////////////////////////////////////////////////////////////
// Master
////////////////////////////////////////////////////////////
class UartMasterPort: public UartPort
{
    using State = UartPort::State;
private:
    void proceed() override
    {
        if (tx_length) 
        {
            setTxDataState();
            dataTransmit();
        }
        else if (rx_length)
        {
            setTxCommandState(); 
            commandTransmit(); 
        }
        else setIdleState(); 
    }

    void setIdleState() override
    {
        state = STATE_IDLE; 
        GPIOA_ModeCfg(TX_PIN, GPIO_ModeIN_PU);
        if (rx_command > 0) connection = true; 
        reset(); 
    }
public:
    void poll() override
    {
        switch (state) {
            case STATE_IDLE: 
            setTxCommandState(); 
            commandTransmit(); break;
            
            case STATE_TX_COMMAND: break; 
            case STATE_TX_DATA: break; 
            case STATE_WAIT: break; 
            case STATE_RX_DATA:
            case STATE_RX_COMMAND: 
            if (!(R8_UART1_LSR & RB_LSR_DATA_RDY))
            {
                connection = false; 
                reset(); 
                setTxCommandState(); 
                commandTransmit();                
            }
            break; 
        }
    }

    void interruptReceived() override
    {
        switch (state) {
            case STATE_IDLE: 
                // §Õ§à§Ò§Ñ§Ó§Ú§ä§î §ã§Ò§â§à§ã FIFO
                break; 
            
            case STATE_TX_COMMAND: 
                echoReceived();
                if (!rx_command) setRxCommandState(); 
                else if (rx_length) setRxDataState(); 
                break; 
            
            case STATE_RX_COMMAND: 
                commandReceived();
                if (rx_command == HELLO)
                {
                    if (tx_length) 
                    {
                        setTxDataState();
                        dataTransmit(); 
                    }
                    else setIdleState(); 
                }
                else
                {
                    if (!rx_length) state = STATE_WAIT; 
                    else if (tx_length)
                    {
                        setTxDataState();
                        dataTransmit();                         
                    }
                    else
                    {
                        setTxCommandState(); 
                        commandTransmit(); 
                    }
                }

            break;
            case STATE_WAIT: break; 
            case STATE_TX_DATA:
                echoReceived();
                if (!echo_count) 
                {
                    if (rx_length) setRxDataState();
                    else setIdleState(); 
                }
                break; 
            case STATE_RX_DATA: 
                dataReceived(); 
                if (!rx_length) setIdleState(); 
                break;
        }
    }

};

///////////////////////////////////////////////////////////////////////////////////
// Slave
///////////////////////////////////////////////////////////////////////////////////
class UartSlavePort: public UartPort
{
    using State = UartPort::State;
private:
    void proceed() override
    {
        setTxCommandState(); 
        commandTransmit();  
    }
    
    void setIdleState() override
    {
        state = STATE_IDLE; 
        GPIOA_ModeCfg(TX_PIN, GPIO_ModeIN_PU);
        connection = (rx_command)? true : false;  

        echo_count = 0; 
        trig_point = 1;
        UART1_ByteTrigCfg(UART_1BYTE_TRIG); 
    }
public:
    void poll() override
    {
        switch (state) {
            case STATE_IDLE: 
            // connection = (rx_command)? true : false; 
            reset();
            break; 
            case STATE_WAIT: break; 
            case STATE_TX_COMMAND: break; 
            case STATE_TX_DATA: break; 
            
            case STATE_RX_DATA: 
            case STATE_RX_COMMAND: 
            if (!(R8_UART1_LSR & RB_LSR_DATA_RDY))
            {
                setIdleState(); 
                reset(); 
                connection = false;                
            }
            break; 
        }
    }

    void interruptReceived() override
    {
        switch (state) {
            case STATE_IDLE: 
                commandReceived(); 
                if (rx_command != HELLO && !rx_length) state = STATE_WAIT; 
                else
                {
                    setTxCommandState(); 
                    commandTransmit(); 
                }
                break; 
            case STATE_WAIT: break; 
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
                    {
                        setTxDataState(); 
                        dataTransmit();
                    }
                    else setIdleState(); 
                }
                break; 
            case STATE_TX_DATA: 
                echoReceived();
                if (!echo_count) setIdleState();
                break; 
        }
    }

};