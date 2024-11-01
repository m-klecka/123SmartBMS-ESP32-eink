/**
 * @file main.cpp
 * @author TheRealKasumi
 * @brief Entry point.
 * @copyright Copyright (c) 2023 TheRealKasumi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <HardwareSerial.h>
#include "bms/BMS.h"
#include <GxEPD2_BW.h>
#include <Fonts/FreeMono9pt7b.h>

// Some configuration
#define SERIAL_BAUD_RATE 115200
#define BMS_SERIAL_RX_PIN 15  // Connect RX to a suitable pin on ESP32 (e.g., GPIO 16)
#define BMS_SERIAL_BAUD_RATE 9600

// Pin definitions for the display
#define DISPLAY_POWER_PIN 2

HardwareSerial *bmsSerial = &Serial1; // Use hardware Serial1
BMS *bms = nullptr;

// Define the display
GxEPD2_BW<GxEPD2_290_GDEY029T71H, GxEPD2_290_GDEY029T71H::HEIGHT> display(GxEPD2_290_GDEY029T71H(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4));  // ESPink-Shelf-2.9 GDEY029T94  128x296, SSD1680, (FPC-A005 20.06.15)

// Update interval
const unsigned long updateInterval = 10000; // Update every 10 seconds
unsigned long lastUpdateTime = 0;

/**
 * @brief Setup.
 */
void setup()
{
    // Initialize the (PC) serial
    Serial.begin(SERIAL_BAUD_RATE);

    // Initialize the (BMS) serial
    bmsSerial->begin(BMS_SERIAL_BAUD_RATE, SERIAL_8N1, BMS_SERIAL_RX_PIN);

    // Create an {@link BMS} instance to decode its data
    bms = new BMS(bmsSerial);

    // Activate the display
    pinMode(DISPLAY_POWER_PIN, OUTPUT);
    digitalWrite(DISPLAY_POWER_PIN, HIGH); // Activate the display
    delay(100); // Wait for the display to initialize
    display.init(115200); // Initialize the display with the specified baud rate
    display.setRotation(1); // Rotate the display 90 degrees clockwise
}

/**
 * @brief Endless loop.
 */
