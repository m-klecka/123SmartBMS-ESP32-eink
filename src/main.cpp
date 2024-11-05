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
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Icons/icons.h>

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
        display.setCursor(82, 93); // Adjust cursor position as needed
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold18pt7b);

        display.print("ZADNA DATA");
        display.display();

        return;
    }
    else if (result == 2)
    {
        Serial.println("The received BMS data is corrupted. Checksum mismatch.");

        // Clear the display
        display.fillScreen(GxEPD_WHITE); 
        display.setCursor(52, 93); // Adjust cursor position as needed
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold18pt7b);

        display.print("POSKOZENA DATA");
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
            const BatteryData battery = bms->getBatteryData();

            // Clear the display
            display.fillScreen(GxEPD_WHITE);
            display.setTextColor(GxEPD_BLACK);
            display.setFont(&FreeMonoBold9pt7b);

            // Display battery information
            display.drawBitmap(15, 15, icon_charge, 24, 24, GxEPD_BLACK);
            display.drawBitmap(15, 50, icon_up, 24, 24, GxEPD_BLACK);
            display.drawBitmap(15, 85, icon_hot, 24, 24, GxEPD_BLACK);
            display.drawBitmap(160, 15, icon_discharge, 24, 24, GxEPD_BLACK);
            display.drawBitmap(160, 50, icon_down, 24, 24, GxEPD_BLACK);
            display.drawBitmap(160, 85, icon_cold, 24, 24, GxEPD_BLACK);
            display.drawBitmap(320, 15, icon_battery, 45, 75, GxEPD_BLACK);

            
            // Text x+34
            // Text y+17

            display.setCursor(49, 33); // Adjust cursor position as needed
            display.println(String(battery.packChargeCurrent) + "A");

            display.setCursor(49, 67); // Adjust cursor position as needed
            display.println(String(battery.highestCellVoltage) + "V @ " + String(battery.highestCellVoltageNumber));

            display.setCursor(49, 102); // Adjust cursor position as needed
            display.println(String(battery.highestCellTemperature) + "C @ " + String(battery.highestCellTemperatureNumber));

            display.setCursor(194, 33); // Adjust cursor position as needed
            display.println(String(battery.packDischargeCurrent) + "A");

            display.setCursor(194, 67); // Adjust cursor position as needed
            display.print(String(battery.lowestCellVoltage) + "V @ " + String(battery.lowestCellVoltageNumber));

            display.setCursor(194, 102); // Adjust cursor position as needed
            display.print(String(battery.lowestCellTemperature) + "C @ " + String(battery.lowestCellTemperatureNumber));

            display.setFont(&FreeMonoBold12pt7b);
            display.setCursor(320, 115); // Adjust cursor position as needed
            display.println(String(battery.packSoc) + "%");

            display.setFont(&FreeMonoBold9pt7b);
            display.setCursor(310, 140); // Adjust cursor position as needed
            display.println(String(battery.packVoltage) + "V");

            display.setCursor(15, 135); // Adjust cursor position as needed
            display.println("Povoleno: " + String(battery.allowedToCharge ? "Nab:ano" : "Nab:ne") + " " + String(battery.allowedToDischarge ? "Vyb:ano" : "Vyb:ne") + "\n");

            display.setCursor(15, 155); // Adjust cursor position as needed
            display.print("Chyba komunikace: " + String(battery.communicationError ? "Ano" : "Ne") + "\n");
            
            display.setCursor(15, 155); // Adjust cursor position as needed
            // display.print("Alarm Min / Max Voltage: " + String(battery.alarmMinVoltage ? "Active" : "Inactive") + String(battery.alarmMaxVoltage ? "Active" : "Inactive") + "\n");
            // // display.print("Alarm Max Voltage: " + String(battery.alarmMaxVoltage ? "Active" : "Inactive") + "\n");
            // display.print("Alarm Min / Max Temp: " + String(battery.alarmMinTemperature ? "Active" : "Inactive") + String(battery.alarmMaxTemperature ? "Active" : "Inactive") + "\n");
            // // display.print("Alarm Max Temp: " + String(battery.alarmMaxTemperature ? "Active" : "Inactive") + "\n");

            // Display the content
            display.display();
        }
    }
}
