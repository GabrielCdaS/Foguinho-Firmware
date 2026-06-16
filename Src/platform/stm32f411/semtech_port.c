#include "semtech_port.h"

#include <stddef.h>

#include "delay.h"
#include "gpio.h"
#include "hal_plataforma.h"
#include "pinout.h"
#include "spi.h"
#include "stm32f411_hw.h"
#include "stm32f4xx_hal.h"
#include "sx1276-board.h"
#include "sx1276/sx1276.h"
#include "timer.h"

#define SEMTECH_TIMER_CAPACITY 8U

static TimerEvent_t *timers[SEMTECH_TIMER_CAPACITY];
static bool radio_active;
static uint32_t dio0_previous;

static GPIO_TypeDef *pin_port(PinNames pin)
{
    uint32_t index = (uint32_t)pin;
    switch (index / 16U) {
        case 0: return GPIOA;
        case 1: return GPIOB;
        case 2: return GPIOC;
        default: return NULL;
    }
}

static uint16_t pin_mask(PinNames pin)
{
    return (uint16_t)(1UL << ((uint32_t)pin % 16U));
}

static void pin_clock_enable(PinNames pin)
{
    switch ((uint32_t)pin / 16U) {
        case 0: __HAL_RCC_GPIOA_CLK_ENABLE(); break;
        case 1: __HAL_RCC_GPIOB_CLK_ENABLE(); break;
        case 2: __HAL_RCC_GPIOC_CLK_ENABLE(); break;
        default: break;
    }
}

