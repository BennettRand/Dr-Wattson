#include "dataAck.h"

#include "uart.h"

//------------ These values are used externally to access power data ------------
uint32_t sampleCount;
int64_t powerSum[2];
int64_t voltageSum[2];

#ifdef EXTENDED_DATA_PACKET
int64_t currentSum[2];
uint16_t linePeriod[2];
#endif
//-------------------------------------------------------------------------------

uint32_t newSampleCount;
int64_t newPowerSum[2];
int64_t newVoltageSum[2];

#ifdef EXTENDED_DATA_PACKET
int64_t newCurrentSum[2];
#endif

int16_t adcSampleData[4];

ISR(PCINT0_vect) { // Data ready triggered
	readData(adcSampleData,4);
	
	newSampleCount++;
	//newPowerSum[0] += (int64_t)(((int32_t)(adcSampleData[0] - deviceCalibration.channel1VoltageOffset) * (int32_t)(adcSampleData[2] - deviceCalibration.channel1CurrentOffset)));
	//newPowerSum[1] += (int64_t)(((int32_t)(adcSampleData[1] - deviceCalibration.channel2VoltageOffset) * (int32_t)(adcSampleData[3] - deviceCalibration.channel2CurrentOffset)));

	newVoltageSum[0] += (int64_t)(((int32_t)(adcSampleData[0] - deviceCalibration.channel1VoltageOffset) * (int32_t)(adcSampleData[0] - deviceCalibration.channel1VoltageOffset)));
	newVoltageSum[1] += (int64_t)(((int32_t)(adcSampleData[1] - deviceCalibration.channel2VoltageOffset) * (int32_t)(adcSampleData[1] - deviceCalibration.channel2VoltageOffset)));
	#ifdef EXTENDED_DATA_PACKET
	newCurrentSum[0] += (int64_t)(((int32_t)(adcSampleData[2] - deviceCalibration.channel1VoltageOffset) * (int32_t)(adcSampleData[2] - deviceCalibration.channel1VoltageOffset)));
	newCurrentSum[1] += (int64_t)(((int32_t)(adcSampleData[3] - deviceCalibration.channel2VoltageOffset) * (int32_t)(adcSampleData[3] - deviceCalibration.channel2VoltageOffset)));
	#endif
	
	PCIFR = (1<<PCIF0); // Need to clear the pin change interrupt flag as we leave to clear out the rising edge interrupt on drdy.
}

ISR(INT0_vect) {
	sampleCount = newSampleCount;
	voltageSum[0] = newVoltageSum[0];
	newSampleCount = 0;
	newVoltageSum[0] = 0;
	sei();
	printf("%lu, %8lx%8lx\n", sampleCount, (int32_t)(voltageSum[0]>>32), (int32_t)(voltageSum[0] & 0xFFFFFFFF));
/*	sampleCount += newSampleCount;
	powerSum[0] += newPowerSum[0];
	powerSum[1] += newPowerSum[1];
	newSampleCount = 0;
	newPowerSum[0] = 0;
	newPowerSum[1] = 0;

	voltageSum[0] += newVoltageSum[0];
	voltageSum[1] += newVoltageSum[1];
	newVoltageSum[0] = 0;
	newVoltageSum[1] = 0;
	#ifdef EXTENDED_DATA_PACKET
	currentSum[0] += newCurrentSum[0];
	currentSum[1] += newCurrentSum[1];
	newCurrentSum[0] = 0;
	newCurrentSum[1] = 0;
	#endif
*/	
}

void initDataAck() {
	// Configure the ADC
	initADC();
	sendCommand(CMD_STOP);
	sendCommand(CMD_SDATAC);
	uint8_t registers[] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x90, 0x90, 0x90};
	writeRegisters(0x05, registers, 8);

	// Configure data ready interrupt
	DDRB &= ~(1<<4);
	PCMSK0 = 1<<4;

	// Configure zero cross interrupt
	EICRA |= (1<<ISC01) | (1<<ISC00);
	DDRE |= 1<<2;
}

void stopDataAck() {
	EIMSK &= ~1;
	PCICR &= ~1;
	sendCommand(CMD_STOP);
	sendCommand(CMD_SDATAC);
}


void startDataAck() {
	sendCommand(CMD_START);
	sendCommand(CMD_RDATAC);
	EIMSK |= 1;
	PCICR |= 1;
	printf("Enabling Interrupts");
}

