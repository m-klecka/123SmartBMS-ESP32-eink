/**
 * @file main.cpp
 * @author TheRealKasumi
 * @brief Example application
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
#include <HardwareSerial.h>

#include "bms/SmartBmsData.h"
#include "bms/SmartBmsError.h"
#include "bms/SmartBmsReader.h"

#include <GxEPD2_BW.h>

#include <fonts/SourceSans3_Regular9pt7b.h>
#include <fonts/SourceSans3_Bold9pt7b.h>
#include <fonts/SourceSans3_Bold12pt7b.h>
#include <fonts/SourceSans3_Bold18pt7b.h>

#include <icons/icons.h>

// Serial configuration, adjust as needed
#define PC_SERIAL_BAUD 115200
#define BMS_SERIAL_MODE SERIAL_8N1
#define BMS_SERIAL_PERIPHERAL 1
#define BMS_SERIAL_BAUD_RATE 9600
#define BMS_SERIAL_RX_PIN 15
#define BMS_SERIAL_INVERT false

#define DISPLAY_UPDATE_TIME 10		// In seconds

// Serial connections
HardwareSerial smartBmsSerial(BMS_SERIAL_PERIPHERAL);
SmartBmsReader smartBmsReader(&smartBmsSerial);

// Define the display
GxEPD2_BW<GxEPD2_290_GDEY029T71H, GxEPD2_290_GDEY029T71H::HEIGHT> display(GxEPD2_290_GDEY029T71H(/*CS=5*/ SS, /*DC=*/17, /*RST=*/16, /*BUSY=*/4)); // ESPink-Shelf-2.9 GDEY029T94  128x296, SSD1680, (FPC-A005 20.06.15)

// Pin definitions for the display
#define DISPLAY_POWER_PIN 2

// Update interval
const unsigned long updateInterval = DISPLAY_UPDATE_TIME*1000;
unsigned long lastUpdateTime = 0;

// Remaining charge time
float remainingChargeTime = 0.0;

/**
 * @brief Setup.
 */
void setup()
{
	// Initialize the serial connections
	Serial.begin(PC_SERIAL_BAUD);																				// Begin pc serial monitor
	smartBmsSerial.begin(BMS_SERIAL_BAUD_RATE, BMS_SERIAL_MODE, BMS_SERIAL_RX_PIN, -1, BMS_SERIAL_INVERT);		// Begin BMS serial

	// Activate the display
	pinMode(DISPLAY_POWER_PIN, OUTPUT);																			// Set display pin mode
	digitalWrite(DISPLAY_POWER_PIN, HIGH); 																		// Activate the display
	delay(100);							   																		// Wait for the display to initialize
	display.init(115200);				  																		// Initialize the display with the specified baud rate
	display.setRotation(1);				 																		// Rotate the display 90 degrees clockwise
}

/**
 * @brief Endless loop.
 */
