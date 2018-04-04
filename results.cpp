#include "performance.h"

const std::string Results::logTime()
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
    pc_time_periteravg = pc_time_total / cfgs.iterations;
    pc_speed = static_cast<double>(pattern_size) * MEGA / pc_time_periteravg;
    LOG(INFO) << "Counted PC time for single duration: " << pc_time_periteravg << " msec";
    LOG(INFO) << "Counted speed on PC side: " << pc_speed << " B/s";
}

void Results::countFPGATime()
{
    dev->UpdateWireOuts();
    fpga_counts = dev->GetWireOutValue(NUMBER_OF_COUNTS_A);
    fpga_counts += static_cast<uint64_t>(dev->GetWireOutValue(NUMBER_OF_COUNTS_B) << 32);
    if (cfgs.direction_m[direction] == WRITE) errors = dev->GetWireOutValue(ERROR_COUNT);

    fpga_time_total = fpga_counts / FIFO_CLOCK;

    fpga_time_periteravg = fpga_time_total / cfgs.iterations;
    fpga_speed = static_cast<double>(pattern_size) * MEGA / fpga_time_periteravg;
    LOG(INFO) << "FPGA clock counts: " << fpga_counts;
    LOG(INFO) << "Counted FPGA total transfer time: " << fpga_time_total << " msec";
    LOG(INFO) << "Counted FPGA time for single duration: " << fpga_time_periteravg << " msec";
    LOG(INFO) << "Counted speed on FPGA side: " << fpga_speed << " B/s";
    LOG(WARNING) << "Errors detected during transfer: " << errors;
}

void Results::saveResultsToFile()
{
    countPCTime();
    countFPGATime();
    std::fstream result_file;
    std::string rs = cfgs.result_sep;
    result_file.open(cfgs.results_path, std::ios::out | std::ios::app);
    if (result_file.good())
    {
        result_file << logTime() << rs;
        result_file << mode << rs;
        result_file << direction << rs;
        result_file << memory << rs;
        result_file << depth << rs;
        result_file << pattern_size << rs;
        result_file << block_size << rs;
        result_file << pattern << rs;
        result_file << cfgs.iterations << rs;
        result_file << stat_iteration << rs;
        result_file << fpga_counts << rs;
        result_file << fpga_time_total << rs;
        result_file << fpga_time_periteravg << rs;
        result_file << pc_time_total << rs;
        result_file << pc_time_periteravg << rs;
        result_file << pc_speed << rs;
        result_file << fpga_speed  << rs;
        result_file << errors;
        result_file << std::endl;
        result_file.close();
        LOG(INFO) << "All results saved to " << cfgs.results_path;
    }
    else
    {
        LOG(FATAL) << "Unable to open " << cfgs.results_path << " file during saving results";
    }
}