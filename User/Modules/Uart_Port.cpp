// #pragma once
#include "CH59x_common.h"
// §°§Ò§Ö§â§ä§Ü§Ñ §Õ§Ý§ñ GPIO

enum GpioPort {
    PORTA,
    PORTB
};
template<GpioPort PORT> struct GpioTraits;

template<> struct GpioTraits<PORTA> {
    static void mode(uint32_t pin, GPIOModeTypeDef m) { GPIOA_ModeCfg(pin,  m); }
    static void set(uint32_t pin) { GPIOA_SetBits(pin); }
    static void reset(uint32_t pin) { GPIOA_ResetBits(pin); }
};

template<> struct GpioTraits<PORTB> {
    static void mode(uint32_t pin, GPIOModeTypeDef m) { GPIOB_ModeCfg(pin, m); }
    static void set(uint32_t pin) { GPIOB_SetBits(pin); }
    static void reset(uint32_t pin) { GPIOB_ResetBits(pin); }
};

template <uint32_t BASE, GpioPort PORT, uint32_t TX_PIN, uint32_t RX_PIN, typename Derived>
class UartPort
{
    using Gpio = GpioTraits<PORT>;
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
    // --------------------------------
        static inline volatile uint8_t&  reg8(uint32_t offset)  { return *(volatile uint8_t*)(BASE + offset); }
        static inline volatile uint16_t& reg16(uint32_t offset) { return *(volatile uint16_t*)(BASE + offset); }

        static inline volatile uint8_t&  MCR() { return reg8(0x00); }
        static inline volatile uint8_t&  IER() { return reg8(0x01); }
        static inline volatile uint8_t&  FCR() { return reg8(0x02); }
        static inline volatile uint8_t&  LCR() { return reg8(0x03); }
        static inline volatile uint8_t&  LSR() { return reg8(0x05); }
        static inline volatile uint8_t&  RBR() { return reg8(0x08); }
        static inline volatile uint8_t&  THR() { return reg8(0x08); }
        static inline volatile uint16_t& DL()  { return reg16(0x0C); }
        static inline volatile uint8_t&  DIV() { return reg8(0x0E); }
public: ////////////////////////////////////////////////////////////////
    UartPort() {}
    void init(uint32_t baudrate = 115200) 
    {
        // §ß§Ñ§ã§ä§â§à§Û§Ü§Ñ §á§Ú§ß§à§Ó
        Gpio::set(TX_PIN); 
        Gpio::mode(RX_PIN, GPIO_ModeIN_PU); 
        Gpio::mode(TX_PIN, GPIO_ModeIN_PU); 
        // §¯§Ñ§ã§ä§â§à§Û§Ü§Ñ §ã§Ñ§Þ§à§Ô§à UART (§ã§Ü§à§â§à§ã§ä§î, §æ§à§â§Þ§Ñ§ä)
        UART_DefInit();
        UART_BaudRateCfg(baudrate);

        // §¬§à§ß§æ§Ú§Ô§å§â§Ñ§è§Ú§ñ §á§â§Ö§â§í§Ó§Ñ§ß§Ú§Û
        UART_ByteTrigCfg(UART_1BYTE_TRIG); 
        UART_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    }

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

        if (state == STATE_WAIT)
        {
            static_cast<Derived*>(this)->proceed();
        }
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
        // GPIOA_ModeCfg(TX_PIN, GPIO_ModeOut_PP_5mA);
        Gpio::mode(TX_PIN, GPIO_ModeOut_PP_5mA); 
        trig_point = 1;
        UART_ByteTrigCfg(UART_1BYTE_TRIG);
        echo_count = 1;          
    }
    void setRxCommandState()
    {
        state = STATE_RX_COMMAND; 
        // GPIOA_ModeCfg(TX_PIN, GPIO_ModeIN_PU);
        Gpio::mode(TX_PIN, GPIO_ModeIN_PU); 
        trig_point = 1;
        UART_ByteTrigCfg(UART_1BYTE_TRIG);  
    }
    
    void setTxDataState()
    {
        state = STATE_TX_DATA; 
        // GPIOA_ModeCfg(TX_PIN, GPIO_ModeOut_PP_5mA);
        Gpio::mode(TX_PIN, GPIO_ModeOut_PP_5mA);

        setTriggerPoint(tx_length); 
        echo_count = tx_length;
    }
    void setRxDataState()
    {
        state = STATE_RX_DATA; 
        GPIOA_ModeCfg(TX_PIN, GPIO_ModeIN_PU);
        Gpio::mode(TX_PIN, GPIO_ModeIN_PU);

        setTriggerPoint(rx_length); 
    }

    void setTriggerPoint(uint16_t length)
    {
        if (length >= 7)      { trig_point = 7; UART_ByteTrigCfg(UART_7BYTE_TRIG); }
        else if (length >= 4) { trig_point = 4; UART_ByteTrigCfg(UART_4BYTE_TRIG); }
        else if (length >= 2) { trig_point = 2; UART_ByteTrigCfg(UART_2BYTE_TRIG); }
        else                  { trig_point = 1; UART_ByteTrigCfg(UART_1BYTE_TRIG); }
    }