void GpioInit(Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config,
              PinTypes type, uint32_t value)
{
    GPIO_TypeDef *port;
    GPIO_InitTypeDef init = {0};
    if (obj == NULL) return;

    obj->pin = pin;
    obj->IrqHandler = NULL;
    obj->Context = NULL;
    if (pin == NC) {
        obj->port = NULL;
        obj->pinIndex = 0U;
        return;
    }

    port = pin_port(pin);
    obj->port = port;
    obj->pinIndex = pin_mask(pin);
    if (port == NULL) return;
    pin_clock_enable(pin);

    init.Pin = obj->pinIndex;
    init.Pull = type == PIN_PULL_UP ? GPIO_PULLUP :
                (type == PIN_PULL_DOWN ? GPIO_PULLDOWN : GPIO_NOPULL);
    init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    switch (mode) {
        case PIN_OUTPUT:
            init.Mode = config == PIN_OPEN_DRAIN ? GPIO_MODE_OUTPUT_OD : GPIO_MODE_OUTPUT_PP;
            HAL_GPIO_WritePin(port, obj->pinIndex, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
            break;
        case PIN_ANALOGIC:
            init.Mode = GPIO_MODE_ANALOG;
            break;
        case PIN_ALTERNATE_FCT:
            init.Mode = GPIO_MODE_AF_PP;
            break;
        case PIN_INPUT:
        default:
            init.Mode = GPIO_MODE_INPUT;
            break;
    }
    HAL_GPIO_Init(port, &init);
}

void GpioSetContext(Gpio_t *obj, void *context)
{
    if (obj != NULL) obj->Context = context;
}

void GpioSetInterrupt(Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority,
                      GpioIrqHandler *irqHandler)
{
    (void)irqMode;
    (void)irqPriority;
    if (obj != NULL) obj->IrqHandler = irqHandler;
}

void GpioRemoveInterrupt(Gpio_t *obj)
{
    if (obj != NULL) obj->IrqHandler = NULL;
}

void GpioWrite(Gpio_t *obj, uint32_t value)
{
    if (obj != NULL && obj->port != NULL) {
        HAL_GPIO_WritePin((GPIO_TypeDef *)obj->port, obj->pinIndex,
                          value ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

void GpioToggle(Gpio_t *obj)
{
    if (obj != NULL && obj->port != NULL) {
        HAL_GPIO_TogglePin((GPIO_TypeDef *)obj->port, obj->pinIndex);
    }
}

uint32_t GpioRead(Gpio_t *obj)
{
    if (obj == NULL || obj->port == NULL) return 0U;
    return HAL_GPIO_ReadPin((GPIO_TypeDef *)obj->port, obj->pinIndex) == GPIO_PIN_SET;
}

void SpiInit(Spi_t *obj, SpiId_t spiId, PinNames mosi, PinNames miso,
             PinNames sclk, PinNames nss)
{
    (void)mosi;
    (void)miso;
    (void)sclk;
    if (obj == NULL) return;
    obj->SpiId = spiId;
    GpioInit(&obj->Nss, nss, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1U);
    (void)hw_spi_init(spiId == SPI_2 ? HW_SPI2 : HW_SPI1, 2U);
}

void SpiDeInit(Spi_t *obj)
{
    (void)obj;
}

void SpiFormat(Spi_t *obj, int8_t bits, int8_t cpol, int8_t cpha, int8_t slave)
{
    (void)obj;
    (void)bits;
    (void)cpol;
    (void)cpha;
    (void)slave;
}

void SpiFrequency(Spi_t *obj, uint32_t hz)
{
    uint8_t divider = hz >= 25000000U ? 1U :
                      (hz >= 12500000U ? 2U : 3U);
    if (obj != NULL) {
        (void)hw_spi_set_baud(obj->SpiId == SPI_2 ? HW_SPI2 : HW_SPI1, divider);
    }
}

uint16_t SpiInOut(Spi_t *obj, uint16_t outData)
{
    uint8_t tx = (uint8_t)outData;
    uint8_t rx = 0U;
    if (obj == NULL ||
        hw_spi_transfer(obj->SpiId == SPI_2 ? HW_SPI2 : HW_SPI1, &tx, &rx, 1U) != STATUS_OK) {
        return 0U;
    }
    return rx;
}

void Delay(float seconds)
{
    DelayMs((uint32_t)(seconds * 1000.0f));
}

void DelayMs(uint32_t ms)
{
    plataforma_delay_ms(ms);
}

void TimerInit(TimerEvent_t *obj, void (*callback)(void *context))
{
    if (obj == NULL) return;
    obj->Timestamp = 0U;
    obj->ReloadValue = 0U;
    obj->IsStarted = false;
    obj->IsNext2Expire = false;
    obj->Callback = callback;
    obj->Context = NULL;
    obj->Next = NULL;
}

void TimerSetContext(TimerEvent_t *obj, void *context)
{
    if (obj != NULL) obj->Context = context;
}

void TimerStart(TimerEvent_t *obj)
{
    if (obj == NULL || obj->IsStarted) return;
    obj->Timestamp = plataforma_obter_tick_ms() + obj->ReloadValue;
    obj->IsStarted = true;
    for (size_t i = 0; i < SEMTECH_TIMER_CAPACITY; ++i) {
        if (timers[i] == NULL || timers[i] == obj) {
            timers[i] = obj;
            return;
        }
    }
    obj->IsStarted = false;
}

bool TimerIsStarted(TimerEvent_t *obj)
{
    return obj != NULL && obj->IsStarted;
}

void TimerStop(TimerEvent_t *obj)
{
    if (obj == NULL) return;
    obj->IsStarted = false;
    for (size_t i = 0; i < SEMTECH_TIMER_CAPACITY; ++i) {
        if (timers[i] == obj) timers[i] = NULL;
    }
}

void TimerReset(TimerEvent_t *obj)
{
    TimerStop(obj);
    TimerStart(obj);
}

void TimerSetValue(TimerEvent_t *obj, uint32_t value)
{
    if (obj == NULL) return;
    TimerStop(obj);
    obj->ReloadValue = value == 0U ? 1U : value;
}

TimerTime_t TimerGetCurrentTime(void)
{
    return plataforma_obter_tick_ms();
}

TimerTime_t TimerGetElapsedTime(TimerTime_t past)
{
    return plataforma_obter_tick_ms() - past;
}

TimerTime_t TimerTempCompensation(TimerTime_t period, float temperature)
{
    (void)temperature;
    return period;
}

void TimerIrqHandler(void)
{
    TimerProcess();
}

void TimerProcess(void)
{
    uint32_t now = plataforma_obter_tick_ms();
    for (size_t i = 0; i < SEMTECH_TIMER_CAPACITY; ++i) {
        TimerEvent_t *timer = timers[i];
        if (timer != NULL && timer->IsStarted &&
            (int32_t)(now - timer->Timestamp) >= 0) {
            timers[i] = NULL;
            timer->IsStarted = false;
            if (timer->Callback != NULL) timer->Callback(timer->Context);
        }
    }
}

void SX1276IoInit(void)
{
    GpioInit(&SX1276.Spi.Nss, PB_12, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1U);
    GpioInit(&SX1276.DIO0, PB_2, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0U);
    GpioInit(&SX1276.DIO1, NC, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0U);
    GpioInit(&SX1276.DIO2, NC, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0U);
    GpioInit(&SX1276.DIO3, NC, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0U);
    GpioInit(&SX1276.DIO4, NC, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0U);
    GpioInit(&SX1276.DIO5, NC, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0U);
    dio0_previous = GpioRead(&SX1276.DIO0);
}

void SX1276IoIrqInit(DioIrqHandler **irqHandlers)
{
    GpioSetInterrupt(&SX1276.DIO0, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, irqHandlers[0]);
}

void SX1276IoDeInit(void)
{
    GpioRemoveInterrupt(&SX1276.DIO0);
}

void SX1276IoTcxoInit(void) {}
void SX1276IoDbgInit(void) {}
void SX1276SetBoardTcxo(uint8_t state) { (void)state; }
uint32_t SX1276GetBoardTcxoWakeupTime(void) { return 0U; }
void SX1276AntSwInit(void) {}
void SX1276AntSwDeInit(void) {}
void SX1276SetAntSw(uint8_t opMode) { (void)opMode; }

void SX1276Reset(void)
{
    GpioInit(&SX1276.Reset, PB_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0U);
    DelayMs(2U);
    GpioWrite(&SX1276.Reset, 1U);
    DelayMs(10U);
}

void SX1276SetRfTxPower(int8_t power)
{
    uint8_t pa_config = SX1276Read(REG_PACONFIG);
    uint8_t pa_dac = SX1276Read(REG_PADAC);

    /* Os módulos SX1278 usados pelo projeto expõem a saída PA_BOOST. */
    pa_config = (pa_config & RF_PACONFIG_PASELECT_MASK) | RF_PACONFIG_PASELECT_PABOOST;
    if (power > 17) {
        pa_dac = (pa_dac & RF_PADAC_20DBM_MASK) | RF_PADAC_20DBM_ON;
        if (power < 5) power = 5;
        if (power > 20) power = 20;
        pa_config = (pa_config & RF_PACONFIG_OUTPUTPOWER_MASK) | (uint8_t)(power - 5);
    } else {
        pa_dac = (pa_dac & RF_PADAC_20DBM_MASK) | RF_PADAC_20DBM_OFF;
        if (power < 2) power = 2;
        pa_config = (pa_config & RF_PACONFIG_OUTPUTPOWER_MASK) | (uint8_t)(power - 2);
    }
    SX1276Write(REG_PACONFIG, pa_config);
    SX1276Write(REG_PADAC, pa_dac);
}

void SX1276SetAntSwLowPower(bool status)
{
    radio_active = !status;
}

bool SX1276CheckRfFrequency(uint32_t frequency)
{
    return frequency >= 137000000UL && frequency <= 525000000UL;
}

uint32_t SX1276GetDio1PinState(void) { return 0U; }
void SX1276DbgPinTxWrite(uint8_t state) { (void)state; }
void SX1276DbgPinRxWrite(uint8_t state) { (void)state; }

void semtech_port_process(void)
{
    uint32_t dio0 = GpioRead(&SX1276.DIO0);
    TimerProcess();
    if (radio_active && dio0 != 0U && dio0_previous == 0U &&
        SX1276.DIO0.IrqHandler != NULL) {
        SX1276.DIO0.IrqHandler(SX1276.DIO0.Context);
    }
    dio0_previous = dio0;
}
