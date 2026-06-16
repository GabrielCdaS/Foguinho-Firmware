#include "stm32f411_hw.h"

#include "cubemx_platform.h"
#include "pinout.h"
#include "stm32f4xx_hal.h"

#define HW_TIMEOUT_MS 100U

static GPIO_TypeDef *gpio_port(uint8_t port)
{
    switch (port) {
        case GPIO_PORTA_A: return GPIOA;
        case GPIO_PORTA_B: return GPIOB;
        case GPIO_PORTA_C: return GPIOC;
        default: return NULL;
    }
}

static uint16_t gpio_pin(uint8_t pin)
{
    return pin <= 15U ? (uint16_t)(1UL << pin) : 0U;
}

static void gpio_clock_enable(uint8_t port)
{
    switch (port) {
        case GPIO_PORTA_A: __HAL_RCC_GPIOA_CLK_ENABLE(); break;
        case GPIO_PORTA_B: __HAL_RCC_GPIOB_CLK_ENABLE(); break;
        case GPIO_PORTA_C: __HAL_RCC_GPIOC_CLK_ENABLE(); break;
        default: break;
    }
}

static SPI_HandleTypeDef *spi_handle(hw_spi_t spi)
{
    switch (spi) {
        case HW_SPI1: return &hspi1;
        case HW_SPI2: return &hspi2;
        case HW_SPI3: return &hspi3;
        default: return NULL;
    }
}

static uint32_t spi_prescaler(uint8_t baud_div)
{
    static const uint32_t prescalers[] = {
        SPI_BAUDRATEPRESCALER_2,
        SPI_BAUDRATEPRESCALER_4,
        SPI_BAUDRATEPRESCALER_8,
        SPI_BAUDRATEPRESCALER_16,
        SPI_BAUDRATEPRESCALER_32,
        SPI_BAUDRATEPRESCALER_64,
        SPI_BAUDRATEPRESCALER_128,
        SPI_BAUDRATEPRESCALER_256
    };
    return prescalers[baud_div];
}

void hw_clock_init(void)
{
    cubemx_platform_init();
}

