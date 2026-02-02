# -----------------------------------------------------------------------------
#  @file climate.py
#  Pool Heater Controller (Hayward PC1001) - ESPHome external component
#
#  Copyright (c) 2024 S. Leclerc
#  License: MIT
# -----------------------------------------------------------------------------

import logging

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core, pins
from esphome.components import (
    binary_sensor,
    button,
    climate,
    number,
    select,
    sensor,
    switch,
    text_sensor,
)
from esphome.const import (
    CONF_FILTERS,
    CONF_ID,
    CONF_INPUT,
    CONF_NAME,
    CONF_NUMBER,
    CONF_OUTPUT,
    CONF_SENSORS,
    CONF_UPDATE_INTERVAL,
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_TEMPERATURE,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_MINUTE,
)

_LOGGER = logging.getLogger(__name__)

CODEOWNERS = ["@sle118"]

AUTO_LOAD = [
    "climate",
    "select",
    "sensor",
    "binary_sensor",
    "button",
    "text_sensor",
    "time",
    "number",
    "watchdog",
]

DEPENDENCIES = ["climate", "esp32"]

# -----------------------------------------------------------------------------
# Component options
# -----------------------------------------------------------------------------
CONF_THROTTLE_AVERAGE = "throttle_average"

CONF_ACTIVE_MODE_SWITCH = "active_mode_switch"
CONF_UPDATE_SENSORS_SWITCH = "update_sensors_switch"
CONF_GENERATE_CODE_BUTTON = "generate_code"

CONF_GPIO_NETPIN = "pin_txrx"

# Temperatures / status
CONF_TEMPERATURE_SUCTION = "suction_temperature_T01"
CONF_TEMPERATURE_OUTLET = "outlet_temperature_T03"
CONF_TEMPERATURE_COIL = "coil_temperature_T04"
CONF_TEMPERATURE_AMBIENT = "ambient_temperature_T05"
CONF_TEMPERATURE_EXHAUST = "exhaust_temperature_T06"
CONF_ACTUAL_STATUS = "actual_status"
CONF_HEATER_STATUS_CODE = "heater_status_code"
CONF_HEATER_STATUS_DESCRIPTION = "heater_status_description"
CONF_HEATER_STATUS_SOLUTION = "heater_status_solution"

# D: Parameters of defrost
CONF_D01_DEFROST_START = "d01_defrost_start"
CONF_D02_DEFROST_END = "d02_defrost_end"
CONF_D03_DEFROSTING_CYCLE_TIME = "d03_defrosting_cycle_time_minutes"
CONF_D04_MAX_DEFROST_TIME = "d04_max_defrost_time_minutes"
CONF_D05_MIN_ECONOMY_DEFROST_TIME = "d05_min_economy_defrost_time_minutes"
CONF_D06_DEFROST_ECO_MODE = "d06_defrost_eco_mode"
CONF_D06_DEFROST_ECO_MODE_OPTIONS = ["Eco", "Normal"]

# R: Parameters of temperature
CONF_R04_RETURN_DIFF_COOLING = "r04_return_diff_cooling"
CONF_R05_SHUTDOWN_TEMP_DIFF_WHEN_COOLING = "r05_shutdown_temp_diff_when_cooling"
CONF_R06_RETURN_DIFF_HEATING = "r06_return_diff_heating"
CONF_R07_SHUTDOWN_DIFF_HEATING = "r07_shutdown_diff_heating"

# read-only parameters (still exposed as sensors)
CONF_R08_MIN_COOL_SETPOINT = "r08_min_cool_setpoint"
CONF_R09_MAX_COOLING_SETPOINT = "r09_max_cooling_setpoint"
CONF_R10_MIN_HEATING_SETPOINT = "r10_min_heating_setpoint"
CONF_R11_MAX_HEATING_SETPOINT = "r11_max_heating_setpoint"

# U: Parameters of water flow
CONF_U01_FLOW_METER = "u01_flow_meter"
CONF_U01_FLOW_METER_OPTIONS = ["Enabled", "Disabled"]
CONF_U02_PULSES_PER_LITER = "u02_pulses_per_liter"

# H: restrictions
CONF_H02_MODE_RESTRICTIONS = "h02_mode_restrictions"
CONF_H02_MODE_RESTRICTIONS_OPTIONS = ["Cooling Only", "Heating Only", "Any Mode"]

