// heat_pump_data.h
// heat_pump_data.h
#pragma once

// Structure to hold your heat pump data
struct HeatPumpData {
    float current_temperature;   // Current water or air temperature
    float target_temperature;    // Desired temperature
    float flow_rate;             // Water flow rate (L/min or m³/h)
    bool heater_on;              // Is the heater currently active
    bool pump_on;                // Is the circulation pump on
    bool fault;                  // Fault/error status
    unsigned long last_update;   // Timestamp of last data update (ms)
};

// Optional: Default initializer
inline HeatPumpData default_heat_pump_data() {
    HeatPumpData data;
    data.current_temperature = 0.0f;
    data.target_temperature = 25.0f; // default target
    data.flow_rate = 0.0f;
    data.heater_on = false;
    data.pump_on = false;
    data.fault = false;
    data.last_update = 0;
    return data;
}
