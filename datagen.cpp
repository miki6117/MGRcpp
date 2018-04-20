#include "performance.h"

void DataGenerator::performActionOnGeneratedData(const unsigned char &data_char, unsigned int index)
{
	if (check_for_errors)
	{
		if (data[index] != data_char) errors += 1;
	}
	else
	{
		data[index] = data_char;
	}
}

void DataGenerator::asic()
{
	// size_t asic_data_size = id * channel * 8;
	// cout << "DEBUG: Asic data size: " << asic_data_size << endl;
	// unsigned char* asic_data = new unsigned char[asic_data_size];
	// uint16_t amplitude = 0x123;
	// unsigned long long int timestamp = 0;
	
	// uint8_t id; // 4b
	// uint8_t channel; // 8b
	uint16_t amplitude = 0x123; // 16b
	uint64_t timestamp = 0; // 36b

	uint8_t asic_data0; // ID and half of channel
	uint8_t asic_data1; // second half of CHANNEL and 1/4 of AMPLITUDE
	uint8_t asic_data2; // 1/2 of AMPLITUDE
	uint8_t asic_data3; // 1/4 of AMPLITUDE and 1/9 of TIMESTAMP

	uint8_t asic_data4; // 2/9 of TIMESTAMP
	uint8_t asic_data5; // 2/9 of TIMESTAMP
	uint8_t asic_data6; // 2/9 of TIMESTAMP
	uint8_t asic_data7; // 2/9 of TIMESTAMP

	uint8_t asic_data[8];

	const uint8_t max_id = 15;
	const uint8_t max_channel = 255;
	const uint16_t max_amplitude = 65535;
	const uint64_t max_timestamp = 68719476735;

	for (uint8_t id = 1; id <= max_id; id++)
	{
		for (uint8_t channel = 1; channel <= max_channel; channel++)
		{
			amplitude = (amplitude << 1) | ((((amplitude >> 11)^(amplitude >> 5)^(amplitude >> 3)) & 1)); // {amplitude[15:0], amplitude[11] ^ amplitude[5] ^ amplitude[3]}; so: x^12 + x^6 + x^4
			asic_data[0] = static_cast<unsigned char>(id);
			asic_data[0] += static_cast<unsigned char>(channel << 4); // ID and half of channel

			asic_data[1] = static_cast<unsigned char>(channel >> 4);
			asic_data[1] += static_cast<unsigned char>(amplitude << 4); // second half of CHANNEL and 1/4 of AMPLITUDE

			asic_data[2] = static_cast<unsigned char>(amplitude >> 4); // 1/2 of AMPLITUDE

			asic_data[3] = static_cast<unsigned char>(amplitude >> 12);
			asic_data[3] += static_cast<unsigned char>(timestamp << 4); // 1/4 of AMPLITUDE and 1/9 of TIMESTAMP

			asic_data[4] = static_cast<unsigned char>(timestamp >> 4);  // 2/9 of TIMESTAMP
			asic_data[5] = static_cast<unsigned char>(timestamp >> 12); // 2/9 of TIMESTAMP
			asic_data[6] = static_cast<unsigned char>(timestamp >> 20); // 2/9 of TIMESTAMP
			asic_data[7] = static_cast<unsigned char>(timestamp >> 28); // 2/9 of TIMESTAMP

		}
	}
}

void DataGenerator::walking1()
{
	uint64_t iter = 1;
	uint64_t last_possible_value = max_register_size / 2 + 1;
	for (unsigned int i=0; i < pattern_size; i+=register_size)
	{
		for (unsigned int j=0; j < register_size; j++)
		{
			unsigned char data_char = static_cast<unsigned char>((iter >> j*8) & 0xFF);
			performActionOnGeneratedData(data_char, i+j);
		}
		
		if (iter == last_possible_value) iter = 1;
		else iter *= 2;
	}
}

void DataGenerator::counter32Bit()
{
	uint64_t iter = 0;
	for (unsigned int i = 0; i < pattern_size; i+=register_size)
	{
		for (unsigned int j = 0; j < register_size; j++)
		{
			unsigned char data_char = static_cast<unsigned char>((iter >> j*8) & 0xFF);
			performActionOnGeneratedData(data_char, i+j);
		}
		if (iter > max_register_size) iter = 0;
		else ++iter;
	}
}

void DataGenerator::counter8Bit()
{
	uint64_t iter = 0;
	for (unsigned int i = 0; i < pattern_size; i++)
	{
		unsigned char data_char = static_cast<unsigned char>(iter);
		performActionOnGeneratedData(data_char, i);
		if (iter > max_register_size) iter = 0;
		else ++iter;
	}
}

void DataGenerator::determineRegisterParameters()
{
	if (mode == BIT32 || mode == DUPLEX)
	{
		register_size = 4;
		max_register_size = std::numeric_limits<unsigned int>::max();
	}
	else if (mode == NONSYM)
	{
		register_size = 8;
		max_register_size = std::numeric_limits<uint64_t>::max();
	}
	else
	{
		LOG(FATAL) << "Wrong width mode detected";
	}
	
	DLOG(INFO) << "Register size set to: " << register_size;
	DLOG(INFO) << "Max register size set to: " << max_register_size;
}

void DataGenerator::generateData()
{
	determineRegisterParameters();
	switch(pattern)
	{
		case COUNTER_8BIT:
			counter8Bit();
			break;

		case COUNTER_32BIT:
			counter32Bit();
			break;
		
		case WALKING_1:
			walking1();
			break;
	}
	DLOG(INFO) << "Data to write generated";
}

void DataGenerator::fillArrayWithData(unsigned char *data)
{
	check_for_errors = false;
	this->data = data;
	generateData();
}

unsigned int DataGenerator::checkArrayForErrors(unsigned char *data)
{
	check_for_errors = true;
	this->data = data;
	generateData();
	return errors;
}