# State sensors
CONF_S02_WATER_FLOW = "s02_water_flow"

# Setpoints (exposed as sensors)
CONF_R01_SETPOINT_COOLING = "r01_setpoint_cooling"
CONF_R02_SETPOINT_HEATING = "r02_setpoint_heating"
CONF_R03_SETPOINT_AUTO = "r03_setpoint_auto"

# -----------------------------------------------------------------------------
# Codegen classes
# -----------------------------------------------------------------------------
hwp_ns = cg.esphome_ns.namespace("hwp")

PoolHeater = hwp_ns.class_("PoolHeater", cg.Component, climate.Climate)
ActiveModeSwitch = hwp_ns.class_("ActiveModeSwitch", switch.Switch, cg.Component)
UpdateStatusSwitch = hwp_ns.class_("UpdateStatusSwitch", switch.Switch, cg.Component)
GenerateCodeButton = hwp_ns.class_("GenerateCodeButton", button.Button, cg.Component, cg.Parented)

# -----------------------------------------------------------------------------
# Helpers
# -----------------------------------------------------------------------------
def create_throttle_avg_filter(_sensor_name: str) -> dict:
    return {CONF_FILTERS: [{CONF_THROTTLE_AVERAGE: "60s"}]}


# -----------------------------------------------------------------------------
# Schema (ESPHome 2025.11+ / 2026.x compatible)
# -----------------------------------------------------------------------------
BASE_SCHEMA = climate.climate_schema(PoolHeater).extend(
    {
        # Hardware pin used by the component
        cv.Required(CONF_GPIO_NETPIN): pins.gpio_pin_schema(
            {CONF_OUTPUT: True, CONF_INPUT: True}
        ),
        # Allow explicit name; keep your default
        cv.Optional(CONF_NAME, default="Pool Heater"): cv.Any(
            cv.All(
                cv.none,
                cv.requires_friendly_name(
                    "Name cannot be None when esphome->friendly_name is not set!"
                ),
            ),
            cv.string,
        ),
        cv.Optional(CONF_ACTIVE_MODE_SWITCH, default={"name": "Active Mode"}): switch.switch_schema(
            ActiveModeSwitch,
            entity_category=ENTITY_CATEGORY_CONFIG,
            default_restore_mode="RESTORE_DEFAULT_OFF",
            icon="mdi:upload-network",
        ),
        cv.Optional(
            CONF_UPDATE_SENSORS_SWITCH, default={"name": "Update Sensors"}
        ): switch.switch_schema(
            UpdateStatusSwitch,
            entity_category=ENTITY_CATEGORY_CONFIG,
            default_restore_mode="RESTORE_DEFAULT_OFF",
            icon="mdi:upload-network",
        ),
        cv.Optional(
            CONF_GENERATE_CODE_BUTTON, default={"name": "Generate Code"}
        ): button.button_schema(
            GenerateCodeButton,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            icon="mdi:code-tags",
        ),
        cv.Optional(CONF_UPDATE_INTERVAL, default="30s"): cv.All(
            cv.positive_time_period_milliseconds,
            cv.Range(
                min=core.TimePeriod(seconds=10),
                max=core.TimePeriod(seconds=1800),
            ),
        ),
    }
)

# -----------------------------------------------------------------------------
# Input (configurable) entities
# -----------------------------------------------------------------------------
INPUT_TYPES_TEMPLATE: dict[str, dict] = {
    CONF_NUMBER: {
        "schema": number.number_schema,
        "registration_function": number.register_number,
        "class_": number.Number,
        "suffix": CONF_NUMBER,
    },
    "select": {
        "schema": select.select_schema,
        "registration_function": select.register_select,
        "class_": select.Select,
        "suffix": "select",
    },
}

