#include "CH59x_common.h"
#include "Modules/UART_Slave_Port.hpp"
#include "Modules/UART_Master_Port.hpp"

// PA9 - TX, PA8 - RX.    
UartSlavePort UART1;

uint8_t TxBuff[] = {0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF,
                    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF,
                    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF};

uint8_t RxBuff[24]; 

uint8_t command = 0;

int main() 
{
    // §ª§ß§Ú§è§Ú§Ñ§Ý§Ú§Ù§Ñ§è§Ú§ñ §ã§Ú§ã§ä§Ö§Þ§í §Ú §ä§Ñ§Ü§ä§Ú§â§à§Ó§Ñ§ß§Ú§ñ (60 §®§¤§è)
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    
    // §¬§à§ß§æ§Ú§Ô§å§â§Ñ§è§Ú§ñ §á§Ú§ß§à§Ó UART (§Ú§ã§á§à§Ý§î§Ù§å§Ö§Þ §ä§Ó§à§Û §â§Ñ§Ò§à§é§Ú§Û §Þ§Ö§ä§à§Õ §ã §Ñ§â§Ô§å§Þ§Ö§ß§ä§Ñ§Þ§Ú)
    UART1.init(GPIO_Pin_9, GPIO_Pin_8); 

    // §¯§Ñ§ã§ä§â§à§Û§Ü§Ñ §Ú§ß§Õ§Ú§Ü§Ñ§ä§à§â§ß§à§Ô§à §á§Ú§ß§Ñ B15
    GPIOB_ModeCfg(GPIO_Pin_15, GPIO_ModeOut_PP_5mA);
    GPIOB_ResetBits(GPIO_Pin_15);

// --- §¯§Ñ§ã§ä§â§à§Û§Ü§Ñ §´§Ñ§Û§Þ§Ö§â§Ñ 0 §Õ§Ý§ñ §Ó§í§Ù§à§Ó§Ñ poll §Ü§Ñ§Ø§Õ§í§Ö 15 §Þ§ã ---
    
    // 1. §£§Ü§Ý§ð§é§Ñ§Ö§Þ §ä§Ñ§Ü§ä§Ú§â§à§Ó§Ñ§ß§Ú§Ö §ä§Ñ§Û§Þ§Ö§â§Ñ (§Ö§ã§Ý§Ú §ä§â§Ö§Ò§å§Ö§ä§ã§ñ §Ó §Ó§Ñ§ê§Ö§Û §Ó§Ö§â§ã§Ú§Ú SDK)
    // TMR0_ITCfg(ENABLE, RB_TMR_IE_CYC_END); 

    // 2. §ª§ß§Ú§è§Ú§Ñ§Ý§Ú§Ù§Ñ§è§Ú§ñ. §¦§ã§Ý§Ú TMR0_TimerInit §ß§Ö §ß§Ñ§ç§à§Õ§Ú§ä§ã§ñ, 
    // §Ú§ã§á§à§Ý§î§Ù§å§Ö§Þ §á§â§ñ§Þ§å§ð §ß§Ñ§ã§ä§â§à§Û§Ü§å §é§Ö§â§Ö§Ù §â§Ö§Ô§Ú§ã§ä§â§í §Ú§Ý§Ú §Ñ§Ý§î§ä§Ö§â§ß§Ñ§ä§Ú§Ó§ß§à§Ö §Ú§Þ§ñ:
    R8_TMR0_CTRL_MOD = RB_TMR_ALL_CLEAR;     // §³§Ò§â§à§ã §ä§Ñ§Û§Þ§Ö§â§Ñ
    R8_TMR0_CTRL_MOD = RB_TMR_COUNT_EN;      // §²§Ö§Ø§Ú§Þ §ã§é§Ö§ä§Ñ
    R32_TMR0_CNT_END = 900000;               // §µ§ã§ä§Ñ§ß§à§Ó§Ü§Ñ §á§Ö§â§Ú§à§Õ§Ñ (15 §Þ§ã §á§â§Ú 60§®§¤§è)
    
    // 3. §£§Ü§Ý§ð§é§Ñ§Ö§Þ §á§â§Ö§â§í§Ó§Ñ§ß§Ú§Ö §á§à §à§Ü§à§ß§é§Ñ§ß§Ú§ð §ã§é§Ö§ä§Ñ
    R8_TMR0_INTER_EN = RB_TMR_IE_CYC_END;

    PFIC_EnableIRQ(TMR0_IRQn);             // §£§Ü§Ý§ð§é§Ö§ß§Ú§Ö §á§â§Ö§â§í§Ó§Ñ§ß§Ú§ñ TMR0 §Ó §Ü§à§ß§ä§â§à§Ý§Ý§Ö§â§Ö

    while(1) 
    {
        command = UART1.hasCommand();
        if (command > 0) UART1.receiveData(RxBuff, command); 

        if (UART1.isConnected())
        {
            GPIOB_SetBits(GPIO_Pin_15); // §ª§ß§Ó§Ö§â§ã§Ú§ñ §ã§à§ã§ä§à§ñ§ß§Ú§ñ §á§Ú§ß§Ñ
        }
        else
        {
            GPIOB_ResetBits(GPIO_Pin_15);   // §£§í§Ü§Ý§ð§é§Ö§ß, §Ö§ã§Ý§Ú §ß§Ö§ä §ã§Ó§ñ§Ù§Ú
        }
    }

}

extern "C" {

    /*********************************************************************
     * §°§Ò§â§Ñ§Ò§à§ä§é§Ú§Ü §á§â§Ö§â§í§Ó§Ñ§ß§Ú§ñ §´§Ñ§Û§Þ§Ö§â§Ñ 0 (§Ü§Ñ§Ø§Õ§í§Ö 15 §Þ§ã)
     *********************************************************************/
    __INTERRUPT
    __HIGH_CODE
    void TMR0_IRQHandler(void) 
    {
        if (TMR0_GetITFlag(RB_TMR_IF_CYC_END)) 
        {
            TMR0_ClearITFlag(RB_TMR_IF_CYC_END); // §³§Ò§â§à§ã §æ§Ý§Ñ§Ô§Ñ §á§â§Ö§â§í§Ó§Ñ§ß§Ú§ñ

            // §£§í§á§à§Ý§ß§ñ§Ö§Þ §Ý§à§Ô§Ú§Ü§å §à§á§â§à§ã§Ñ §á§à §ä§Ñ§Û§Þ§Ö§â§å

            command = 0; 
            UART1.poll(); 
        }
    }

    /*********************************************************************
     * §°§Ò§â§Ñ§Ò§à§ä§é§Ú§Ü §á§â§Ö§â§í§Ó§Ñ§ß§Ú§ñ UART1
     *********************************************************************/
    __INTERRUPT
    __HIGH_CODE
    void UART1_IRQHandler(void) 
    {
        uint8_t int_status = UART1_GetITFlag();

        switch(int_status) 
        {
            case UART_II_LINE_STAT: 
                UART1_GetLinSTA(); 
                break;

            case UART_II_RECV_RDY: 
                UART1.interruptReceived(); 
                break;

            case UART_II_RECV_TOUT: 
                // §£§í§é§Ú§ä§í§Ó§Ñ§Ö§Þ §à§ã§ä§Ñ§ä§Ü§Ú §á§â§Ú §ä§Ñ§Û§Þ§Ñ§å§ä§Ö
                while(R8_UART1_LSR & RB_LSR_DATA_RDY) 
                {
                    UART1_RecvByte();
                }
                break;

            case UART_II_THR_EMPTY:
                break;

            default:
                break;
        }
    }
}