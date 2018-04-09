#ifndef FIFO_PERFORMANCE_H__
#define FIFO_PERFORMANCE_H__
// TODO: Change some logs to DLOG
#include <chrono>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <string>

#include <glog/logging.h>
#include <libconfig.h++>
#include <okFrontPanelDLL.h>

constexpr int MAX_PATTERN_SIZE {1073741824};
constexpr double FIFO_CLOCK    {100.8};

enum Widths     {BIT32, NONSYM, DUPLEX};
enum Directions {READ, WRITE};
enum Memories   {BLOCKRAM, DISTRIBUTEDRAM, SHIFTREGISTER};
enum Patterns   {COUNTER_8BIT, COUNTER_32BIT, WALKING_1};
enum Triggers   {RESET, START_TIMER, STOP_TIMER, RESET_PATTERN};
enum Endpoints
{
    NUMBER_OF_COUNTS_A = 0x20,
    NUMBER_OF_COUNTS_B = 0x21,
    ERROR_COUNT = 0x22,
    PATTERN_TO_GENERATE = 0x00,
    PIPE_IN = 0x80,
    PIPE_OUT = 0xa0,
    TRIGGER = 0x40
};


class Configurations 
{
    public:
        Configurations(const char *path_to_cfg)
        {
            DLOG(INFO) << "Initialization Configuration class";
            libconfig::Config cfg;
            openConfigFile(path_to_cfg, cfg);
            configureOutput(cfg);
            configureParams(cfg);
            LOG(INFO) << "Configuration class fully initialized";
        }

        ~Configurations()
        {
            DLOG(INFO) << "Destroying FIFO config class";
        }
        
        std::string bitfiles_path;

        // Parameters from 'output' scope
        std::string results_path;
        std::string result_sep;

        // Parameters from 'params' scope
        std::vector<std::string> mode_v;
        std::vector<std::string> direction_v;
        std::vector<std::string> memory_v;
        std::vector<unsigned int> depth_v;
        std::vector<unsigned int> pattern_size_v;
        std::vector<unsigned int> pattern_size_duplex_v;
        std::vector<unsigned int> block_size_v;
        std::vector<std::string> pattern_v;
        unsigned int statistic_iter;
        unsigned int iterations;

        // Default hashes for params
        std::map<std::string, unsigned int> mode_m 
            {{"32bit", BIT32}, {"nonsym", NONSYM}, {"duplex", DUPLEX}};
        std::map<std::string, unsigned int> direction_m 
            {{"read", READ}, {"write", WRITE}};
        std::map<std::string, unsigned int> pattern_m 
            {{"counter_8bit", COUNTER_8BIT}, {"counter_32bit", COUNTER_32BIT}, 
             {"walking_1", WALKING_1}};
        
        void writeHeadersToResultFile();

    private:
        const std::regex path_regex{"(\\.|\\.\\.)[a-zA-Z0-9/\\ _-]*/$"};
        std::vector<std::string> headers_v;
        
        // Default values for paramaters TODO: change to const
        std::vector<std::string> headers_default {"Time", "Mode", "Direction",
                "FifoMemoryType", "FifoDepth", "PatternSize", "DataPattern", 
                "Iterations", "StatisticalIter", "CountsInFPGA", "FPGA time(total) [us]", 
                "FPGA time(per iteration) [us]", "PC time(total) [us]", 
                "PC time(per iteration) [us]", "SpeedPC [B/s]", "SpeedFPGA [B/s]", "Errors"};
        std::vector<std::string> mode_default {"32bit", "nonsym", "duplex"};
        std::vector<std::string> direction_default {"read", "write"};
        std::vector<std::string> memory_default {"blockram", "distributedram", "shiftregister"};
        std::vector<unsigned int> depth_default {16, 64, 256, 1024};
        std::vector<unsigned int> pattern_size_default;
        std::vector<unsigned int> pattern_size_duplex_default;
        std::vector<unsigned int> block_size_default {16, 64, 256, 1024};
        std::vector<std::string> pattern_default {"counter_8bit", "counter_32bit", "walking_1"};

        template <class T>
        void vectorParser (std::vector<T> &parse_v, std::vector<T> &default_v, 
                           const libconfig::Setting &setting, const char *option);

        void integerParams(const libconfig::Setting &params);
        void configureParams(libconfig::Config &cfg);
        void configureOutputParameters(const libconfig::Setting &output);
        void configureOutputBitfiles(libconfig::Config &cfg);
        void configureOutput(libconfig::Config &cfg);
        void openConfigFile(const char *cfg_path, libconfig::Config &cfg);
};

class Results
{
    public:
        Results(okCFrontPanel *dev, Configurations &cfgs) : dev{dev}, cfgs{cfgs}
        {
            DLOG(INFO) << "Results class initialized";
        };

        ~Results()
        {
            DLOG(INFO) << "Destroying Results class";
        };

        unsigned int block_size, depth, errors, pattern_size, stat_iteration;
        std::string mode, direction, memory, pattern;
        std::chrono::duration<double, std::micro> pc_duration_total;

        void saveResultsToFile();

