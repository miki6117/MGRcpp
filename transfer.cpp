#include "performance.h"
#include <limits>
#include <iostream> // TODO: delete

#undef max

void TransferTest::writeTimer(unsigned char *data)
{
    // std::chrono::duration<double, std::micro> pc_duration_total;
    std::chrono::time_point<std::chrono::system_clock> timer_start, timer_stop;

    r->pc_duration_total = std::chrono::nanoseconds::zero();
    dev->SetWireInValue(PATTERN_TO_GENERATE, cfgs.pattern_m[r->pattern]);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TRIGGER, RESET);
    LOG(INFO) << "START iterations";
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
    // dev->UpdateWireOuts();
    // r.errors = dev->GetWireOutValue(ERROR_COUNT);

    LOG(INFO) << "Stop iterations";
}

void TransferTest::determineRegisterParameters(unsigned int mode, unsigned int &register_size, uint64_t &max_register_size)
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


void TransferTest::generatedDataToWrite(unsigned char* data)
{
    unsigned int register_size;
    uint64_t max_register_size;
    auto pattern = cfgs.pattern_m[r->pattern];

    // checkIfDataIsInit(data);
    determineRegisterParameters(cfgs.mode_m[r->mode], register_size, max_register_size);

    uint64_t iter = 0;
    if (pattern == COUNTER_8BIT)
    {
        for (auto i = 0; i < r->pattern_size; i++)
        {
            data[i] = static_cast<unsigned char>(iter);
            if (iter > max_register_size) iter = 0;
            else ++iter;
        }
    }
    else if (pattern == COUNTER_32BIT)
    {
        for (auto i = 0; i < r->pattern_size; i+=register_size)
        {
            for (auto j = 0; j < register_size; j++)
                data[i+j] = static_cast<unsigned char>((iter >> j*8) & 0xFF);
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
                data[i+j] = static_cast<unsigned char>((iter >> j*8) & 0xFF);
            
            if (iter == last_possible_value) iter = 1;
            else iter *= 2;
        }
    }

    DLOG(INFO) << "Data to write generated";
}

unsigned int TransferTest::checkErrorsFromRead(unsigned char *data)
{
    unsigned int errors = 0;
    unsigned int register_size = 4;
    uint64_t max_register_size = std::numeric_limits<int>::max();
    auto pattern = cfgs.pattern_m[r->pattern];

    if (cfgs.mode_m[r->mode] == NONSYM)
    {
        register_size = 8;
        max_register_size = std::numeric_limits<uint64_t>::max();
    }
    DLOG(INFO) << "Register size set to: " << register_size;


    if (pattern == COUNTER_8BIT)
    {
        uint64_t iter = 0;
        for (auto i = 0; i < r->pattern_size; i++)
        {
            if (data[i] != static_cast<unsigned char>(iter)) errors += 1;
            if (iter > max_register_size) iter = 0;
            else ++iter;
        }
    }

    else if (pattern == COUNTER_32BIT)
    {
        uint64_t iter = 0;
        for (auto i = 0; i < r->pattern_size; i+=register_size)
        {
            for (auto j = 0; j < register_size; j++)
            {
                if (data[i+j] != static_cast<unsigned char>((iter >> j*8) & 0xFF)) errors += 1;
            }
            if (iter > max_register_size) iter = 0;
            else ++iter;
        }
    }

    else if (pattern == WALKING_1)
    {
        uint64_t iter = 1;
        uint64_t last_possible_value = max_register_size / 2 + 1;
        for (auto i = 0; i < r->pattern_size; i+=register_size)
        {
            for (auto j = 0; j < register_size; j++)
            {
                if (data[i+j] != static_cast<unsigned char>((iter >> j*8) & 0xFF)) errors +=1;
            }
            if (iter == last_possible_value) iter = 1;
            else iter *= 2;
        }
    }

    else
    {
        LOG(FATAL) << "Wrong pattern to check";
    }

    DLOG(INFO) << "Errors detected: " << errors;
    return errors;
}

void TransferTest::readTimer(unsigned char *data)
{
    // std::chrono::duration<double, std::micro> pc_duration_total;
    std::chrono::time_point<std::chrono::system_clock> timer_start, timer_stop;

    r->pc_duration_total = std::chrono::nanoseconds::zero();
    dev->SetWireInValue(PATTERN_TO_GENERATE, cfgs.pattern_m[r->pattern]);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TRIGGER, RESET);
    LOG(INFO) << "START iterations";
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
        r->errors = checkErrorsFromRead(data);
    }
    LOG(INFO) << "Stop iterations";
}

