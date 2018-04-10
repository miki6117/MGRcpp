#include "performance.h"

// INTERFACE
void ITimer::performActionOnGeneratedData(const unsigned char &data_char, unsigned char *data, int index)
{
	if (check_for_errors)
	{
		if (data[index] != data_char) r->errors += 1;
	}
	else
	{
		data[index] = data_char;
	}
}

void ITimer::determineRegisterParameters(unsigned int mode, unsigned int &register_size, uint64_t &max_register_size)
{
	switch(mode)
	{
		case BIT32:
			register_size = 4;
			max_register_size = std::numeric_limits<int>::max();
			break;

		case NONSYM:
			register_size = 8;
			max_register_size = std::numeric_limits<uint64_t>::max();
			break;

		case DUPLEX:
			register_size = 4;
			max_register_size = std::numeric_limits<int>::max();
			break;

		default:
			LOG(FATAL) << "Wrong width mode detected";
			break;
	}
	DLOG(INFO) << "Register size set to: " << register_size;
}

void ITimer::generateData(unsigned char *data)
{
	unsigned int register_size;
	uint64_t max_register_size;
	auto pattern = cfgs.pattern_m[r->pattern];

	determineRegisterParameters(cfgs.mode_m[r->mode], register_size, max_register_size);

	uint64_t iter = 0;
	unsigned char data_char;
	if (pattern == COUNTER_8BIT)
	{
		for (auto i = 0; i < r->pattern_size; i++)
		{
			data_char = static_cast<unsigned char>(iter);
			performActionOnGeneratedData(data_char, data, i);
			if (iter > max_register_size) iter = 0;
			else ++iter;
		}
	}
	else if (pattern == COUNTER_32BIT)
	{
		for (auto i = 0; i < r->pattern_size; i+=register_size)
		{
			for (auto j = 0; j < register_size; j++)
			{
				data_char = static_cast<unsigned char>((iter >> j*8) & 0xFF);
				performActionOnGeneratedData(data_char, data, i+j);
			}
			if (iter > max_register_size) iter = 0;
			else ++iter;
		}
	}
	else if (pattern == WALKING_1)
	{
		iter = 1;
		uint64_t last_possible_value = max_register_size / 2 + 1;
		for (int i=0; i < r->pattern_size; i+=register_size)
		{
			for (auto j=0; j < register_size; j++)
			{
				data_char = static_cast<unsigned char>((iter >> j*8) & 0xFF);
				performActionOnGeneratedData(data_char, data, i+j);
			}
			
			if (iter == last_possible_value) iter = 1;
			else iter *= 2;
		}
	}

	DLOG(INFO) << "Data to write generated";
}

void ITimer::prepareForTransfer(unsigned char *data)
{
	r->pc_duration_total = std::chrono::nanoseconds::zero();
	r->errors = 0;
	dev->SetWireInValue(PATTERN_TO_GENERATE, cfgs.pattern_m[r->pattern]);
	dev->UpdateWireIns();
	dev->ActivateTriggerIn(TRIGGER, RESET);
	void timer(unsigned char *data);
}

// READ
void Read::performTimer(unsigned char *data)
{
	prepareForTransfer(data);
	for (unsigned int i=0; i<cfgs.iterations; i++)
	{
		DLOG(INFO) << "Current iteration: " << i;
		dev->ActivateTriggerIn(TRIGGER, RESET_PATTERN);
		timer_start = std::chrono::system_clock::now();
		dev->ActivateTriggerIn(TRIGGER, START_TIMER);

		dev->ReadFromPipeOut(PIPE_OUT, r->pattern_size, data);

		dev->ActivateTriggerIn(TRIGGER, STOP_TIMER);
		timer_stop = std::chrono::system_clock::now();

		r->pc_duration_total += (timer_stop - timer_start);
		generateData(data);
	}
}

// WRITE
void Write::performTimer(unsigned char *data)
{
	prepareForTransfer(data);
	generateData(data);
	timer_start = std::chrono::system_clock::now();
	dev->ActivateTriggerIn(TRIGGER, START_TIMER);
	for (auto i=0; i<cfgs.iterations; i++)
	{
		dev->ActivateTriggerIn(TRIGGER, RESET_PATTERN);
		dev->WriteToPipeIn(PIPE_IN, r->pattern_size, data);
	}
	dev->ActivateTriggerIn(TRIGGER, STOP_TIMER);
	timer_stop = std::chrono::system_clock::now();
	r->pc_duration_total = timer_stop - timer_start;
}

// DUPLEX
void Duplex::performTimer(unsigned char *data)
{
	prepareForTransfer(data);
	unsigned char *send_data;
	unsigned char *received_data = new unsigned char[r->block_size];
	int errors = 0;
	for (auto i=0; i<cfgs.iterations; i++)
	{
		for (auto j = 0; j < r->pattern_size; j+=r->block_size)
		{
			dev->ActivateTriggerIn(TRIGGER, RESET);
			send_data = data + j;

			timer_start = std::chrono::system_clock::now();
			dev->ActivateTriggerIn(TRIGGER, START_TIMER);

			dev->WriteToPipeIn(PIPE_IN, r->block_size, send_data);
			dev->ReadFromPipeOut(PIPE_OUT, r->block_size, received_data);

			dev->ActivateTriggerIn(TRIGGER, STOP_TIMER);
			timer_stop = std::chrono::system_clock::now();
			r->pc_duration_total += (timer_stop - timer_start);

			// Error checking
			if (std::equal(send_data, send_data + r->block_size, received_data))
			{
				DLOG(INFO) << "Duplex: send data is equal to received data";
			}
			else
			{
				DLOG(ERROR) << "Duplex: send data is NOT equal to received data";
				errors += 1;
			}
		}
	}
	r->errors = errors;

	delete[] received_data;
}
