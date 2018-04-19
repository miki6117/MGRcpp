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
	size_t asic_data_size = id * channel * 8;
    // cout << "DEBUG: Asic data size: " << asic_data_size << endl;
    unsigned char* asic_data = new unsigned char[asic_data_size];
    uint16_t amplitude = 0x123;
    unsigned long long int timestamp = 0;
    

    unsigned int asic_data_position = 0;
    for (uint8_t i = 1; i <= id; i++)
    {
        for (uint8_t j = 1; j <= channel; j++)
        {
            asic_data[asic_data_position] = static_cast<unsigned char>(i); // ID on first position
            asic_data[asic_data_position] += static_cast<unsigned char>(j << 4); // half of CHANNEL on first position
            asic_data[++asic_data_position] = static_cast<unsigned char>(j >> 4); // second half of CHANNEL on second position
            // amplitude = (amplitude >> 1) | ((((amplitude >> 4)^(amplitude >> 10)^(amplitude >> 12)) & 1) << 15); // {amplitude[15:0], amplitude[11] ^ amplitude[5] ^ amplitude[3]}; so: x^12 + x^6 + x^4
            amplitude = (amplitude << 1) | ((((amplitude >> 11)^(amplitude >> 5)^(amplitude >> 3)) & 1)); // {amplitude[15:0], amplitude[11] ^ amplitude[5] ^ amplitude[3]}; so: x^12 + x^6 + x^4
            asic_data[asic_data_position] += static_cast<unsigned char>(amplitude << 4); // 4 bit part of AMPLIT on second position
            asic_data[++asic_data_position] = static_cast<unsigned char>(amplitude >> 4); // 8 bit part of AMPLIT on third position
            asic_data[++asic_data_position] = static_cast<unsigned char>(amplitude >> 12); // 4 bit part of AMPLIT on fourth position

            asic_data[asic_data_position] += static_cast<unsigned char>(timestamp << 4);
            asic_data[++asic_data_position] = static_cast<unsigned char>(timestamp >> 4);
            asic_data[++asic_data_position] = static_cast<unsigned char>(timestamp >> 12);
            asic_data[++asic_data_position] = static_cast<unsigned char>(timestamp >> 20);
            asic_data[++asic_data_position] = static_cast<unsigned char>(timestamp >> 28);
            ++asic_data_position;
            cout << "ID: " << (int)i <<", CHANNEL: " << (int)j << ", AMPLITUDE: " << (int)amplitude << endl;
            // asic_data[asic_data_position] = i + (j << 4); // half of CHANNEL on first position
            // amplitude = (amplitude >> 1) | (((amplitude >> 4)^(amplitude >> 10)^(amplitude >> 12)) << 15); // {amplitude[15:0], amplitude[11] ^ amplitude[5] ^ amplitude[3]}; so: x^12 + x^6 + x^4
            // asic_data[++asic_data_position] = static_cast<unsigned char>(j >> 4) + static_cast<unsigned char>(amplitude << 4); // 4 bit part of AMPLIT on second position
            // asic_data[++asic_data_position] = static_cast<unsigned char>(amplitude >> 4); // 8 bit part of AMPLIT on third position
            // asic_data[++asic_data_position] = static_cast<unsigned char>(amplitude >> 12) + static_cast<unsigned char>(timestamp << 4);
            // asic_data[++asic_data_position] = static_cast<unsigned char>(timestamp >> 4);
            // asic_data[++asic_data_position] = static_cast<unsigned char>(timestamp >> 12);
            // asic_data[++asic_data_position] = static_cast<unsigned char>(timestamp >> 20);
            // asic_data[++asic_data_position] = static_cast<unsigned char>(timestamp >> 28);
            // ++asic_data_position;
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