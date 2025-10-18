#include "FunctionGenerator.h"

#include <math.h>
static constexpr float TWO_PI = 6.28318530717958647692f; // 2π

FunctionGenerator::FunctionGenerator(uint16_t updatePeriod_ms) : m_updatePeriod_ms(updatePeriod_ms)
{
    xTaskCreate(
        generate,             // Function that implements the task
        "generate",           // Name of the task (for debugging)
        // 768,                 // Stack size in words (not bytes)
        1024,                 // Stack size in words (not bytes)
        this,                 // Object passed to the task
        1,                    // Priority (higher = more important)
        &m_generateTaskHandle // Task handle
    );
}

FunctionGenerator::~FunctionGenerator()
{
    if (m_generateTaskHandle != nullptr)
    {
        vTaskDelete(m_generateTaskHandle);
        m_generateTaskHandle = nullptr;
    }
}

float FunctionGenerator::getValue() const
{
    portENTER_CRITICAL(&m_lock);
    float v = m_fgValue;
    portEXIT_CRITICAL(&m_lock);
    return v;
}

FunctionGenerator::FgMode FunctionGenerator::getMode() const
{
    portENTER_CRITICAL(&m_lock);
    FgMode w = m_mode;
    portEXIT_CRITICAL(&m_lock);
    return w;
}

void FunctionGenerator::setDcOffset(float dc)
{
    portENTER_CRITICAL(&m_lock);
    m_dc = dc;
    portEXIT_CRITICAL(&m_lock);
}

void FunctionGenerator::setAmplitude(float a)
{
    portENTER_CRITICAL(&m_lock);
    m_amp = a;
    portEXIT_CRITICAL(&m_lock);
}

void FunctionGenerator::setPeriod(uint32_t T_ms)
{
    if (T_ms == 0)
        T_ms = 1;
    portENTER_CRITICAL(&m_lock);
    m_T = T_ms;
    portEXIT_CRITICAL(&m_lock);
}

void FunctionGenerator::setMode(FgMode m)
{
    portENTER_CRITICAL(&m_lock);
    m_mode = m;
    portEXIT_CRITICAL(&m_lock);
}

void FunctionGenerator::setModeFromIndex(int idx)
{
    if (idx < 0)
        idx = 0;
    if (idx > 4)
        idx = 4;
    setMode(static_cast<FgMode>(idx));
}

void FunctionGenerator::generate(void *parameter)
{
    auto *self = static_cast<FunctionGenerator *>(parameter);
    TickType_t lastWake = xTaskGetTickCount();
    while (true)
    {
        // xTaskGetTickCount();
        // float phase = (millis() % self->m_T) / (float)self->m_T;
        float phase = (xTaskGetTickCount() * portTICK_PERIOD_MS % self->m_T) / (float)self->m_T;

        float out = 0.0f;
        switch (self->m_mode)
        {
            case FgMode::DC:       out = 1.0f;                                  break;
            case FgMode::Sine:     out = sinf(TWO_PI * phase);             break;
            case FgMode::Square:   out = (phase < 0.5f) ? 1.0f : -1.0f;         break;
            case FgMode::Saw:      out = 2.0f * phase - 1.0f;                   break;
            case FgMode::Triangle: out = 4.0f * std::fabs(phase - 0.5f) - 1.0f; break;
        }

        portENTER_CRITICAL(&self->m_lock);
        self->m_fgValue = self->m_amp * out + self->m_dc;
        portEXIT_CRITICAL(&self->m_lock);

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(self->m_updatePeriod_ms));
    }
}
