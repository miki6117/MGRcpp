#ifndef FIFO_CONFIG_H__
#define FIFO_CONFIG_H__
#include<string>
#include "libconfig.h++"

enum Widths {BIT32, NONSYM, DUPLEX};
enum Directions {READ, WRITE};
enum Memories {BLOCKRAM, DISTRIBUTEDRAM, SHIFTREGISTER};
enum Patterns {COUNTER_8BIT, COUNTER_32BIT, WALKING_1};
enum Triggers {RESET, START_TIMER, STOP_TIMER, RESET_PATTERN};
enum Endpoints
{
    NUMBER_OF_COUNTS_A = 0x20,
    NUMBER_OF_COUNTS_B = 0x21,
    ERROR_COUNT = 0x22,
    PATTERN_TO_GENERATE = 0x01, // TODO: Really? Check!
    PIPE_IN = 0x80,
    PIPE_OUT = 0xa0,
    TRIGGER = 0x40
}; // TODO: enums in global header file!!

constexpr regex path_regex{"(\\.|\\.\\.)[a-zA-Z0-9/\\ _-]*/$"};

class FifoTestConfig 
{
    public:
        FifoTestConfig (std::string path_to_cfg) : path_to_cfg{path_to_cfg}
        {
            // this -> path_to_cfg = path_to_cfg;
        }
        
    private:
        Config cfg;
        const char* cfg_path;
        bool show_logs_stdout;
        std::string path_to_cfg;
        std::string logs_path;
        std::string logfile_name;
        std::string results_path;
        std::string bitfiles_path;
        std::string resultfile_name;
        char result_sep;

        vector<std::string> width_v;
        vector<std::string> memory_v;
        vector<unsigned int> depth_v;
        vector<std::string> direction_v;
        vector<unsigned int> pattern_size_v;
        vector<std::string> pattern_v;
        map<std::string, unsigned int> pattern_m;
        map<std::string, unsigned int> width_m;
        map<std::string, unsigned int> direction_m;
};

#endif // FIFO_CONFIG_H__