void loop()
{
	// Check if enough data was received
	if (smartBmsReader.bmsDataReady() == SmartBmsError::SBMS_OK)
	{
		// Read the BMS data and check for errors
		SmartBmsData smartBmsData;
		const SmartBmsError err = smartBmsReader.decodeBmsData(&smartBmsData);
		if (err == SmartBmsError::SBMS_OK)

		{
			// Data is ok, lets print it
			Serial.println();
			Serial.println("===========================");
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
			Serial.println("===========================");
			Serial.println();

			// Calculate remaining charge time if charging
			if (smartBmsData.getPackChargeCurrent() > 5 && smartBmsData.getPackSoc() < 100)
			{
				remainingChargeTime = (smartBmsData.getPackCapacity() - smartBmsData.getPackRemainingEnergy()) * 1000 /
									  (smartBmsData.getPackChargeCurrent() * smartBmsData.getPackVoltage());
			}
			else if (smartBmsData.getPackSoc() >= 100)
			{
				remainingChargeTime = 0; // Battery already at 100%
			}
			else
			{
				remainingChargeTime = -1; // Error or invalid value
			}

			// Update display if enough time has passed
			unsigned long currentMillis = millis();
			if (currentMillis - lastUpdateTime >= updateInterval)
			{
				lastUpdateTime = currentMillis;

				// Clear the display
				display.fillScreen(GxEPD_WHITE);
				display.setTextColor(GxEPD_BLACK);

				// Display battery information
				display.drawBitmap(15, 15, icon_charge, 24, 24, GxEPD_BLACK);
				display.drawBitmap(15, 50, icon_up, 24, 24, GxEPD_BLACK);
				display.drawBitmap(15, 85, icon_hot, 24, 24, GxEPD_BLACK);
				display.drawBitmap(160, 15, icon_discharge, 24, 24, GxEPD_BLACK);
				display.drawBitmap(160, 50, icon_down, 24, 24, GxEPD_BLACK);
				display.drawBitmap(160, 85, icon_cold, 24, 24, GxEPD_BLACK);

				// Battery icon dependenat on SoC and charging current
				if (smartBmsData.getPackSoc() < 10)
				{
					if (smartBmsData.getPackChargeCurrent() > 0)
					{
						display.drawBitmap(320, 15, icon_battery_0_charging, 45, 75, GxEPD_BLACK);
					}
					else
					{
						display.drawBitmap(320, 15, icon_battery_0, 45, 75, GxEPD_BLACK);
					}
				}
				else if (smartBmsData.getPackSoc() < 20)
				{
					if (smartBmsData.getPackChargeCurrent() > 0)
					{
						display.drawBitmap(320, 15, icon_battery_10_charging, 45, 75, GxEPD_BLACK);
					}
					else
					{
						display.drawBitmap(320, 15, icon_battery_10, 45, 75, GxEPD_BLACK);
					}
				}
				else if (smartBmsData.getPackSoc() < 30)
				{
					if (smartBmsData.getPackChargeCurrent() > 0)
					{
						display.drawBitmap(320, 15, icon_battery_20_charging, 45, 75, GxEPD_BLACK);
					}
					else
					{
						display.drawBitmap(320, 15, icon_battery_20, 45, 75, GxEPD_BLACK);
					}
				}
				else if (smartBmsData.getPackSoc() < 40)
				{
					if (smartBmsData.getPackChargeCurrent() > 0)
					{
						display.drawBitmap(320, 15, icon_battery_30_charging, 45, 75, GxEPD_BLACK);
					}
					else
					{
						display.drawBitmap(320, 15, icon_battery_30, 45, 75, GxEPD_BLACK);
					}
				}
				else if (smartBmsData.getPackSoc() < 50)
				{
					if (smartBmsData.getPackChargeCurrent() > 0)
					{
						display.drawBitmap(320, 15, icon_battery_40_charging, 45, 75, GxEPD_BLACK);
					}
					else
					{
						display.drawBitmap(320, 15, icon_battery_40, 45, 75, GxEPD_BLACK);
					}
				}
				else if (smartBmsData.getPackSoc() < 60)
				{
					if (smartBmsData.getPackChargeCurrent() > 0)
					{
						display.drawBitmap(320, 15, icon_battery_50_charging, 45, 75, GxEPD_BLACK);
					}
					else
					{
						display.drawBitmap(320, 15, icon_battery_50, 45, 75, GxEPD_BLACK);
					}
				}
				else if (smartBmsData.getPackSoc() < 70)
				{
					if (smartBmsData.getPackChargeCurrent() > 0)
					{
						display.drawBitmap(320, 15, icon_battery_60_charging, 45, 75, GxEPD_BLACK);
					}
					else
					{
						display.drawBitmap(320, 15, icon_battery_60, 45, 75, GxEPD_BLACK);
					}
				}
				else if (smartBmsData.getPackSoc() < 80)
				{
					if (smartBmsData.getPackChargeCurrent() > 0)
					{
						display.drawBitmap(320, 15, icon_battery_70_charging, 45, 75, GxEPD_BLACK);
					}
					else
					{
						display.drawBitmap(320, 15, icon_battery_70, 45, 75, GxEPD_BLACK);
					}
				}
				else if (smartBmsData.getPackSoc() < 90)
				{
					if (smartBmsData.getPackChargeCurrent() > 0)
					{
						display.drawBitmap(320, 15, icon_battery_80_charging, 45, 75, GxEPD_BLACK);
					}
					else
					{
						display.drawBitmap(320, 15, icon_battery_80, 45, 75, GxEPD_BLACK);
					}
				}
				else if (smartBmsData.getPackSoc() < 100)
				{
					if (smartBmsData.getPackChargeCurrent() > 5)
					{
						display.drawBitmap(320, 15, icon_battery_90_charging, 45, 75, GxEPD_BLACK);
					}
					else
					{
						display.drawBitmap(320, 15, icon_battery_90, 45, 75, GxEPD_BLACK);
					}
				}
				else
				{ // If SOC is already at 100%
					display.drawBitmap(320, 15, icon_battery_100, 45, 75, GxEPD_BLACK);
				}

				display.setFont(&SourceSans3_Bold9pt7b);

				display.setCursor(49, 33); // Adjust cursor position as needed
				display.println((String)smartBmsData.getPackChargeCurrent() + "A");

				display.setCursor(49, 67); // Adjust cursor position as needed
				display.println((String)smartBmsData.getHighestCellVoltage() + "V @ " + smartBmsData.getHighestCellVoltageNumber());

				display.setCursor(49, 102); // Adjust cursor position as needed
				display.println((String)smartBmsData.getHighestCellTemperature() + "C @ " + smartBmsData.getHighestCellTemperatureNumber());

				display.setCursor(194, 33); // Adjust cursor position as needed
				display.println((String)smartBmsData.getPackDischargeCurrent() + "A");

				display.setCursor(194, 67); // Adjust cursor position as needed
				display.println((String)smartBmsData.getLowestCellVoltage() + "V @ " + smartBmsData.getLowestCellVoltageNumber());

				display.setCursor(194, 102); // Adjust cursor position as needed
				display.println((String)smartBmsData.getLowestCellTemperature() + "C @ " + smartBmsData.getLowestCellTemperatureNumber());

				display.setCursor(317, 140); // Adjust cursor position as needed
				display.println((String)smartBmsData.getPackVoltage() + "V");

				display.setFont(&SourceSans3_Bold12pt7b);

				display.setCursor(320, 115); // Adjust cursor position as needed
				display.println((String)smartBmsData.getPackSoc() + "%");

				display.setFont(&SourceSans3_Bold9pt7b);

				display.setCursor(15, 135); // Adjust cursor position as needed

				// If communication error print it else show relevant data
				if (smartBmsData.hasCommunicationError())
				{
					display.println("CHYBA KOMUNIKACE");
				}
				else if (!smartBmsData.isAllowedToCharge() && (!smartBmsData.isAllowedToDischarge()))
				{
					display.println("Vybijeni ZAKAZANO - chyba");
					display.setCursor(15, 155); // Adjust cursor position as needed
					display.println("Nabijeni ZAKAZANO - chyba");
				}
				else if (!smartBmsData.isAllowedToCharge())
				{
					display.println("Nabijeni ZAKAZANO - chyba");
				}
				else if (!smartBmsData.isAllowedToDischarge())
				{
					display.println("Vybijeni ZAKAZANO - chyba");
				}
				else if (remainingChargeTime > 0)
				{
					int hours = (int)remainingChargeTime;
					int minutes = (int)((remainingChargeTime - hours) * 60);
					display.setCursor(15, 155); // Adjust cursor position as needed
					display.println((String) "Cas do nabiti: ~" + hours + "h " + minutes + "min");
				}

				// Display the content
				display.display();
			}
		}
		else if (err == SmartBmsError::SBMS_ERR_READ_STREAM)
		{
			// Failed to read the input stream
			Serial.println("Error: Failed to read BMS data. The input stream could not be read.");

			// Clear the display
			display.fillScreen(GxEPD_WHITE);
			display.setCursor(82, 93); // Adjust cursor position as needed
			display.setTextColor(GxEPD_BLACK);
			display.setFont(&SourceSans3_Bold18pt7b);

			display.print("ZADNA DATA");
			display.display();

			return;
		}
		else if (err == SmartBmsError::SBMS_ERR_INVALID_CHECKSUM)
		{
			// Checksum is invalid, something went very wrong
			Serial.println("Error: Failed to read BMS data. The checksum is invalid.");

			// Clear the display
			display.fillScreen(GxEPD_WHITE);
			display.setCursor(52, 93); // Adjust cursor position as needed
			display.setTextColor(GxEPD_BLACK);
			display.setFont(&SourceSans3_Bold18pt7b);

			display.print("POSKOZENA DATA");
			display.display();

			return;
		}
	}

	/*
	 * Do something else in the meantime, but make sure your serial buffer will not overflow.
	 */
}
