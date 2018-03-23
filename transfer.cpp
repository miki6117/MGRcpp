#include "performance.h"

void TransferTest::runTestBasedOnParameters()
{
    DLOG(INFO) << "Current width mode: " << r->mode;
    DLOG(INFO) << "Current direction transfer: " << r->direction;
    DLOG(INFO) << "Current FIFO memory: " << r->memory;
    DLOG(INFO) << "Current FIFO depth value: " << r->depth;
    DLOG(INFO) << "Current size: " << r->pattern_size;
    DLOG(INFO) << "Current pattern: " << r->pattern;

    LOG(INFO) <<  "Detecting test mode...";
    unsigned char* data = new unsigned char[r.pattern_size];
    auto dir = cfgs.direction_m[r->direction];

    if (dir == READ)
    {
        LOG(INFO) << "Read mode detected";
        readModeTimer(dev, r, data);
    }
    else if (dir == WRITE)
    {
        LOG(INFO) << "Write mode detected";
        generatedDataToWrite(data, patterns_m[r.pattern], r.pattern_size, width_m[r.width]);
        writeModeTimer(dev, r, data);
    }
    saveResultToFile(r);
    delete[] data;
}

void TransferTest::checkIfOpen()
{
    LOG(INFO) << "Checking if device is open...";
    if (dev->IsOpen())
    {
        LOG(INFO) << "Device is open";
    }
    else
    {
        LOG(FATAL) << "Device disconnected. Program stopped";
    }
}

void TransferTest::setupFPGA()
{
    std::string bitfiles = cfgs.bitfiles_path + r->mode + "/";
    LOG(INFO) << "Path to bitfiles for current transfer mode: " << bitfiles;
    std::string bitfile_name = r->direction + "_" + r->mode + "_fifo_" + r->memory + \
                               "_" + std::to_string(r->depth) + ".bit";
    std::string bitfile_to_load = bitfiles + bitfile_name;
    LOG(INFO) << "FPGA configure file: " << bitfile_to_load;
    auto err_code = dev->ConfigureFPGA(bitfile_to_load);
    if (err_code == okCFrontPanel::NoError) 
    {
        LOG(INFO) << "Configure status for file " << bitfile_name << " : all ok";
	} 
    else 
    {
        LOG(FATAL) << "FPGA configuration failed: " << dev->GetErrorString(err_code);
    }
}

void TransferTest::runonSpecificDepth(std::vector<unsigned int> &depth_v)
{
    for (const auto &depth : depth_v)
    {
        r->depth = depth;
        LOG(INFO) << "FIFO depth value set to: " << depth;
        setupFPGA();
        for (const auto &size : cfgs.pattern_size_v)
        {
            r->pattern_size = size;
            LOG(INFO) << "Size set to: " << size;
            checkIfOpen();
            for (const auto &pattern : cfgs.pattern_v)
            {
                for (auto i = 1; i <= cfgs.statistic_iter; i++)
                {
                    LOG(INFO) << "Current statistical iteration: " << i;
                    r->stat_iteration = i;
                    r->pattern = pattern;
                    LOG(INFO) << "Current pattern: " << pattern;
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

    if (mode == NONSYM && direction == WRITE)
    {
        specific_depth_v = {32, 64, 256, 1024}; //TODO: Fill in
    }
    else
    {
        specific_depth_v = cfgs.depth_v;
    }
    runonSpecificDepth(specific_depth_v);
}

void TransferTest::runOnSpecificMemory(std::vector<std::string> &memory_v)
{
    for (const auto &direction : cfgs.direction_v)
    {
        r->direction = direction;
        LOG(INFO) << "Direction transfer mode set to: " << direction;
        for (const auto &memory : memory_v)
        {
            r->memory = memory;
            LOG(INFO) << "FIFO memory mode set to: " << memory;
            specifyDepth();
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
            break;
        
        case NONSYM:
            memory_v_for_specific_mode = {"blockram"};
            LOG(WARNING) << "FYI: For nonsym mode, the only valid memory is blockram";
            break;

        case DUPLEX:
            break;
    }
    LOG(INFO) << "Initialized memory vector for: " << r->mode;

    runOnSpecificMemory(memory_v_for_specific_mode);
}

void TransferTest::performTransferTest()
{
    r = new Results(dev, cfgs);
    for (const auto &mode : cfgs.mode_v)
    {
        r->mode = mode;
        LOG(INFO) <<  "Transfer mode set to: : " << mode;
        runOnSpecificMode();
    }
    delete r;
}