void TransferTest::runTestBasedOnParameters()
{
    DLOG(INFO) << "Current mode: " << r->mode;
    DLOG(INFO) << "Current direction transfer: " << r->direction;
    DLOG(INFO) << "Current FIFO memory: " << r->memory;
    DLOG(INFO) << "Current FIFO depth value: " << r->depth;
    DLOG(INFO) << "Current size: " << r->pattern_size;
    DLOG(INFO) << "Current pattern: " << r->pattern;

    unsigned char* data = new unsigned char[r->pattern_size];
    auto mode = cfgs.mode_m[r->mode];
    auto dir = cfgs.direction_m[r->direction];

    if (mode != DUPLEX)
    {
        if (dir == READ)
        {
            DLOG(INFO) << "Setting read timer";
            readTimer(data);
        }
        else if (dir == WRITE)
        {
            LOG(INFO) << "Setting write timer";
            generatedDataToWrite(data);
            writeTimer(data);
        }
    }
    r->saveResultsToFile();
    delete[] data;
}

void TransferTest::checkIfOpen()
{
    DLOG(INFO) << "Checking if device is open...";
    if (dev->IsOpen())
    {
        DLOG(INFO) << "Device is open";
    }
    else
    {
        LOG(FATAL) << "Device disconnected. Program stopped";
    }
}

void TransferTest::setupFPGA()
{
    std::string bitfiles = cfgs.bitfiles_path + r->mode + "/";
    DLOG(INFO) << "Path to bitfiles for current transfer mode: " << bitfiles;
    std::string bitfile_name = r->direction + "_" + r->mode + "_fifo_" + r->memory + \
                               "_" + std::to_string(r->depth) + ".bit";
    std::string bitfile_to_load = bitfiles + bitfile_name;
    DLOG(INFO) << "FPGA configure file: " << bitfile_to_load;
    auto err_code = dev->ConfigureFPGA(bitfile_to_load);
    if (err_code == okCFrontPanel::NoError) 
    {
        LOG(INFO) << "Configure status for file " << bitfile_name << " : all ok";
	} 
    else 
    {
        LOG(FATAL) << "FPGA configuration failed [" << dev->GetErrorString(err_code)
                   << "] for file " << bitfile_name;
    }
}

void TransferTest::runOnSpecificDepth(std::vector<unsigned int> &depth_v)
{
    for (const auto &depth : depth_v)
    {
        r->depth = depth;
        DLOG(INFO) << "FIFO depth value set to: " << depth;
        setupFPGA();
        for (const auto &size : cfgs.pattern_size_v)
        {
            r->pattern_size = size;
            r->block_size = size;
            DLOG(INFO) << "Pattern size set to: " << size;
            checkIfOpen();
            for (const auto &pattern : cfgs.pattern_v)
            {
                DLOG(INFO) << "Current pattern: " << pattern;
                for (auto i = 1; i <= cfgs.statistic_iter; i++)
                {
                    DLOG(INFO) << "Current statistical iteration: " << i;
                    r->stat_iteration = i;
                    r->pattern = pattern;
                    runTestBasedOnParameters();
                }
            }
        }
    }
}

void TransferTest::specifyDepth() // TODO: logs
{
    std::vector<unsigned int> specific_depth_v;
    auto direction = cfgs.direction_m[r->direction];
    auto mode = cfgs.mode_m[r->mode];

    DLOG(INFO) << "Specifying depth based on " << mode << " mode and " 
               << direction << " direction";

    if (mode == NONSYM && direction == WRITE)
    {
        specific_depth_v = {32, 64, 256, 1024}; //TODO: Fill in
        DLOG(INFO) << "Depth values specified for NONSYM mode and WRITE direction";
    }
    else
    {
        specific_depth_v = cfgs.depth_v;
        DLOG(INFO) << "Depth values copied from config file";
    }
    runOnSpecificDepth(specific_depth_v);
}

