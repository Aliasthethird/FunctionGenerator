#ifndef FUNCTIONGENERATOR_H
#define FUNCTIONGENERATOR_H

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"


/*------------------------------------------------------------------*/
/**
 * @class FunctionGenerator
 * @brief Generates simple waveforms (DC, Sine, Square, Saw, Triangle).
 *
 * Creates a FreeRTOS task that updates an internal signal value every `updatePeriod_ms`.
 * Thread-safe getter/setter functions allow changing amplitude, DC offset, period,
 * and waveform mode at runtime.
 *
 * Example:
 * ```cpp
 * FunctionGenerator fg(50);                 // updates every 50 ms
 * fg.setMode(FunctionGenerator::FgMode::Sine);
 * fg.setAmplitude(1.0f);
 * fg.setDcOffset(0.5f);
 * float value = fg.getValue();              // read current waveform sample
 * ```
 */
class FunctionGenerator
{
public:
    enum class FgMode : uint8_t
    {
        DC,
        Sine,
        Square,
        Saw,
        Triangle
    };

    explicit FunctionGenerator(uint16_t updatePeriod_ms);
    ~FunctionGenerator();

    float getValue() const;
    FgMode getMode() const;

    void setDcOffset(float dc);
    void setAmplitude(float a);
    void setPeriod(uint32_t T_ms);
    void setMode(FgMode m);
    // 0: DC, 1: Sine, 2; Square, 3: Saw, 4: Triangle
    void setModeFromIndex(int idx);

        TaskHandle_t m_generateTaskHandle = nullptr;
private:
    uint32_t m_T = 1000;
    float m_fgValue = 0.0f;
    float m_dc = 0.0f;
    float m_amp = 0.0f;
    FgMode m_mode = FgMode::DC;
    uint16_t m_updatePeriod_ms = 1;

    // TaskHandle_t m_generateTaskHandle = nullptr;
    mutable portMUX_TYPE m_lock = portMUX_INITIALIZER_UNLOCKED;

    static void generate(void *parameter);
};

#endif