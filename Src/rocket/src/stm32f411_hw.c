#include "stm32f411_hw.h"

#include "config.h"
#include "pinout.h"

#define REG32(address) (*(volatile uint32_t *)(address))

#define RCC_BASE        0x40023800UL
#define RCC_CR          REG32(RCC_BASE + 0x00)
#define RCC_PLLCFGR     REG32(RCC_BASE + 0x04)
#define RCC_CFGR        REG32(RCC_BASE + 0x08)
#define RCC_AHB1ENR     REG32(RCC_BASE + 0x30)
#define RCC_APB1ENR     REG32(RCC_BASE + 0x40)
#define RCC_APB2ENR     REG32(RCC_BASE + 0x44)

#define FLASH_ACR       REG32(0x40023C00UL)
#define SCB_CPACR       REG32(0xE000ED88UL)

#define GPIOA_BASE      0x40020000UL
#define GPIOB_BASE      0x40020400UL
#define GPIOC_BASE      0x40020800UL
#define GPIO_MODER(b)   REG32((b) + 0x00)
#define GPIO_OTYPER(b)  REG32((b) + 0x04)
#define GPIO_OSPEEDR(b) REG32((b) + 0x08)
#define GPIO_PUPDR(b)   REG32((b) + 0x0C)
#define GPIO_IDR(b)     REG32((b) + 0x10)
#define GPIO_BSRR(b)    REG32((b) + 0x18)
#define GPIO_AFRL(b)    REG32((b) + 0x20)
#define GPIO_AFRH(b)    REG32((b) + 0x24)

#define SPI1_BASE       0x40013000UL
#define SPI2_BASE       0x40003800UL
#define SPI3_BASE       0x40003C00UL
#define SPI_CR1(b)      REG32((b) + 0x00)
#define SPI_CR2(b)      REG32((b) + 0x04)
#define SPI_SR(b)       REG32((b) + 0x08)
#define SPI_DR8(b)      (*(volatile uint8_t *)((b) + 0x0C))

#define ADC1_BASE       0x40012000UL
#define ADC_SR          REG32(ADC1_BASE + 0x00)
#define ADC_CR1         REG32(ADC1_BASE + 0x04)
#define ADC_CR2         REG32(ADC1_BASE + 0x08)
#define ADC_SMPR2       REG32(ADC1_BASE + 0x10)
#define ADC_SQR3        REG32(ADC1_BASE + 0x34)
#define ADC_DR          REG32(ADC1_BASE + 0x4C)
#define ADC_CCR         REG32(0x40012304UL)

#define USART2_BASE     0x40004400UL
#define USART2_SR       REG32(USART2_BASE + 0x00)
#define USART2_DR       REG32(USART2_BASE + 0x04)
#define USART2_BRR      REG32(USART2_BASE + 0x08)
#define USART2_CR1      REG32(USART2_BASE + 0x0C)
#define USART2_CR2      REG32(USART2_BASE + 0x10)
#define USART2_CR3      REG32(USART2_BASE + 0x14)

#define NVIC_ISER1      REG32(0xE000E104UL)

#define HW_TIMEOUT_LOOPS 1000000UL

static uint32_t gpio_base(uint8_t port) {
    switch (port) {
        case GPIO_PORTA_A: return GPIOA_BASE;
        case GPIO_PORTA_B: return GPIOB_BASE;
        case GPIO_PORTA_C: return GPIOC_BASE;
        default: return 0;
    }
}

static uint32_t spi_base(hw_spi_t spi) {
    switch (spi) {
        case HW_SPI1: return SPI1_BASE;
        case HW_SPI2: return SPI2_BASE;
        case HW_SPI3: return SPI3_BASE;
        default: return 0;
    }
}

static void gpio_enable_clock(uint8_t port) {
    if (port <= GPIO_PORTA_C) {
        RCC_AHB1ENR |= (1UL << port);
        (void)RCC_AHB1ENR;
    }
}

void hw_delay_cycles(volatile uint32_t cycles) {
    while (cycles-- > 0U) {
        __asm volatile ("nop");
    }
}

void hw_clock_init(void) {
    RCC_CR |= 1UL;
    while ((RCC_CR & (1UL << 1)) == 0U) {
    }

    FLASH_ACR = (3UL << 0) | (1UL << 8) | (1UL << 9) | (1UL << 10);
    RCC_CFGR = (RCC_CFGR & ~((7UL << 10) | (7UL << 13))) | (4UL << 10);

    RCC_CR &= ~(1UL << 24);
    while ((RCC_CR & (1UL << 25)) != 0U) {
    }

    RCC_PLLCFGR = 16UL | (200UL << 6) | (0UL << 16) | (4UL << 24);
    RCC_CR |= (1UL << 24);
    while ((RCC_CR & (1UL << 25)) == 0U) {
    }

    RCC_CFGR = (RCC_CFGR & ~3UL) | 2UL;
    while (((RCC_CFGR >> 2) & 3UL) != 2UL) {
    }
}

