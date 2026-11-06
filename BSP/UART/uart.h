#ifndef UART_H
#define UART_H

#include "ti_msp_dl_config.h"

/* 一帧的字节数，最多返回一共-2字节，按你实际协议改 */
#define UART_RX_FRAME_SIZE   (8U)

extern volatile uint8_t uart_rx_buf[UART_RX_FRAME_SIZE];
extern volatile bool    uart_rx_done;   /* 收到一整帧后由中断置 true */

void UART_Send_char(UART_Regs *uart, const uint8_t chr);

void UART_Send_String(UART_Regs *uart, const char* str);

void UART_Send_Buf(UART_Regs *uart, const uint8_t *buf, uint32_t len);

/* 上电初始化一次：关掉逐字节中断，配好 DMA 并启动 */
void UART_DMA_RX_Init(void);

/* 处理完一帧后调用，重新装填缓冲区 */
void UART_DMA_RX_Rearm(void);

#endif // !UART_H