void hw_gpio_output(uint8_t port, uint8_t pin, bool initial_high)
{
    GPIO_TypeDef *instance = gpio_port(port);
    uint16_t mask = gpio_pin(pin);
    GPIO_InitTypeDef gpio = {
        .Pin = mask,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH
    };
    if (instance == NULL || mask == 0U) return;
    gpio_clock_enable(port);
    HAL_GPIO_WritePin(instance, mask, initial_high ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_Init(instance, &gpio);
}

void hw_gpio_input_pullup(uint8_t port, uint8_t pin)
{
    GPIO_TypeDef *instance = gpio_port(port);
    uint16_t mask = gpio_pin(pin);
    GPIO_InitTypeDef gpio = {
        .Pin = mask,
        .Mode = GPIO_MODE_INPUT,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_LOW
    };
    if (instance == NULL || mask == 0U) return;
    gpio_clock_enable(port);
    HAL_GPIO_Init(instance, &gpio);
}

void hw_gpio_analog(uint8_t port, uint8_t pin)
{
    GPIO_TypeDef *instance = gpio_port(port);
    uint16_t mask = gpio_pin(pin);
    GPIO_InitTypeDef gpio = {
        .Pin = mask,
        .Mode = GPIO_MODE_ANALOG,
        .Pull = GPIO_NOPULL
    };
    if (instance == NULL || mask == 0U) return;
    gpio_clock_enable(port);
    HAL_GPIO_Init(instance, &gpio);
}

void hw_gpio_write(uint8_t port, uint8_t pin, bool high)
{
    GPIO_TypeDef *instance = gpio_port(port);
    uint16_t mask = gpio_pin(pin);
    if (instance != NULL && mask != 0U) {
        HAL_GPIO_WritePin(instance, mask, high ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

bool hw_gpio_read(uint8_t port, uint8_t pin)
{
    GPIO_TypeDef *instance = gpio_port(port);
    uint16_t mask = gpio_pin(pin);
    return instance != NULL && mask != 0U &&
           HAL_GPIO_ReadPin(instance, mask) == GPIO_PIN_SET;
}

status_t hw_spi_set_baud(hw_spi_t spi, uint8_t baud_div)
{
    SPI_HandleTypeDef *handle = spi_handle(spi);
    if (handle == NULL || baud_div > 7U) return STATUS_ERRO_GENERICO;
    handle->Init.BaudRatePrescaler = spi_prescaler(baud_div);
    return HAL_SPI_Init(handle) == HAL_OK ? STATUS_OK : STATUS_ERRO_SPI;
}

status_t hw_spi_init(hw_spi_t spi, uint8_t baud_div)
{
    SPI_HandleTypeDef *handle = spi_handle(spi);
    if (handle == NULL || baud_div > 7U) return STATUS_ERRO_GENERICO;

    handle->Instance = spi == HW_SPI1 ? SPI1 : (spi == HW_SPI2 ? SPI2 : SPI3);
    handle->Init.Mode = SPI_MODE_MASTER;
    handle->Init.Direction = SPI_DIRECTION_2LINES;
    handle->Init.DataSize = SPI_DATASIZE_8BIT;
    handle->Init.CLKPolarity = SPI_POLARITY_LOW;
    handle->Init.CLKPhase = SPI_PHASE_1EDGE;
    handle->Init.NSS = SPI_NSS_SOFT;
    handle->Init.BaudRatePrescaler = spi_prescaler(baud_div);
    handle->Init.FirstBit = SPI_FIRSTBIT_MSB;
    handle->Init.TIMode = SPI_TIMODE_DISABLE;
    handle->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    handle->Init.CRCPolynomial = 7;
    return HAL_SPI_Init(handle) == HAL_OK ? STATUS_OK : STATUS_ERRO_SPI;
}

status_t hw_spi_transfer(hw_spi_t spi, const uint8_t *tx, uint8_t *rx, size_t length)
{
    SPI_HandleTypeDef *handle = spi_handle(spi);
    HAL_StatusTypeDef result;
    if (handle == NULL || length == 0U || length > UINT16_MAX) return STATUS_ERRO_GENERICO;

    if (tx != NULL && rx != NULL) {
        result = HAL_SPI_TransmitReceive(handle, (uint8_t *)tx, rx, (uint16_t)length, HW_TIMEOUT_MS);
    } else if (tx != NULL) {
        result = HAL_SPI_Transmit(handle, (uint8_t *)tx, (uint16_t)length, HW_TIMEOUT_MS);
    } else if (rx != NULL) {
        result = HAL_SPI_Receive(handle, rx, (uint16_t)length, HW_TIMEOUT_MS);
    } else {
        return STATUS_ERRO_GENERICO;
    }
    return result == HAL_OK ? STATUS_OK :
           (result == HAL_TIMEOUT ? STATUS_ERRO_TIMEOUT : STATUS_ERRO_SPI);
}

status_t hw_adc1_init_channel(uint8_t channel)
{
    ADC_ChannelConfTypeDef config = {0};
    if (channel > 18U) return STATUS_ERRO_GENERICO;
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) return STATUS_ERRO_GENERICO;

    config.Channel = channel;
    config.Rank = 1;
    config.SamplingTime = ADC_SAMPLETIME_144CYCLES;
    return HAL_ADC_ConfigChannel(&hadc1, &config) == HAL_OK ? STATUS_OK : STATUS_ERRO_GENERICO;
}

status_t hw_adc1_read(uint16_t *value)
{
    if (value == NULL) return STATUS_ERRO_GENERICO;
    if (HAL_ADC_Start(&hadc1) != HAL_OK) return STATUS_ERRO_GENERICO;
    if (HAL_ADC_PollForConversion(&hadc1, HW_TIMEOUT_MS) != HAL_OK) return STATUS_ERRO_TIMEOUT;
    *value = (uint16_t)HAL_ADC_GetValue(&hadc1);
    (void)HAL_ADC_Stop(&hadc1);
    return STATUS_OK;
}

status_t hw_usart1_init(uint32_t baudrate)
{
    if (baudrate == 0U) return STATUS_ERRO_GENERICO;

    huart1.Instance = USART1;
    huart1.Init.BaudRate = baudrate;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    return HAL_UART_Init(&huart1) == HAL_OK ? STATUS_OK : STATUS_ERRO_GENERICO;
}

bool hw_usart1_rx_ready(void)
{
    return __HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET;
}

status_t hw_usart1_write(const uint8_t *dados, size_t tamanho, uint32_t timeout_ms)
{
    HAL_StatusTypeDef resultado;
    if (dados == NULL || tamanho == 0U || tamanho > UINT16_MAX) {
        return STATUS_ERRO_GENERICO;
    }

    resultado = HAL_UART_Transmit(&huart1, (uint8_t *)dados, (uint16_t)tamanho, timeout_ms);
    return resultado == HAL_OK ? STATUS_OK :
           (resultado == HAL_TIMEOUT ? STATUS_ERRO_TIMEOUT : STATUS_ERRO_GENERICO);
}

status_t hw_usart1_read(uint8_t *byte, uint32_t timeout_ms)
{
    HAL_StatusTypeDef resultado;
    if (byte == NULL) return STATUS_ERRO_GENERICO;

    resultado = HAL_UART_Receive(&huart1, byte, 1U, timeout_ms);
    return resultado == HAL_OK ? STATUS_OK :
           (resultado == HAL_TIMEOUT ? STATUS_ERRO_TIMEOUT : STATUS_ERRO_GENERICO);
}

status_t hw_usart2_init(uint32_t baudrate)
{
    if (baudrate == 0U) return STATUS_ERRO_GENERICO;

    huart2.Instance = USART2;
    huart2.Init.BaudRate = baudrate;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    return HAL_UART_Init(&huart2) == HAL_OK ? STATUS_OK : STATUS_ERRO_GENERICO;
}

bool hw_usart2_rx_ready(void)
{
    return __HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) != RESET;
}

uint8_t hw_usart2_read(void)
{
    return (uint8_t)(huart2.Instance->DR & 0xFFU);
}