    private:
        const int MEGA = 1000000;
        double fpga_time_total, fpga_time_periteravg;
        double pc_time_total, pc_time_periteravg;
        double fpga_speed, pc_speed;
        uint64_t fpga_counts;
        okCFrontPanel *dev;
        Configurations &cfgs;

        const std::string logTime();
        void countPCTime();
        void countFPGATime();
};

class TransferTest
{
    public:
        TransferTest(okCFrontPanel *dev, Configurations &cfgs) : dev{dev}, cfgs{cfgs}
        {
            DLOG(INFO) << "TransferTest class initialized";
        }
        ~TransferTest()
        {
            DLOG(INFO) << "Destroying TransferTest class";
        }

        void performTransferTest();
    
    private:
        okCFrontPanel *dev;
        Configurations &cfgs;
        Results *r;

        void duplexTimer(unsigned char *data, const int &block_size);
        void runDuplexMode(std::vector<std::string> &memory_v);
        void writeTimer(unsigned char *data);
        void determineRegisterParameters(unsigned int mode, unsigned int &register_size, uint64_t &max_register_size);
        void generatedDataToWrite(unsigned char* data);
        unsigned int checkErrorsFromRead(unsigned char *data);
        void readTimer(unsigned char *data);
        void runTestBasedOnParameters();
        void checkIfOpen();
        void setupFPGA();
        void runOnSpecificDepth(std::vector<unsigned int> &depth_v);
        void specifyDepth();
        void runOnSpecificMemory(std::vector<std::string> &memory_v);
        void runOnSpecificMode();
};

class ITimer
{
    public:
        ITimer(okCFrontPanel *dev, Results *r, Configurations &cfgs) :
        dev{dev}, r{r}, cfgs{cfgs}
        {
            DLOG(INFO) << "Timer interface initialized";
        }
        virtual ~ITimer() {}
        void performTimer(unsigned char *data); // TODO: Can be private?
    
    private:
        okCFrontPanel *dev;
        Configurations &cfgs;
        Results *r;
        std::chrono::time_point<std::chrono::system_clock> timer_start, timer_stop;
        bool checkForErrors;

        virtual void timer(unsigned char *data) = 0;
        void generateData(unsigned char *data);
        void determineRegisterParameters(unsigned int mode, unsigned int &register_size, uint64_t &max_register_size)
        void performActionOnGeneratedData(const unsigned char &data_char, unsigned char *data, int index);
};

void ITimer::performTimer(unsigned char *data)
{
    r->pc_duration_total = std::chrono::nanoseconds::zero();
    r->errors = 0;
    dev->SetWireInValue(PATTERN_TO_GENERATE, cfgs.pattern_m[r->pattern]);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TRIGGER, RESET);
    void timer(unsigned char *data);
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

void ITimer::performActionOnGeneratedData(const unsigned char &data_char, unsigned char *data, int index)
{
    if (checkForErrors)
    {
        if (data[index] != data_char) r->errors += 1;
    }
    else
    {
        data[index] = data_char;
    }
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
            // data[i] = static_cast<unsigned char>(iter);
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
                // data[i+j] = static_cast<unsigned char>((iter >> j*8) & 0xFF);
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
                // data[i+j] = static_cast<unsigned char>((iter >> j*8) & 0xFF);
                performActionOnGeneratedData(data_char, data, i+j);
            }
            
            if (iter == last_possible_value) iter = 1;
            else iter *= 2;
        }
    }

    DLOG(INFO) << "Data to write generated";
}


class Read : public ITimer
{
    public:
        Read(okCFrontPanel *dev, Results *r, Configurations &cfgs) :
        ITimer(dev, r, cfgs), checkForErrors{true}
        {
            DLOG(INFO) << "Read class initialized";
        }

    private:
        virtual void timer(unsigned char *data);

};

void Read::timer(unsigned char *data)
{
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

class Write : public ITimer
{
    public:
        Write(okCFrontPanel *dev, Results *r, Configurations &cfgs) :
        ITimer(dev, r, cfgs), checkForErrors{false}
        {
            DLOG(INFO) << "Write class initialized";
        }

    private:
        virtual void timer(unsigned char *data);

};

void Write::timer(unsigned char *data)
{
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

class Duplex : public ITimer
{
    public:
        Duplex(okCFrontPanel *dev, Results *r, Configurations &cfgs) :
        ITimer(dev, r, cfgs), checkForErrors{false}
        {
            DLOG(INFO) << "Duplex class initialized";
        }

        void runDuplexMode(std::vector<std::string> &memory_v);

    private:
        virtual void timer(unsigned char *data);

};

void Duplex::timer(unsigned char *data)
{
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

void Duplex::runDuplexMode(std::vector<std::string> &memory_v)
{
    r->direction = "bidir";
    r->depth = 1024; // TODO: be sure that this will work


    for (const auto &memory : memory_v)
    {
        r->memory = memory;
        DLOG(INFO) << "Duplex FIFO memory mode set to: " << memory;
        setupFPGA(); // TODO: Will not work!!
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
                        
                        generatedData(data);
                        performTimer(data);
                        r->saveResultsToFile();
                        delete[] data;
                    }
                }
            }
        }
    }
}

#endif // FIFO_PERFORMANCE_H__