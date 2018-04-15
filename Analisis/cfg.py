"""Specify path to csv file that contains transfer results"""
CSV_FILE = './AnalisisFromServer/test_resultVER1.csv'

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
INT_VALUES = ['FifoDepth', 'PatternSize', 'BlockSize', 'StatisticalIter', 'Iterations', 'Errors']

"""Results that need casting to floats"""
FLOAT_VALUES = ['PC time(total) [us]', 'PC time(per iteration) [us]',
				'FPGA time(total) [us]', 'CountsInFPGA',
				'FPGA time(per iteration) [us]', 'SpeedPC [B/s]',
				'SpeedFPGA [B/s]']

"""Names of heads after refactoring"""
REFACTORED_HEADS = ['Mode', 'Direction', 'FifoMemoryType', 'FifoDepth', 'PatternSize', 'BlockSize', 'DataPattern', 'SpeedPC', 'u(PC)', 'SpeedFPGA', 'u(FPGA)', 'Average', 'u(av)']

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
	'Mode' : ['nonsym', '32bit', 'duplex'],
	'Direction' : ['read', 'write', 'bidir'],
	'FifoMemoryType' : ['blockram', 'distributedram', 'shiftregister'],
	'FifoDepth': [16, 32, 64, 256, 1024],
	'BlockSize': [16, 64, 256, 1024],
	'DataPattern' : ['counter_8bit', 'counter_32bit', 'walking_1']
}

"""Combined parameters"""
PLOTTING_OPTIONS = {
	'memtype_depth_pattern' : {
		'title' : 'Fifo memory type: {}. Depth = {}',
		'savefig' : '{}_{}_{}_patterns.png',
		'valid_modes': ['32bit', 'nonsym'],
		'first_param' : 'FifoMemoryType',
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
		'valid_modes': ['32bit', 'nonsym'],
		'first_param' : 'FifoDepth',
		'second_param' : 'DataPattern',
		'third_param' : 'FifoMemoryType',
		'legend' : {
			'blockram' : 'ro',
			'distributedram' : 'g*',
			'shiftregister' : 'b+'
		}
	},
	'pattern_memtype_depth' : {
		'title' : 'Pattern type: {}. Fifo memory type: {}',
		'savefig' : '{}_{}_{}_depths.png',
		'valid_modes': ['32bit', 'nonsym'],
		'first_param' : 'DataPattern',
		'second_param' : 'FifoMemoryType',
		'third_param' : 'FifoDepth',
		'legend' : {
			16 : 'ro',
			32 : 'y^',
			64 : 'g*',
			256 : 'b+',
			1024 : 'mv'
		}
	},
	'duplex_memtype_blocksize_pattern': {
		'title': 'Fifo memory type: {}. Block size = {}',
		'savefig': '{}_{}_{}_patterns.png',
		'valid_modes': ['duplex'],
		'first_param': 'FifoMemoryType',
		'second_param': 'BlockSize',
		'third_param': 'DataPattern',
		'legend': {
			'counter_8bit': 'ro',
			'counter_32bit': 'g*',
			'walking_1': 'b+'
		}
	},
	'duplex_blocksize_pattern_memtype': {
		'title' : 'Block size: {}. Pattern type = {}',
		'savefig' : '{}_{}_{}_memory_types.png',
		'valid_modes': ['duplex'],
		'first_param' : 'BlockSize',
		'second_param' : 'DataPattern',
		'third_param' : 'FifoMemoryType',
		'legend' : {
			'blockram' : 'ro',
			'distributedram' : 'g*',
			'shiftregister' : 'b+'
		}
	},
	'duplex_pattern_memtype_blocksize': {
		'title' : 'Pattern type: {}. Fifo memory type: {}',
		'savefig' : '{}_{}_{}_blocksizes.png',
		'valid_modes': ['duplex'],
		'first_param' : 'DataPattern',
		'second_param' : 'FifoMemoryType',
		'third_param' : 'BlockSize',
		'legend' : {
			16 : 'ro',
			64 : 'g*',
			256 : 'b+',
			1024 : 'mv'
		}
	}
}
