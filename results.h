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
                {}; //TODO: add log to constructor and destructor
        ~Results() {};

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
        
};

void Results::countPCTime() // TODO: add logs here
{
    pc_time_total = pc_duration_total.count();
    pc_time_periteravg = pc_time_total / iterations;// TODO: iterations from config file!!
    pc_speed = static_cast<double>(pattern_size) * MEGA / pc_time_periteravg;
}

void Results::countFPGATime()
{
    uint64_t number_of_counts;
    dev->UpdateWireOuts();
    number_of_counts = dev->GetWireOutValue(NUMBER_OF_COUNTS_A); //TODO: enum from global header file
    number_of_counts += static_cast<uint64_t>(dev->GetWireOutValue(NUMBER_OF_COUNTS_B) << 32);
    if (direction_m[direction] == WRITE) errors = dev->GetWireOutValue(ERROR_COUNT); // TODO: direction_m from config file!

    fpga_time_total = number_of_counts / FIFO_CLOCK; // TODO: logs!
    //TODO: FIFO_CLOCK const declaration!
    // logAction("RESULT: Counts in FPGA: " + to_string(number_of_counts), false, false);
    // logAction("RESULT: Number of errors during transfer: " + to_string(r.errors), false, false);
    // logAction("RESULT: FPGA time [msec]: " + to_string(r.fpga_time_total / 1000), false, false);

    fpga_time_periteravg = fpga_time_total / iterations;
    fpga_speed = static_cast<double>(pattern_size) * MEGA / fpga_time_periteravg; // TODO: logs!
    // logAction("RESULT: FPGA single duration: " + to_string(r.fpga_time_periteravg), false, false);
    // logAction("RESULT: FPGA total speed [B/s]: " + to_string(r.fpga_speed), false, false);
}

void Results::saveResultsToFile()
{
    countPCTime();
    countFPGATime();
    std::fstream result_file;
    result_file.open(results_file_name, std::ios::out | std::ios::app);
 	if (result_file.good())
    {
        result_file << logTime() << rs << width << rs << direction << rs; // TODO: log time!!
        result_file << memory << rs << depth << rs << pattern_size << rs;
        result_file << pattern << rs << iterations << rs << stat_iteration << rs << fpga_counts << rs;
        result_file << fpga_time_total << rs << fpga_time_periteravg << rs;
        result_file << pc_time_total << rs << pc_time_periteravg << rs;
        result_file << pc_speed << rs << fpga_speed  << rs << errors;
        result_file << std::endl;
        result_file.close();
        // logAction("INFO: All results saved to file: " + results_path + resultfile_name);
    }
    else
    {
        // logAction("ERROR: Unable to open test_result.csv file", true);
    }
}

#endif // FIFO_RESULTS_H__