void loop()
{
    // Read the BMS data
    const int result = bms->readBatteryData(2000);
    if (result == 1)
    {
        Serial.println("Warn: Failed to read BMS data. Timeout occurred because no data was received within the timeout.");

        // Clear the display
        display.fillScreen(GxEPD_WHITE); 
        display.setCursor(0, 10); // Adjust cursor position as needed
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMono9pt7b);

        display.print("BMS NO DATA");
        display.display();

        return;
    }
    else if (result == 2)
    {
        Serial.println("The received BMS data is corrupted. Checksum mismatch.");

        // Clear the display
        display.fillScreen(GxEPD_WHITE); 
        display.setCursor(0, 10); // Adjust cursor position as needed
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMono9pt7b);

        display.print("BMS DATA CORRUPTED");
        display.display();

        return;
    }
    
    // If we have valid data, store it and prepare to update display if needed
    if (result == 0)
    {
        // Get the battery data
        const BatteryData battery = bms->getBatteryData();
        
        // Print all battery data to Serial
        Serial.println((String)"Cell-Count: " + battery.cellCount);
        Serial.println((String)"Min-Cell-Voltage: " + battery.cellVoltageMin + "V");
        Serial.println((String)"Max-Cell-Voltage: " + battery.cellVoltageMax + "V");
        Serial.println((String)"Balance-Voltage: " + battery.cellVoltageBalance + "V");
        Serial.println((String)"Pack-SOC: " + battery.packSoc + "%");
        Serial.println((String)"Pack-Voltage: " + battery.packVoltage + "V");
        Serial.println((String)"Pack-Current: " + battery.packCurrent + "A");
        Serial.println((String)"Pack-Charge-Current: " + battery.packChargeCurrent + "A");
        Serial.println((String)"Pack-Discharge-Current: " + battery.packDischargeCurrent + "A");
        Serial.println((String)"Pack-Capacity: " + battery.packCapacity + "kWh");
        Serial.println((String)"Pack-Energy: " + battery.packRemainingEnergy + "kWh");
        Serial.println((String)"Lowest-Cell-Voltage: " + battery.lowestCellVoltage + "V");
        Serial.println((String)"Lowest-Cell-Voltage-Number: " + battery.lowestCellVoltageNumber);
        Serial.println((String)"Highest-Cell-Voltage: " + battery.highestCellVoltage + "V");
        Serial.println((String)"Highest-Cell-Voltage-Number: " + battery.highestCellVoltageNumber);
        Serial.println((String)"Lowest-Cell-Temp: " + battery.lowestCellTemperature + "°C");
        Serial.println((String)"Lowest-Cell-Temp-Number: " + battery.lowestCellTemperatureNumber);
        Serial.println((String)"Highest-Cell-Temp: " + battery.highestCellTemperature + "°C");
        Serial.println((String)"Highest-Cell-Temp-Number: " + battery.highestCellTemperatureNumber);
        Serial.println((String)"Allowed-Charge: " + String(battery.allowedToCharge ? "Yes" : "No"));
        Serial.println((String)"Allowed-Discharge: " + String(battery.allowedToDischarge ? "Yes" : "No"));
        Serial.println((String)"Alarm-Communication-Error: " + String(battery.communicationError ? "Active" : "Inactive"));
        Serial.println((String)"Alarm-Min-Voltage: " + String(battery.alarmMinVoltage ? "Active" : "Inactive"));
        Serial.println((String)"Alarm-Max-Voltage: " + String(battery.alarmMaxVoltage ? "Active" : "Inactive"));
        Serial.println((String)"Alarm-Min-Temp: " + String(battery.alarmMinTemperature ? "Active" : "Inactive"));
        Serial.println((String)"Alarm-Max-Temp: " + String(battery.alarmMaxTemperature ? "Active" : "Inactive"));

        // Update display if enough time has passed
        unsigned long currentMillis = millis();
        if (currentMillis - lastUpdateTime >= updateInterval)
        {
            lastUpdateTime = currentMillis;

            // Clear the display
            display.fillScreen(GxEPD_WHITE);
            display.setCursor(0, 10); // Adjust cursor position as needed
            display.setTextColor(GxEPD_BLACK);
            display.setFont(&FreeMono9pt7b);

            // Display battery information
            display.print("SoC: " + String(battery.packSoc) + "% @ " + String(battery.packVoltage) + "V\n");
            display.print("Charge Current: " + String(battery.packChargeCurrent) + "A\n");
            display.print("Discharge Current: " + String(battery.packDischargeCurrent) + "A\n");
            display.print("Energy Remaining: " + String(battery.packRemainingEnergy) + "kWh\n");
            display.print("L Cell Voltage: " + String(battery.lowestCellVoltage) + "V @ Cell: " + String(battery.lowestCellVoltageNumber) + "\n");
            display.print("H Cell Voltage: " + String(battery.highestCellVoltage) + "V @ Cell: " + String(battery.highestCellVoltageNumber) + "\n");
            display.print("L Cell Temp: " + String(battery.lowestCellTemperature) + "°C @ Cell: " + String(battery.lowestCellTemperatureNumber) + "\n");
            display.print("H Cell Temp: " + String(battery.highestCellTemperature) + "°C @ Cell: " + String(battery.highestCellTemperatureNumber) + "\n");
            display.print("Allowed Charge: " + String(battery.allowedToCharge ? "Yes" : "No") + "\n");
            display.print("Allowed Discharge: " + String(battery.allowedToDischarge ? "Yes" : "No") + "\n");
            display.print("Alarm Communication Error: " + String(battery.communicationError ? "Active" : "Inactive") + "\n");
            display.print("Alarm Min Voltage: " + String(battery.alarmMinVoltage ? "Active" : "Inactive") + "\n");
            display.print("Alarm Max Voltage: " + String(battery.alarmMaxVoltage ? "Active" : "Inactive") + "\n");
            display.print("Alarm Min Temp: " + String(battery.alarmMinTemperature ? "Active" : "Inactive") + "\n");
            display.print("Alarm Max Temp: " + String(battery.alarmMaxTemperature ? "Active" : "Inactive") + "\n");

            // Display the content
            display.display();
        }
    }
}
