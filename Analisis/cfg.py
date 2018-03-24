"""Specify path to csv file that contains transfer results"""
CSV_FILE = './test_result.csv'

"""Specify separator in csv file"""
SEPARATOR = ';'

"""Specify number of statistical iterations based on config file"""
STATISTICAL_ITERATIONS = 10

"""Check if any error occured during transfer and return it on stdout"""
CHECK_FOR_ERRORS = False

"""Specify the target speed that will be collocated with pattern sizes on charts (also available: 'PC' and 'FPGA')"""
TARGET_SPEED = 'AV'

"""Each combination of parameters on separated chart? (default: no)"""
PARAMETERS_SEPARATED = False


"""Try not to modify this section on your own (except you know exactely what are you doing)!"""

"""Results that need casting to integers"""
INT_VALUES = ['FifoDepth', 'PatternSize', 'StatisticalIter', 'Iterations', 'Errors']

"""Results that need casting to floats"""
FLOAT_VALUES = ['PC time(total) [us]', 'PC time(iteration) [us]',
                'FPGA time(total) [us]', 'CountsInFPGA',
                'FPGA time(iteration) [us]', 'SpeedPC [B/s]',
                'SpeedFPGA [B/s]']

"""Names of heads after refactoring"""
REFACTORED_HEADS = ['Width', 'Direction', 'MemoryType', 'FifoDepth', 'PatternSize', 'DataPattern', 'SpeedPC', 'u(PC)', 'SpeedFPGA', 'u(FPGA)', 'Average', 'u(av)']

"""Metadata for figure objects"""
FIGURE_METADATA = {
    'xlabel' : 'Pattern size [B]',
    'xscale' : 'log',
    'ylabel_PC' : 'Speed PC [MB/s]',
    'ylabel_FPGA' : 'Speed FPGA [MB/s]',
    'ylabel_AV' : 'Average speed [MB/s]',
    'yticks' : [i for i in range(0, 401, 50)],
    'grid' : True,
    'error' : 'Target ylabel should be defined as PC, FPGA or AV'
}

"""Properties that can be combined with each other"""
BASIC_PROPERTIES = {
    'Width' : ['nonsym', '32bit'],
    'Direction' : ['read', 'write'],
    'MemoryType' : ['blockram', 'distributedram', 'shiftregister'],
    'FifoDepth': [16, 32, 64, 256, 1024],
    'DataPattern' : ['counter_8bit', 'counter_32bit', 'walking_1']
}

"""Combined parameters"""
PLOTTING_OPTIONS = {
    'memtype_depth_pattern' : {
        'title' : 'Fifo memory type: {}. Depth = {}',
        'savefig' : '{}_{}_{}_patterns.png',
        'first_param' : 'MemoryType',
        'second_param' : 'FifoDepth',
        'third_param' : 'DataPattern',
        'legend' : {
            'counter_8bit' : 'ro',
            'counter_32bit' : 'g*',
            'walking_1' : 'b+'
        }
    },
    'depth_pattern_memtype' : {
        'title' : 'Fifo depth: {}. Pattern type = {}',
        'savefig' : '{}_{}_{}_memory_types.png',
        'first_param' : 'FifoDepth',
        'second_param' : 'DataPattern',
        'third_param' : 'MemoryType',
        'legend' : {
            'blockram' : 'ro',
            'distributedram' : 'g*',
            'shiftregister' : 'b+'
        }
    },
    'pattern_memtype_depth' : {
        'title' : 'Pattern type: {}. Fifo memory type: {}',
        'savefig' : '{}_{}_{}_depths.png',
        'first_param' : 'DataPattern',
        'second_param' : 'MemoryType',
        'third_param' : 'FifoDepth',
        'legend' : {
            16 : 'ro',
            32 : 'y^',
            64 : 'g*',
            256 : 'b+',
            1024 : 'mv'
        }
    }
}