# Each input: (friendly name, schema key, schema options, register options)
INPUTS: dict[str, tuple] = {
    CONF_D01_DEFROST_START: (
        "Defost start temperature",
        CONF_NUMBER,
        {
            "icon": "mdi:thermometer-chevron-up",
            "unit_of_measurement": UNIT_CELSIUS,
            "device_class": DEVICE_CLASS_TEMPERATURE,
        },
        {"min_value": -30, "max_value": 0, "step": 0.5},
    ),
    CONF_D02_DEFROST_END: (
        "Defrost end temperature",
        CONF_NUMBER,
        {
            "icon": "mdi:thermometer-chevron-down",
            "unit_of_measurement": UNIT_CELSIUS,
            "device_class": DEVICE_CLASS_TEMPERATURE,
        },
        {"min_value": 0, "max_value": 30, "step": 0.5},
    ),
    CONF_D03_DEFROSTING_CYCLE_TIME: (
        "Defrosting cycle time",
        CONF_NUMBER,
        {
            "icon": "mdi:timer-marker",
            "unit_of_measurement": UNIT_MINUTE,
            "device_class": DEVICE_CLASS_DURATION,
        },
        {"min_value": 0, "max_value": 90, "step": 1},
    ),
    CONF_D04_MAX_DEFROST_TIME: (
        "Max Defrost Time",
        CONF_NUMBER,
        {
            "icon": "mdi:timer-marker",
            "unit_of_measurement": UNIT_MINUTE,
            "device_class": DEVICE_CLASS_DURATION,
        },
        {"min_value": 0, "max_value": 20, "step": 1},
    ),
    CONF_D05_MIN_ECONOMY_DEFROST_TIME: (
        "Min Economy Defrost Time",
        CONF_NUMBER,
        {
            "icon": "mdi:timer-marker",
            "unit_of_measurement": UNIT_MINUTE,
            "device_class": DEVICE_CLASS_DURATION,
        },
        {"min_value": 0, "max_value": 20, "step": 1},
    ),
    CONF_D06_DEFROST_ECO_MODE: (
        "Defrost Eco Mode",
        "select",
        {"icon": "mdi:sprout"},
        {"options": CONF_D06_DEFROST_ECO_MODE_OPTIONS},
    ),
    CONF_R04_RETURN_DIFF_COOLING: (
        "Return Diff Cooling",
        CONF_NUMBER,
        {
            "icon": "mdi:thermometer-chevron-up",
            "unit_of_measurement": UNIT_CELSIUS,
            "device_class": DEVICE_CLASS_TEMPERATURE,
        },
        {"min_value": 0, "max_value": 10, "step": 0.5},
    ),
    CONF_R05_SHUTDOWN_TEMP_DIFF_WHEN_COOLING: (
        "Shutdown Temp Diff When Cooling",
        CONF_NUMBER,
        {
            "icon": "mdi:thermometer-chevron-down",
            "unit_of_measurement": UNIT_CELSIUS,
            "device_class": DEVICE_CLASS_TEMPERATURE,
        },
        {"min_value": 0, "max_value": 10, "step": 0.5},
    ),
    CONF_R06_RETURN_DIFF_HEATING: (
        "Return Diff Heating",
        CONF_NUMBER,
        {
            "icon": "mdi:thermometer",
            "unit_of_measurement": UNIT_CELSIUS,
            "device_class": DEVICE_CLASS_TEMPERATURE,
        },
        {"min_value": 0, "max_value": 10, "step": 0.5},
    ),
    CONF_R07_SHUTDOWN_DIFF_HEATING: (
        "Shutdown Temp Diff When Heating",
        CONF_NUMBER,
        {
            "icon": "mdi:thermometer-off",
            "unit_of_measurement": UNIT_CELSIUS,
            "device_class": DEVICE_CLASS_TEMPERATURE,
        },
        {"min_value": 0, "max_value": 10, "step": 0.5},
    ),
    CONF_U02_PULSES_PER_LITER: (
        "Pulses Per Liter",
        CONF_NUMBER,
        {"icon": "mdi:speedometer"},
        {"min_value": 0, "max_value": 300, "step": 1},
    ),
    CONF_U01_FLOW_METER: (
        "Flow Meter",
        "select",
        {"icon": "mdi:water-sync"},
        {"options": CONF_U01_FLOW_METER_OPTIONS},
    ),
    CONF_H02_MODE_RESTRICTIONS: (
        "Mode Restrictions",
        "select",
        {"icon": "mdi:clipboard-check-multiple"},
        {"options": CONF_H02_MODE_RESTRICTIONS_OPTIONS},
    ),
}

