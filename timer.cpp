#include "performance.h"

// INTERFACE
void ITimer::performActionOnData(unsigned char *data, unsigned int pattern_size)
{
	DataGenerator datagen(mode, pattern, pattern_size);
	if (check_for_errors)
	{
		errors += datagen.checkArrayForErrors(data);
	}
	else
	{
		datagen.fillArrayWithData(data);
	}
}

void ITimer::prepareForTransfer()
{
	pc_duration_total = std::chrono::nanoseconds::zero();
	errors = 0;
	dev->SetWireInValue(PATTERN_TO_GENERATE, pattern);
	dev->UpdateWireIns();
	dev->ActivateTriggerIn(TRIGGER, RESET);
}

// READ
void Read::performTimer(unsigned int pattern_size, unsigned int iterations)
{
	prepareForTransfer();
	unsigned char *data = new unsigned char[pattern_size];
	for (unsigned int i=0; i<iterations; i++)
	{
		DLOG(INFO) << "Current iteration: " << i;
		dev->ActivateTriggerIn(TRIGGER, RESET_PATTERN);
		timer_start = std::chrono::system_clock::now();
		dev->ActivateTriggerIn(TRIGGER, START_TIMER);

		dev->ReadFromPipeOut(PIPE_OUT, pattern_size, data);

		dev->ActivateTriggerIn(TRIGGER, STOP_TIMER);
		timer_stop = std::chrono::system_clock::now();

		pc_duration_total += (timer_stop - timer_start);
		performActionOnData(data, pattern_size);
	}
	delete[] data;
}

// WRITE
void Write::performTimer(unsigned int pattern_size, unsigned int iterations)
{
	prepareForTransfer();
	unsigned char *data = new unsigned char[pattern_size];
	performActionOnData(data, pattern_size);
	timer_start = std::chrono::system_clock::now();
	dev->ActivateTriggerIn(TRIGGER, START_TIMER);
	for (unsigned int i=0; i<iterations; i++)
	{
		dev->ActivateTriggerIn(TRIGGER, RESET_PATTERN);
		dev->WriteToPipeIn(PIPE_IN, pattern_size, data);
	}
	dev->ActivateTriggerIn(TRIGGER, STOP_TIMER);
	timer_stop = std::chrono::system_clock::now();
	pc_duration_total = timer_stop - timer_start;
	delete[] data;
}

// DUPLEX
void Duplex::checkIfReceivedEqualsSend(unsigned char *send_data, unsigned char *received_data)
{
	if (std::equal(send_data, send_data + block_size, received_data))
	{
		DLOG(INFO) << "Duplex: send data is equal to received data";
	}
	else
	{
		DLOG(ERROR) << "Duplex: send data is NOT equal to received data";
		errors += 1;
	}
}

void Duplex::performTimer(unsigned int pattern_size, unsigned int iterations)
{
	prepareForTransfer();
	unsigned char *data = new unsigned char[pattern_size];
	performActionOnData(data, pattern_size);
	unsigned char *received_data = new unsigned char[block_size];
	unsigned char *send_data;
	for (unsigned int i=0; i<iterations; i++)
	{
		for (unsigned int j = 0; j < pattern_size; j+=block_size)
		{
			send_data = data + j;

			timer_start = std::chrono::system_clock::now();
			dev->ActivateTriggerIn(TRIGGER, START_TIMER);

			dev->WriteToPipeIn(PIPE_IN, block_size, send_data);
			dev->ReadFromPipeOut(PIPE_OUT, block_size, received_data);

			dev->ActivateTriggerIn(TRIGGER, STOP_TIMER);
			timer_stop = std::chrono::system_clock::now();
			pc_duration_total += (timer_stop - timer_start);

			// Error checking
			checkIfReceivedEqualsSend(send_data, received_data);
		}
	}

	delete[] received_data;
	delete[] data;
}
