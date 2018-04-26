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
	uint16_t amplitude = 0x123; // 16b
	uint64_t timestamp = 0; // 36b

	const uint8_t max_id = 15; // 4b
	const uint8_t max_channel = 255; // 8b
	const uint16_t max_amplitude = 65535; // 16b
	const uint64_t max_timestamp = 68719476735; // 36b

	unsigned char asic_data[8];

	uint8_t id = 1;
	uint8_t channel = 1;
	unsigned int i_data = 0;

	while (i_data < pattern_size)
	{
		timestamp = i_data + 1; // TODO: Do with something better than that
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

		amplitude = (amplitude << 1) | ((((amplitude >> 11)^(amplitude >> 5)^(amplitude >> 3)) & 1)); // {amplitude[15:0], amplitude[11] ^ amplitude[5] ^ amplitude[3]}; so: x^12 + x^6 + x^4

		for (std::size_t i = 0; i < sizeof(asic_data); i++)
		{
			if (check_for_errors && i >= 3) break;
			performActionOnGeneratedData(asic_data[i], i_data + i);
		}
		i_data += 8;

		if (channel == max_channel)
		{
			channel = 1;
			++id;
		}
		else
		{
			++channel;
		}
		if (id == max_id)
		{
			id = 1;
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

		case ASIC:
			asic();
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