void TransferTest::runOnSpecificMemory(std::vector<std::string> &memory_v)
{
    for (const auto &direction : cfgs.direction_v)
    {
        r->direction = direction;
        DLOG(INFO) << "Direction transfer mode set to: " << direction;
        for (const auto &memory : memory_v)
        {
            r->memory = memory;
            DLOG(INFO) << "FIFO memory mode set to: " << memory;
            specifyDepth();
        }
    }
}

void TransferTest::duplexTimer(unsigned char *data, const int &block_size)
{
    std::chrono::time_point<std::chrono::system_clock> timer_start, timer_stop;
    unsigned char *send_data;
    unsigned char *received_data = new unsigned char[block_size];

    r->pc_duration_total = std::chrono::nanoseconds::zero();
    dev->SetWireInValue(PATTERN_TO_GENERATE, cfgs.pattern_m[r->pattern]);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TRIGGER, RESET);
    LOG(INFO) << "START iterations";
    int errors = 0;
    for (auto i=0; i<cfgs.iterations; i++)
    {
        for (auto j = 0; j < r->pattern_size; j+=block_size)
        {
            dev->ActivateTriggerIn(TRIGGER, RESET);
            send_data = data + j;

            timer_start = std::chrono::system_clock::now();
            dev->ActivateTriggerIn(TRIGGER, START_TIMER);

            dev->WriteToPipeIn(PIPE_IN, block_size, send_data);
            dev->ReadFromPipeOut(PIPE_OUT, block_size, received_data);

            dev->ActivateTriggerIn(TRIGGER, STOP_TIMER);
            timer_stop = std::chrono::system_clock::now();
            r->pc_duration_total += (timer_stop - timer_start);

            // Error checking
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
    }
    r->errors = errors;

    delete[] received_data;
    LOG(INFO) << "Stop iterations";
}

void TransferTest::runDuplexMode(std::vector<std::string> &memory_v)
{
    r->direction = "bidir";
    r->depth = 1024; // TODO: be sure that this will work


    for (const auto &memory : memory_v)
    {
        r->memory = memory;
        DLOG(INFO) << "Duplex FIFO memory mode set to: " << memory;
        setupFPGA();
        for (const auto &pattern_size : cfgs.pattern_size_duplex_v)
        {
            r->pattern_size = pattern_size;
            DLOG(INFO) << "Duplex pattern size set to: " << pattern_size;
            for (const auto &block_size : cfgs.block_size_v)
            {
                r->block_size = block_size;
                DLOG(INFO) << "Duplex block size set to: " << block_size;
                for (const auto &pattern : cfgs.pattern_v)
                {
                    r->pattern = pattern;
                    DLOG(INFO) << "Duplex current pattern: " << pattern;
                    for (auto i = 1; i <= cfgs.statistic_iter; i++)
                    {
                        r->stat_iteration = i;
                        DLOG(INFO) << "Duplex current statistical iteration: " << i;
                        unsigned char* data = new unsigned char[r->pattern_size];
                        
                        generatedDataToWrite(data);
                        duplexTimer(data, block_size);
                        r->saveResultsToFile();
                        delete[] data;
                    }
                }
            }
        }
    }
}

void TransferTest::runOnSpecificMode()
{
    auto mode = cfgs.mode_m[r->mode];
    std::vector<std::string> memory_v_for_specific_mode;

    switch(mode)
    {
        case BIT32:
            memory_v_for_specific_mode = cfgs.memory_v;
            DLOG(INFO) << "Initialized memory vector for: " << r->mode;
            runOnSpecificMemory(memory_v_for_specific_mode);
            break;
        
        case NONSYM:
            memory_v_for_specific_mode = {"blockram"};
            DLOG(WARNING) << "FYI: For nonsym mode, the only valid memory is blockram";
            runOnSpecificMemory(memory_v_for_specific_mode);
            break;

        case DUPLEX:
            memory_v_for_specific_mode = cfgs.memory_v;
            DLOG(INFO) << "Initialized memory vector for: " << r->mode;
            runDuplexMode(memory_v_for_specific_mode);
            break;
    }
}

void TransferTest::performTransferTest()
{
    r = new Results(dev, cfgs);
    DLOG(INFO) << "Memory allocated for Results class";
    for (const auto &mode : cfgs.mode_v)
    {
        r->mode = mode;
        DLOG(INFO) <<  "Transfer mode set to: : " << mode;
        runOnSpecificMode();
    }
    delete r;
}