// --------------------------------------------------------------
    void commandTransmit()
    {
        THR() = (tx_command > 0) ? tx_command : HELLO;
    }

    void dataTransmit()
    {
        uint16_t frame_length = (tx_length > 8)? 8 : tx_length; 
        for (int i=0; i<frame_length; i++)
        {
            THR() = *(tx_buffer - tx_length); 
            tx_length--; 
        }  
    }
    void echoReceived()
    {
        for (int i=0; i<trig_point; i++)
        {
            volatile uint8_t unused = RBR();
            (void)unused;  
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
        rx_command = RBR();
    }
    void dataReceived()
    {
        for (int i=0; i<trig_point; i++)
        {
            *(rx_buffer - rx_length) = RBR(); 
            rx_length--; 
        }
        setTriggerPoint(rx_length);  
    }
// -----------------------------------------------------------------
// §³§Ú§ã§ä§Ö§Þ§ß§í§Ö §æ§å§ß§Ü§è§Ú§Ú 
    void UART_DefInit(void)
    {
        FCR() = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO
        LCR() = RB_LCR_WORD_SZ;
        IER() = RB_IER_TXD_EN;
        DIV() = 1;
    }
    void UART_BaudRateCfg(uint32_t baudrate)
    {
        uint32_t x;
        x = 10 * GetSysClock() / 8 / baudrate;
        x = (x + 5) / 10;
        DL() = (uint16_t)x;
    }
    void UART_ByteTrigCfg(UARTByteTRIGTypeDef b)
    {
        FCR() = (FCR() & ~RB_FCR_FIFO_TRIG) | (b << 6);
    }
    
    void UART_INTCfg(FunctionalState s, uint8_t i)
{
    if(s)
    {
        IER() |= i;
        MCR() |= RB_MCR_INT_OE;
    }
    else
    {
        IER() &= ~i;
    }
}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Master
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <uint32_t BASE, GpioPort PORT, uint32_t TX_PIN, uint32_t RX_PIN>
class UartMasterPort: public UartPort<BASE, PORT, TX_PIN, RX_PIN, UartMasterPort<BASE, PORT, TX_PIN, RX_PIN>>
{
    using Base = UartPort<BASE, PORT, TX_PIN, RX_PIN, UartMasterPort>;
    using State = typename Base::State;
    using Gpio = GpioTraits<PORT>;
    friend Base;    
private:
    void proceed()
    {
        if (this->tx_length) 
        {
            this->setTxDataState();
            this->dataTransmit();
        }
        else if (this->rx_length)
        {
            this->setTxCommandState(); 
            this->commandTransmit(); 
        }
        else setIdleState(); 
    }

    void setIdleState()
    {
        this->state = State::STATE_IDLE; 
        // GPIOA_ModeCfg(TX_PIN, GPIO_ModeIN_PU);
        Gpio::mode(TX_PIN, GPIO_ModeIN_PU);
        if (this->rx_command > 0) this->connection = true; 
        this->reset(); 
    }
public:
    using Base::init;
    using Base::sendData;
    using Base::receiveData;
    using Base::isConnected;
    using Base::hasCommand;
    void poll()
    {
        switch (this->state) {
            case State::STATE_IDLE: 
            this->setTxCommandState(); 
            this->commandTransmit(); break;
            
            case State::STATE_TX_COMMAND: break; 
            case State::STATE_TX_DATA: break; 
            case State::STATE_WAIT: break; 
            case State::STATE_RX_DATA:
            case State::STATE_RX_COMMAND: 
            if (!(this->LSR() & RB_LSR_DATA_RDY))
            {
                this->connection = false; 
                this->reset(); 
                this->setTxCommandState(); 
                this->commandTransmit();                
            }
            break; 
        }
    }
    
    void interruptReceived()
    {
        switch (this->state) {
            case State::STATE_IDLE: 
                // §Õ§à§Ò§Ñ§Ó§Ú§ä§î §ã§Ò§â§à§ã FIFO
                break; 
            
            case State::STATE_TX_COMMAND: 
                this->echoReceived();
                if (!this->rx_command) this->setRxCommandState(); 
                else if (this->rx_length) this->setRxDataState(); 
                break; 
            
            case State::STATE_RX_COMMAND: 
                this->commandReceived();
                if (this->rx_command == Base::HELLO)
                {
                    if (this->tx_length) 
                    {
                        this->setTxDataState();
                        this->dataTransmit(); 
                    }
                    else this->setIdleState(); 
                }
                else
                {
                    if (!this->rx_length) this->state = State::STATE_WAIT; 
                    else if (this->tx_length)
                    {
                        this->setTxDataState();
                        this->dataTransmit();                         
                    }
                    else
                    {
                        this->setTxCommandState(); 
                        this->commandTransmit(); 
                    }
                }

            break;
            case State::STATE_WAIT: break; 
            case State::STATE_TX_DATA:
                this->echoReceived();
                if (!this->echo_count) 
                {
                    if (this->rx_length) this->setRxDataState();
                    else this->setIdleState(); 
                }
                break; 
            case State::STATE_RX_DATA: 
                this->dataReceived(); 
                if (!this->rx_length) this->setIdleState(); 
                break;
        }
    }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Slave
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <uint32_t BASE, GpioPort PORT, uint32_t TX_PIN, uint32_t RX_PIN>
class UartSlavePort: public UartPort<BASE, PORT, TX_PIN, RX_PIN, UartSlavePort<BASE, PORT, TX_PIN, RX_PIN>>
{
    using Base = UartPort<BASE, PORT, TX_PIN, RX_PIN, UartSlavePort>;
    using State = typename Base::State;
    using Gpio = GpioTraits<PORT>;
    friend Base;

private:
    void proceed()
    {
        this->setTxCommandState(); 
        this->commandTransmit();  
    }

    void setIdleState()
    {
        this->state = State::STATE_IDLE; 
        GPIOA_ModeCfg(TX_PIN, GPIO_ModeIN_PU);
        Gpio::mode(TX_PIN, GPIO_ModeIN_PU);

        this->connection = (this->rx_command)? true : false;  

        this->echo_count = 0; 
        this->trig_point = 1;
        this->UART_ByteTrigCfg(UART_1BYTE_TRIG); 
    }
public:
    void poll()
    {
        switch (this->state) {
            case State::STATE_IDLE: 
            this->connection = (this->rx_command)? true : false; 
            this->reset();
            break; 
            case State::STATE_WAIT: break; 
            case State::STATE_TX_COMMAND: break; 
            case State::STATE_TX_DATA: break; 
            
            case State::STATE_RX_DATA: 
            case State::STATE_RX_COMMAND: 
            if (!(this->LSR() & RB_LSR_DATA_RDY))
            {
                setIdleState(); 
                this->reset(); 
                this->connection = false;                
            }
            break; 
        }
    }
    
    void interruptReceived()
    {
        switch (this->state) {
            case State::STATE_IDLE: 
                this->commandReceived(); 
                if (this->rx_command != Base::HELLO && !this->rx_length) this->state = State::STATE_WAIT; 
                else
                {
                    this->setTxCommandState(); 
                    this->commandTransmit(); 
                }
                break; 
            case State::STATE_WAIT: break; 
            case State::STATE_TX_COMMAND:
                this->echoReceived(); 
                if (this->rx_length) this->setRxDataState(); 
                else if (this->tx_length) this->setRxCommandState();
                else setIdleState();  
                break;
            
            case State::STATE_RX_COMMAND: 
                this->commandReceived(); 
                this->setTxDataState(); 
                this->dataTransmit();
                
                break; 
            
            case State::STATE_RX_DATA: 
                this->dataReceived(); 
                if (!this->rx_length)
                {
                    if (this->tx_length) 
                    {
                        this->setTxDataState(); 
                        this->dataTransmit();
                    }
                    else setIdleState(); 
                }
                break; 
            case State::STATE_TX_DATA: 
                this->echoReceived();
                if (!this->echo_count) setIdleState();
                break; 
        }
    }
};