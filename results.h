#ifndef FIFO_RESULTS_H__
#define FIFO_RESULTS_H__
#include<chrono>
#include<cstdint>
#include<fstream>
#include<string>
#include "okFrontPanelDLL.h"

// TODO: fstream object initialize earlier?? As it need be init with header!
// TODO: migrate functions to implementation cpp file
class Results // TODO: add config object as pointer
{
    public:
        Results(char results_separator, okCFrontPanel *dev, std::string results_file_name) : // TODO: if config object passed, then delete separator and results_file_name
                rs{results_separator},
                dev{dev},
                results_file_name{results_file_name}
        {
            LOG(INFO) << "Results class initialized";
        };

        ~Results()
        {
            LOG(INFO) << "Destroying Results class";
        };

        const char rs;
        okCFrontPanel *dev;
        const std::string results_file_name;
        const int MEGA = 1000000;

        unsigned int depth, errors, pattern_size, stat_iteration;
        uint64_t fpga_counts;
        double fpga_time_total, fpga_time_periteravg;
        double fpga_speed, pc_speed;
        double pc_time_total, pc_time_periteravg;
        std::string direction, memory, pattern, width;
        std::chrono::duration<double, std::micro> pc_duration_total;

        void saveResultsToFile();

    private:
        void countPCTime();
        void countFPGATime();
        const std::string logTime();
        
};

const std::string Results::logTime() // TODO: #include<ctime>
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

    return buf;
}

void Results::countPCTime()
{
    pc_time_total = pc_duration_total.count();
    pc_time_periteravg = pc_time_total / iterations;// TODO: iterations from config file!!
    pc_speed = static_cast<double>(pattern_size) * MEGA / pc_time_periteravg;
    LOG(INFO) << "Counted PC time for single duration: " << pc_time_periteravg << " msec";
    LOG(INFO) << "Counted speed on PC side: " << pc_speed << " B/s";
}

void Results::countFPGATime()
{
    uint64_t number_of_counts;
    dev->UpdateWireOuts();
    number_of_counts = dev->GetWireOutValue(NUMBER_OF_COUNTS_A);
    number_of_counts += static_cast<uint64_t>(dev->GetWireOutValue(NUMBER_OF_COUNTS_B) << 32);
    if (direction_m[direction] == WRITE) errors = dev->GetWireOutValue(ERROR_COUNT); // TODO: direction_m from config file!

    fpga_time_total = number_of_counts / FIFO_CLOCK;

    fpga_time_periteravg = fpga_time_total / iterations;
    fpga_speed = static_cast<double>(pattern_size) * MEGA / fpga_time_periteravg; // TODO: logs!
    LOG(INFO) << "FPGA clock counts: " << number_of_counts;
    LOG(INFO) << "Counted FPGA total transfer time: " << fpga_time_total << " msec";
    LOG(INFO) << "Counted FPGA time for single duration: " << fpga_time_periteravg << " msec";
    LOG(INFO) << "Counted speed on FPGA side: " << fpga_speed << " B/s";
    LOG(WARNNG) << "Errors detected during transfer: " << errors;
}

void Results::saveResultsToFile()
{
    countPCTime();
    countFPGATime();
    std::fstream result_file;
    result_file.open(results_file_name, std::ios::out | std::ios::app);
    if (result_file.good())
    {
        result_file << logTime() << rs << width << rs << direction << rs;
        result_file << memory << rs << depth << rs << pattern_size << rs;
        result_file << pattern << rs << iterations << rs << stat_iteration << rs << fpga_counts << rs;
        result_file << fpga_time_total << rs << fpga_time_periteravg << rs;
        result_file << pc_time_total << rs << pc_time_periteravg << rs;
        result_file << pc_speed << rs << fpga_speed  << rs << errors;
        result_file << std::endl;
        result_file.close();
        LOG(INFO) << "All results saved to " << results_file_name;
    }
    else
    {
        LOG(FATAL) << "Unable to open " << results_file_name << " file during saving results";
    }
}

#endif // FIFO_RESULTS_H__