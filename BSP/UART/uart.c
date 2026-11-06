#include "uart.h"

/* 用的 DMA 通道，本项目没用别的 DMA，CH0 就行 */
#define UART_RX_DMA_CH   (0U)

volatile uint8_t uart_rx_buf[UART_RX_FRAME_SIZE];
volatile bool    uart_rx_done = false;

/* DMA 通道配置：UART0_RX -> 内存，字节宽度，目标地址自增 */
static const DL_DMA_Config uart_rx_dma_cfg = {
    .transferMode  = DL_DMA_SINGLE_TRANSFER_MODE,
    .extendedMode  = DL_DMA_NORMAL_MODE,
    .destIncrement = DL_DMA_ADDR_INCREMENT,
    .srcIncrement  = DL_DMA_ADDR_UNCHANGED,
    .destWidth     = DL_DMA_WIDTH_BYTE,
    .srcWidth      = DL_DMA_WIDTH_BYTE,
    .trigger       = DMA_UART0_RX_TRIG,          /* 注意：UART0 是 17 */
    .triggerType   = DL_DMA_TRIGGER_TYPE_EXTERNAL,
};

static void uart_dma_rx_start(void)
{
    DL_DMA_setSrcAddr(DMA, UART_RX_DMA_CH, (uint32_t)(&PRINT_INST->RXDATA));
    DL_DMA_setDestAddr(DMA, UART_RX_DMA_CH, (uint32_t)&uart_rx_buf[0]);
    DL_DMA_setTransferSize(DMA, UART_RX_DMA_CH, UART_RX_FRAME_SIZE);
    DL_DMA_enableChannel(DMA, UART_RX_DMA_CH);
}

void UART_DMA_RX_Init(void)
{
    /* 1. 关掉原来"每收一个字节进一次中断"的 RX 中断 */
    DL_UART_Main_disableInterrupt(PRINT_INST, DL_UART_MAIN_INTERRUPT_RX);

    /* 2. 配置 DMA 通道 */
    DL_DMA_initChannel(DMA, UART_RX_DMA_CH, (DL_DMA_Config *)&uart_rx_dma_cfg);

    /* 3. 让 UART 的 RX 事件作为 DMA 触发源 */
    DL_UART_Main_enableDMAReceiveEvent(PRINT_INST, DL_UART_DMA_INTERRUPT_RX);

    /* 4. 开"DMA 接收完成"中断（走 UART0 自己的中断线） */
    DL_UART_Main_enableInterrupt(PRINT_INST, DL_UART_MAIN_INTERRUPT_DMA_DONE_RX);
    NVIC_EnableIRQ(PRINT_INST_INT_IRQN);

    /* 5. 启动 */
    uart_rx_done = false;
    uart_dma_rx_start();
}

void UART_DMA_RX_Rearm(void)
{
    uart_rx_done = false;
    uart_dma_rx_start();
}

void UART_Send_char(UART_Regs *uart, const uint8_t chr)
{
    DL_UART_transmitDataBlocking(uart,chr);
}

void UART_Send_Buf(UART_Regs *uart, const uint8_t *buf, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        DL_UART_transmitDataBlocking(uart, buf[i]);
    }
}

void UART_Send_String(UART_Regs *uart, const char* str)
{
    while(*str)
    {
        UART_Send_char(uart,(uint8_t)*str);
        str++;
    }
}

void PRINT_INST_IRQHandler()
{
    switch (DL_UART_getPendingInterrupt(PRINT_INST))
    {
    case DL_UART_MAIN_IIDX_DMA_DONE_RX:
        {
            UART_DMA_RX_Rearm();     /* 先重新装填 */
            uart_rx_done = true;     /* 再置标志（Rearm 里会把 done 清 0，顺序别反） */
            // uint8_t rec = DL_UART_receiveData(PRINT_INST);
            // UART_Send_char(PRINT_INST,rec);
            break;
        }    
    
    default:
        break;
    }
}