void SystemInit(void) {
    SCB_CPACR |= (0xFUL << 20);
    __asm volatile ("dsb");
    __asm volatile ("isb");
    hw_clock_init();
}

void hw_gpio_output(uint8_t port, uint8_t pin, bool initial_high) {
    uint32_t base = gpio_base(port);
    if (base == 0U || pin > 15U) return;
    gpio_enable_clock(port);
    hw_gpio_write(port, pin, initial_high);
    GPIO_MODER(base) = (GPIO_MODER(base) & ~(3UL << (pin * 2U))) | (1UL << (pin * 2U));
    GPIO_OTYPER(base) &= ~(1UL << pin);
    GPIO_OSPEEDR(base) |= (3UL << (pin * 2U));
    GPIO_PUPDR(base) &= ~(3UL << (pin * 2U));
}

void hw_gpio_input_pullup(uint8_t port, uint8_t pin) {
    uint32_t base = gpio_base(port);
    if (base == 0U || pin > 15U) return;
    gpio_enable_clock(port);
    GPIO_MODER(base) &= ~(3UL << (pin * 2U));
    GPIO_PUPDR(base) = (GPIO_PUPDR(base) & ~(3UL << (pin * 2U))) | (1UL << (pin * 2U));
}

void hw_gpio_alternate(uint8_t port, uint8_t pin, uint8_t alternate) {
    uint32_t base = gpio_base(port);
    volatile uint32_t *afr;
    uint8_t shift;
    if (base == 0U || pin > 15U || alternate > 15U) return;
    gpio_enable_clock(port);
    GPIO_MODER(base) = (GPIO_MODER(base) & ~(3UL << (pin * 2U))) | (2UL << (pin * 2U));
    GPIO_OTYPER(base) &= ~(1UL << pin);
    GPIO_OSPEEDR(base) |= (3UL << (pin * 2U));
    GPIO_PUPDR(base) &= ~(3UL << (pin * 2U));
    afr = pin < 8U ? &GPIO_AFRL(base) : &GPIO_AFRH(base);
    shift = (uint8_t)((pin & 7U) * 4U);
    *afr = (*afr & ~(15UL << shift)) | ((uint32_t)alternate << shift);
}

void hw_gpio_analog(uint8_t port, uint8_t pin) {
    uint32_t base = gpio_base(port);
    if (base == 0U || pin > 15U) return;
    gpio_enable_clock(port);
    GPIO_MODER(base) |= (3UL << (pin * 2U));
    GPIO_PUPDR(base) &= ~(3UL << (pin * 2U));
}

void hw_gpio_write(uint8_t port, uint8_t pin, bool high) {
    uint32_t base = gpio_base(port);
    if (base == 0U || pin > 15U) return;
    GPIO_BSRR(base) = high ? (1UL << pin) : (1UL << (pin + 16U));
}

bool hw_gpio_read(uint8_t port, uint8_t pin) {
    uint32_t base = gpio_base(port);
    return base != 0U && pin <= 15U && (GPIO_IDR(base) & (1UL << pin)) != 0U;
}

status_t hw_spi_set_baud(hw_spi_t spi, uint8_t baud_div) {
    uint32_t base = spi_base(spi);
    if (base == 0U || baud_div > 7U) return STATUS_ERRO_GENERICO;
    SPI_CR1(base) &= ~(1UL << 6);
    SPI_CR1(base) = (SPI_CR1(base) & ~(7UL << 3)) | ((uint32_t)baud_div << 3);
    SPI_CR1(base) |= (1UL << 6);
    return STATUS_OK;
}

