# Seat Heater Controller

This project implements a Seat Heater Controller using FreeRTOS to efficiently manage seat heating levels based on user input and current temperature readings. The controller monitors the temperature, adjusts the heating intensity accordingly, handles potential sensor failures, and provides real-time feedback via UART.

## Features

- **Multi-tasking**: Utilizes FreeRTOS to handle multiple tasks for reading temperature, controlling the heater, and displaying status updates.
- **Heating Levels**: Supports multiple heating levels (low, medium, high, no heating) based on user button presses.
- **Temperature Monitoring**: Continuously monitors the current temperature and adjusts heating states to ensure comfort and safety.
- **Failure Handling**: Detects out-of-range temperature readings and activates safety measures (e.g., LED indicators).
- **Real-time Status Display**: Sends current status and system measurements to a display via UART.

## Task Overview

The project consists of several key tasks, each responsible for specific functionality:

1. **GetDesiredTemperatureTask**: Reads the desired heating level based on user button presses.
2. **GetCurrentTemperatureTask**: Monitors the current temperature using an ADC and checks for valid temperature ranges.
3. **HeaterControlTask**: Adjusts the heater's state based on the current temperature and desired heating level.
4. **FailureHandlingTask**: Manages temperature out-of-range scenarios and activates failure indicators.
5. **DisplayScreenTask**: Sends the current temperature, heater state, and required heating level to the UART for display.
6. **RunTimeMeasurementsTask**: Collects and reports execution time for each task and overall CPU load.

## Getting Started

### Prerequisites

- FreeRTOS
- A suitable microcontroller with UART and ADC capabilities
- Development environment for C programming

### Installation

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd seat-heater-controller