# -----------------------------------------------------------------------------
# Sensor entities
# -----------------------------------------------------------------------------
SENSORS: dict[str, tuple] = {
    CONF_S02_WATER_FLOW: (
        "Water Flow",
        binary_sensor.binary_sensor_schema(
            icon="mdi:water",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        binary_sensor.register_binary_sensor,
        None,
    ),
    CONF_R01_SETPOINT_COOLING: (
        "Cooling Setpoint",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            device_class=DEVICE_CLASS_TEMPERATURE,
            accuracy_decimals=1,
            icon="mdi:thermostat",
        ),
        sensor.register_sensor,
        create_throttle_avg_filter,
    ),
    CONF_R02_SETPOINT_HEATING: (
        "Heating Setpoint",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            device_class=DEVICE_CLASS_TEMPERATURE,
            accuracy_decimals=1,
            icon="mdi:thermostat",
        ),
        sensor.register_sensor,
        create_throttle_avg_filter,
    ),
    CONF_R03_SETPOINT_AUTO: (
        "Auto Setpoint",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            device_class=DEVICE_CLASS_TEMPERATURE,
            accuracy_decimals=1,
            icon="mdi:thermostat-auto",
        ),
        sensor.register_sensor,
        create_throttle_avg_filter,
    ),
    CONF_TEMPERATURE_OUTLET: (
        "Outlet Temperature",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
            accuracy_decimals=1,
            icon="mdi:sun-thermometer-outline",
        ),
        sensor.register_sensor,
        create_throttle_avg_filter,
    ),
    CONF_R08_MIN_COOL_SETPOINT: (
        "Min Cool Setpoint",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            device_class=DEVICE_CLASS_TEMPERATURE,
            accuracy_decimals=1,
            icon="mdi:thermostat",
        ),
        sensor.register_sensor,
        create_throttle_avg_filter,
    ),
    CONF_R09_MAX_COOLING_SETPOINT: (
        "Max Cooling Setpoint",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            device_class=DEVICE_CLASS_TEMPERATURE,
            accuracy_decimals=1,
            icon="mdi:thermostat",
        ),
        sensor.register_sensor,
        create_throttle_avg_filter,
    ),
    CONF_R10_MIN_HEATING_SETPOINT: (
        "Min Heating Setpoint",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            device_class=DEVICE_CLASS_TEMPERATURE,
            accuracy_decimals=1,
            icon="mdi:thermostat",
        ),
        sensor.register_sensor,
        create_throttle_avg_filter,
    ),
    CONF_R11_MAX_HEATING_SETPOINT: (
        "Max Heating Setpoint",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            device_class=DEVICE_CLASS_TEMPERATURE,
            accuracy_decimals=1,
            icon="mdi:thermostat",
        ),
        sensor.register_sensor,
        create_throttle_avg_filter,
    ),
    CONF_TEMPERATURE_SUCTION: (
        "Suction Temperature",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
            accuracy_decimals=1,
            icon="mdi:sun-thermometer-outline",
        ),
        sensor.register_sensor,
        create_throttle_avg_filter,
    ),
    CONF_TEMPERATURE_COIL: (
        "Coil Temperature",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
            accuracy_decimals=1,
            icon="mdi:air-conditioner",
        ),
        sensor.register_sensor,
        create_throttle_avg_filter,
    ),
    CONF_TEMPERATURE_AMBIENT: (
        "Ambient Temperature",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
            accuracy_decimals=1,
            icon="mdi:thermometer",
        ),
        sensor.register_sensor,
        create_throttle_avg_filter,
    ),
    CONF_TEMPERATURE_EXHAUST: (
        "Exhaust Temperature",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
            accuracy_decimals=1,
            icon="mdi:smoke-detector",
        ),
        sensor.register_sensor,
        create_throttle_avg_filter,
    ),
    CONF_ACTUAL_STATUS: (
        "Actual Status",
        text_sensor.text_sensor_schema(
            icon="mdi:heat",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        text_sensor.register_text_sensor,
        None,
    ),
    CONF_HEATER_STATUS_CODE: (
        "Heater Status",
        text_sensor.text_sensor_schema(
            icon="mdi:alert-circle-outline",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        text_sensor.register_text_sensor,
        None,
    ),
    CONF_HEATER_STATUS_DESCRIPTION: (
        "Status Description",
        text_sensor.text_sensor_schema(
            icon="mdi:information-outline",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        text_sensor.register_text_sensor,
        None,
    ),
    CONF_HEATER_STATUS_SOLUTION: (
        "Status Solution",
        text_sensor.text_sensor_schema(
            icon="mdi:toolbox-outline",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        text_sensor.register_text_sensor,
        None,
    ),
}

# -----------------------------------------------------------------------------
# Build sensor schema dict
# -----------------------------------------------------------------------------
SENSORS_SCHEMA = cv.All(
    {
        cv.Optional(
            sensor_designator,
            default={
                "name": f"{sensor_name}",
                "disabled_by_default": "false",
                **(filter_creation(sensor_designator) if filter_creation else {}),
            },
        ): sensor_schema
        for sensor_designator, (
            sensor_name,
            sensor_schema,
            _register_fn,
            filter_creation,
        ) in SENSORS.items()
    }
)

# -----------------------------------------------------------------------------
# Dynamically declare codegen classes for inputs and build input schema dict
# -----------------------------------------------------------------------------
for sensor_designator, (sensor_name, schema_name, schema_options, register_options) in list(
    INPUTS.items()
):
    schema_registry = INPUT_TYPES_TEMPLATE[schema_name]
    schema_options["class_"] = hwp_ns.class_(
        f"{sensor_designator}_{schema_registry['suffix']}",
        schema_registry["class_"],
    )
    schema_options["entity_category"] = ENTITY_CATEGORY_CONFIG
    INPUTS[sensor_designator] = (sensor_name, schema_name, schema_options, register_options)

INPUTS_SCHEMA = cv.All(
    {
        cv.Optional(sensor_designator, default={"name": f"{sensor_name}"}): INPUT_TYPES_TEMPLATE[
            schema_name
        ]["schema"](**schema_options)
        for sensor_designator, (
            sensor_name,
            schema_name,
            schema_options,
            _register_options,
        ) in INPUTS.items()
    }
)

# -----------------------------------------------------------------------------
# Final schema
# -----------------------------------------------------------------------------
CONFIG_SCHEMA = BASE_SCHEMA.extend(
    {
        cv.Optional(CONF_SENSORS, default={}): SENSORS_SCHEMA,
        cv.Optional(CONF_INPUT, default={}): INPUTS_SCHEMA,
    }
)

# -----------------------------------------------------------------------------
# Code generation
# -----------------------------------------------------------------------------
async def to_code(config):
    pin_component = await cg.gpio_pin_expression(config[CONF_GPIO_NETPIN])

    # PoolHeater is the Climate + Component instance (don’t create a second climate!)
    heater_component = cg.new_Pvariable(config[CONF_ID], pin_component)
    await cg.register_component(heater_component, config)
    await climate.register_climate(heater_component, config)

    # Sensors
    for sensor_designator, (_name, _schema, registration_function, _filter_fn) in SENSORS.items():
        sensor_conf = config[CONF_SENSORS][sensor_designator]
        sensor_component = cg.new_Pvariable(sensor_conf[CONF_ID])
        await registration_function(sensor_component, sensor_conf)
        cg.add(getattr(heater_component, f"set_{sensor_designator}_sensor")(sensor_component))

    # Inputs (numbers/selects)
    for sensor_designator, (_name, schema_name, _schema_options, register_options) in INPUTS.items():
        if sensor_designator not in config[CONF_INPUT]:
            continue

        input_conf = config[CONF_INPUT][sensor_designator]
        input_component = cg.new_Pvariable(input_conf[CONF_ID])

        registration_function = INPUT_TYPES_TEMPLATE[schema_name]["registration_function"]
        await registration_function(input_component, input_conf, **register_options)
        await cg.register_parented(input_component, heater_component)

        cg.add(getattr(heater_component, f"set_{sensor_designator}_sensor")(input_component))

    # Debug switches/buttons (parented)
    if am_switch_conf := config.get(CONF_ACTIVE_MODE_SWITCH):
        switch_component = await switch.new_switch(am_switch_conf)
        await cg.register_component(switch_component, am_switch_conf)
        await cg.register_parented(switch_component, heater_component)

    if us_switch_conf := config.get(CONF_UPDATE_SENSORS_SWITCH):
        us_switch_component = await switch.new_switch(us_switch_conf)
        await cg.register_component(us_switch_component, us_switch_conf)
        await cg.register_parented(us_switch_component, heater_component)

    if generate_code_conf := config.get(CONF_GENERATE_CODE_BUTTON):
        button_component = await button.new_button(generate_code_conf)
        await cg.register_component(button_component, generate_code_conf)
        await cg.register_parented(button_component, heater_component)
