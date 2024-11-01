/**
 * @file main.cpp
 * @author TheRealKasumi
 * @brief Entry point.
 * @copyright Copyright (c) 2024 TheRealKasumi
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <Arduino.h>
#include <HardwareSerial.h>

#include <GxEPD2_BW.h>
#include <Fonts/FreeMono9pt7b.h>

#include "bms/SmartBmsData.h"
#include "bms/SmartBmsError.h"
#include "bms/SmartBmsReader.h"

#define SERIAL_BAUD_RATE 115200
#define BMS_SERIAL_RX_PIN 15
#define BMS_SERIAL_BAUD_RATE 9600
#define BMS_SERIAL_INVERT false

HardwareSerial *smartBmsSerial = &Serial1; // Use hardware Serial1
SmartBmsReader *smartBmsReader = nullptr;

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
	smartBmsSerial->begin(BMS_SERIAL_BAUD_RATE, SERIAL_8N1, BMS_SERIAL_RX_PIN);

	// Create an {@link BMS} instance to decode its data
	smartBmsReader = new SmartBmsReader(smartBmsSerial);
}

/**
 * @brief Endless loop.
 */
void loop()
{
	// Read the BMS data and check for errors
	SmartBmsData smartBmsData;

	// Check if enough data was received
	if (smartBmsReader->bmsDataReady() == SmartBmsError::DATA_OK)
	{

		const SmartBmsError err = smartBmsReader->decodeBmsData(&smartBmsData);
		if (err == SmartBmsError::DATA_OK)
		{
			// Data is ok, lets print it
			Serial.println((String) "Cell-Count: " + smartBmsData.getCellCount());
			Serial.println((String) "Min-Cell-Voltage: " + smartBmsData.getCellVoltageMin() + "V");
			Serial.println((String) "Max-Cell-Voltage: " + smartBmsData.getCellVoltageMax() + "V");
			Serial.println((String) "Balance-Voltage: " + smartBmsData.getCellVoltageBalance() + "V");
			Serial.println((String) "Pack-SOC: " + smartBmsData.getPackSoc() + "%");
			Serial.println((String) "Pack-Voltage: " + smartBmsData.getPackVoltage() + "V");
			Serial.println((String) "Pack-Current: " + smartBmsData.getPackCurrent() + "A");
			Serial.println((String) "Pack-Charge-Current: " + smartBmsData.getPackChargeCurrent() + "A");
			Serial.println((String) "Pack-Discharge-Current: " + smartBmsData.getPackDischargeCurrent() + "A");
			Serial.println((String) "Pack-Capacity: " + smartBmsData.getPackCapacity() + "kWh");
			Serial.println((String) "Pack-Energy: " + smartBmsData.getPackRemainingEnergy() + "kWh");
			Serial.println((String) "Lowest-Cell-Voltage: " + smartBmsData.getLowestCellVoltage() + "V");
			Serial.println((String) "Lowest-Cell-Voltage-Numer: " + smartBmsData.getLowestCellVoltageNumber());
			Serial.println((String) "Highest-Cell-Voltage: " + smartBmsData.getHighestCellVoltage() + "V");
			Serial.println((String) "Highest-Cell-Voltage-Number: " + smartBmsData.getHighestCellVoltageNumber());
			Serial.println((String) "Lowest-Cell-Temp: " + smartBmsData.getLowestCellTemperature() + "°C");
			Serial.println((String) "Lowest-Cell-Temp-Number: " + smartBmsData.getLowestCellTemperatureNumber());
			Serial.println((String) "Highest-Cell-Temp: " + smartBmsData.getHighestCellTemperature() + "°C");
			Serial.println((String) "Highest-Cell-Temp-Number: " + smartBmsData.getHighestCellTemperatureNumber());
			Serial.println((String) "Allowed-Charge: " + (smartBmsData.isAllowedToCharge() ? "Yes" : "No"));
			Serial.println((String) "Allowed-Discharge: " + (smartBmsData.isAllowedToDischarge() ? "Yes" : "No"));
			Serial.println((String) "Alarm-Communication-Error: " + (smartBmsData.hasCommunicationError() ? "Active" : "Inactive"));
			Serial.println((String) "Alarm-Min-Voltage: " + (smartBmsData.isMinVoltageAlarmActive() ? "Active" : "Inactive"));
			Serial.println((String) "Alarm-Max-Voltage: " + (smartBmsData.isMaxVoltageAlarmActive() ? "Active" : "Inactive"));
			Serial.println((String) "Alarm-Min-Temp: " + (smartBmsData.isMinTemperatureAlarmActive() ? "Active" : "Inactive"));
			Serial.println((String) "Alarm-Max-Temp: " + (smartBmsData.isMaxTemperatureAlarmActive() ? "Active" : "Inactive"));
		}
		else if (err == SmartBmsError::ERR_READ_STREAM)
		{
			// Failed to read the input stream
			Serial.println("Error: Failed to read BMS data. The input stream could not be read.");
			
			display.fillScreen(GxEPD_WHITE); 
			display.setCursor(0, 10); // Adjust cursor position as needed
			display.setTextColor(GxEPD_BLACK);
			display.setFont(&FreeMono9pt7b);

			display.print("BMS NO DATA");
			display.display();

			return;
		}
		else if (err == SmartBmsError::ERR_INVALID_CHECKSUM)
		{
			// Checksum is invalid, something went very wrong
			Serial.println("Error: Failed to read BMS data. The checksum is invalid.");
			
			display.fillScreen(GxEPD_WHITE); 
			display.setCursor(0, 10); // Adjust cursor position as needed
			display.setTextColor(GxEPD_BLACK);
			display.setFont(&FreeMono9pt7b);

			display.print("BMS DATA CORRUPTED");
			display.display();

			return;
		}
	}

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
		display.println((String) "Pack-SOC: " + smartBmsData.getPackSoc() + "% @ " + smartBmsData.getPackVoltage() + "V");
		display.println((String) "Pack-Charge-Current: " + smartBmsData.getPackChargeCurrent() + "A");
		display.println((String) "Pack-Discharge-Current: " + smartBmsData.getPackDischargeCurrent() + "A");
		display.println((String) "Pack-Energy: " + smartBmsData.getPackRemainingEnergy() + "kWh");
		display.println((String) "Lowest-Cell-Voltage: " + smartBmsData.getLowestCellVoltage() + "V @ Cell: " + smartBmsData.getLowestCellVoltageNumber());
		display.println((String) "Highest-Cell-Voltage: " + smartBmsData.getHighestCellVoltage() + "V @ Cell: " + smartBmsData.getHighestCellVoltageNumber());
		display.println((String) "Lowest-Cell-Temperature: " + smartBmsData.getLowestCellTemperature() + "V @ Cell: " + smartBmsData.getLowestCellTemperatureNumber());
		display.println((String) "Highest-Cell-Temperature: " + smartBmsData.getHighestCellTemperature() + "V @ Cell: " + smartBmsData.getHighestCellTemperatureNumber());
		display.println((String) "Allowed-Charge: " + (smartBmsData.isAllowedToCharge() ? "Yes" : "No"));
		display.println((String) "Allowed-Discharge: " + (smartBmsData.isAllowedToDischarge() ? "Yes" : "No"));
		display.println((String) "Alarm-Communication-Error: " + (smartBmsData.hasCommunicationError() ? "Active" : "Inactive"));
		display.println((String) "Alarm-Min-Voltage: " + (smartBmsData.isMinVoltageAlarmActive() ? "Active" : "Inactive"));
		display.println((String) "Alarm-Max-Voltage: " + (smartBmsData.isMaxVoltageAlarmActive() ? "Active" : "Inactive"));
		display.println((String) "Alarm-Min-Temp: " + (smartBmsData.isMinTemperatureAlarmActive() ? "Active" : "Inactive"));
		display.println((String) "Alarm-Max-Temp: " + (smartBmsData.isMaxTemperatureAlarmActive() ? "Active" : "Inactive"));

		// Display the content
		display.display();
	}
}
