#pragma once

#include <chrono>
#include <map>
#include <string>
#include <mutex>

#include "variable_expression.h"

class MODEL_API TimingStats
{
public:
    static TimingStats& get_instance();

    void record_phase(const char* phase_name, double elapsed_ms);
    void print_report();
    void reset();
    double get_total_time();
    double get_phase_time(const char* phase_name);
    bool is_enabled() const;
    void set_enabled(bool enabled);

private:
    TimingStats() = default;
    TimingStats(const TimingStats&) = delete;
    TimingStats& operator=(const TimingStats&) = delete;

    static constexpr double PHASE_NAME_WIDTH = 24.0;
    static constexpr double TIME_WIDTH = 12.2;
    static constexpr double PERCENT_WIDTH = 8.2;

    std::map<std::string, double> m_phase_times;
    std::mutex m_mutex;
    bool m_enabled = false;
};

#define TIMING_SCOPE(phase_name) \
    TimingScopeTimer _timer_##__LINE__(phase_name)

class TimingScopeTimer
{
    const char* m_phase;
    std::chrono::high_resolution_clock::time_point m_start;

public:
    TimingScopeTimer(const char* phase) : m_phase(phase)
    {
        m_start = std::chrono::high_resolution_clock::now();
    }

    ~TimingScopeTimer()
    {
        if (TimingStats::get_instance().is_enabled())
        {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double, std::milli>(end - m_start).count();
            TimingStats::get_instance().record_phase(m_phase, duration);
        }
    }
};

namespace modeling_phases
{
    static const char* VARIABLE_CREATION = "variable_creation";
    static const char* LINEAR_CONSTRAINT = "linear_constraint";
    static const char* QUADRATIC_CONSTRAINT = "quadratic_constraint";
    static const char* NONLINEAR_CONSTRAINT = "nonlinear_constraint";
    static const char* OBJECTIVE_SETTING = "objective_setting";
}