status_t hw_spi_init(hw_spi_t spi, uint8_t baud_div) {
    uint32_t base = spi_base(spi);
    if (base == 0U || baud_div > 7U) return STATUS_ERRO_GENERICO;

    if (spi == HW_SPI1) {
        RCC_APB2ENR |= (1UL << 12);
        hw_gpio_alternate(PINO_SPI1_SCK_PORTA, PINO_SPI1_SCK_PINO, 5);
        hw_gpio_alternate(PINO_SPI1_MISO_PORTA, PINO_SPI1_MISO_PINO, 5);
        hw_gpio_alternate(PINO_SPI1_MOSI_PORTA, PINO_SPI1_MOSI_PINO, 5);
    } else if (spi == HW_SPI2) {
        RCC_APB1ENR |= (1UL << 14);
        hw_gpio_alternate(PINO_SPI2_SCK_PORTA, PINO_SPI2_SCK_PINO, 5);
        hw_gpio_alternate(PINO_SPI2_MISO_PORTA, PINO_SPI2_MISO_PINO, 5);
        hw_gpio_alternate(PINO_SPI2_MOSI_PORTA, PINO_SPI2_MOSI_PINO, 5);
    } else {
        RCC_APB1ENR |= (1UL << 15);
        hw_gpio_alternate(PINO_SPI3_SCK_PORTA, PINO_SPI3_SCK_PINO, 6);
        hw_gpio_alternate(PINO_SPI3_MISO_PORTA, PINO_SPI3_MISO_PINO, 6);
        hw_gpio_alternate(PINO_SPI3_MOSI_PORTA, PINO_SPI3_MOSI_PINO, 6);
    }

    SPI_CR1(base) = (1UL << 2) | (1UL << 8) | (1UL << 9) | ((uint32_t)baud_div << 3);
    SPI_CR2(base) = 0;
    SPI_CR1(base) |= (1UL << 6);
    return STATUS_OK;
}

status_t hw_spi_transfer(hw_spi_t spi, const uint8_t *tx, uint8_t *rx, size_t length) {
    uint32_t base = spi_base(spi);
    if (base == 0U || length == 0U) return STATUS_ERRO_GENERICO;

    for (size_t i = 0; i < length; ++i) {
        uint32_t timeout = HW_TIMEOUT_LOOPS;
        while ((SPI_SR(base) & (1UL << 1)) == 0U && timeout-- > 0U) {
        }
        if (timeout == 0U) return STATUS_ERRO_TIMEOUT;

        SPI_DR8(base) = tx ? tx[i] : 0xFFU;
        timeout = HW_TIMEOUT_LOOPS;
        while ((SPI_SR(base) & (1UL << 0)) == 0U && timeout-- > 0U) {
        }
        if (timeout == 0U) return STATUS_ERRO_TIMEOUT;
        uint8_t value = SPI_DR8(base);
        if (rx) rx[i] = value;
    }

    uint32_t timeout = HW_TIMEOUT_LOOPS;
    while ((SPI_SR(base) & (1UL << 7)) != 0U && timeout-- > 0U) {
    }
    return timeout == 0U ? STATUS_ERRO_TIMEOUT : STATUS_OK;
}

status_t hw_adc1_init_channel(uint8_t channel) {
    if (channel > 18U) return STATUS_ERRO_GENERICO;
    RCC_APB2ENR |= (1UL << 8);
    ADC_CCR = (ADC_CCR & ~(3UL << 16)) | (1UL << 16);
    ADC_CR1 = 0;
    ADC_CR2 = 0;
    if (channel <= 9U) {
        ADC_SMPR2 = (ADC_SMPR2 & ~(7UL << (channel * 3U))) | (5UL << (channel * 3U));
    }
    ADC_SQR3 = channel;
    ADC_CR2 = (1UL << 0);
    hw_delay_cycles(CLOCK_SISTEMA_HZ / 100000U);
    return STATUS_OK;
}

status_t hw_adc1_read(uint16_t *value) {
    uint32_t timeout = HW_TIMEOUT_LOOPS;
    if (!value) return STATUS_ERRO_GENERICO;
    ADC_CR2 |= (1UL << 30);
    while ((ADC_SR & (1UL << 1)) == 0U && timeout-- > 0U) {
    }
    if (timeout == 0U) return STATUS_ERRO_TIMEOUT;
    *value = (uint16_t)(ADC_DR & 0x0FFFU);
    return STATUS_OK;
}

status_t hw_usart2_init(uint32_t baudrate) {
    if (baudrate == 0U) return STATUS_ERRO_GENERICO;
    RCC_APB1ENR |= (1UL << 17);
    hw_gpio_alternate(PINO_GPS_USART_TX_PORTA, PINO_GPS_USART_TX_PINO, 7);
    hw_gpio_alternate(PINO_GPS_USART_RX_PORTA, PINO_GPS_USART_RX_PINO, 7);
    USART2_CR1 = 0;
    USART2_CR2 = 0;
    USART2_CR3 = 0;
    USART2_BRR = (50000000UL + (baudrate / 2U)) / baudrate;
    USART2_CR1 = (1UL << 13) | (1UL << 3) | (1UL << 2) | (1UL << 5);
    NVIC_ISER1 = (1UL << (38U - 32U));
    return STATUS_OK;
}

bool hw_usart2_rx_ready(void) {
    return (USART2_SR & (1UL << 5)) != 0U;
}

uint8_t hw_usart2_read(void) {
    return (uint8_t)USART2_DR;
}
