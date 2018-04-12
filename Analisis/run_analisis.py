from cfg import * 
import csv
import math
import matplotlib.pyplot as plt
import statistics as stat


class ResultsParser(object):
	"""Transform data from csv file to list of dicts.

	Attributes:
		int_values (list): specifes which values need to be integers.
		float_values (list): specifes which values need to be floats.
		refactored_heads (list): list of headers for refactoring the original data.
		stat_iterations (int): how many iterations were performed for better statistics.
		results_list (list): list of results from csv file.
		headers_list (list): list of headers from csv file.
		list_of_results_dicts (list): list of dicts of original data.
	"""
	def __init__(self, csv_file, delimiter, int_values, float_values, refactored_heads, stat_iterations):
		"""Args:
			csv_file (string): name of csv file that contains data.
			delimiter (char): the character used for separating values in csv file.
			int_values (list): specifes which values need to be integers.
			float_values (list): specifes which values need to be floats.
			refactored_heads (list): list of headers for refactoring the original data.
			stat_iterations (int): how many iterations were performed for better statistics.
		"""
		self.int_values = int_values
		self.float_values = float_values
		self.refactored_heads = refactored_heads
		self.stat_iterations = stat_iterations
		self.results_list, self.headers_list = ResultsParser.results_and_headers_lists_from_file(csv_file, delimiter)
		self.list_of_results_dicts = self.__generate_results_list_of_dicts()

	@staticmethod
	def results_and_headers_lists_from_file(csv_file, delimiter):
		"""Returns list with results and headers list as tuple from csv file"""
		with open(csv_file, mode='r') as results_file:
			results_reader = csv.reader(results_file, delimiter=delimiter)
			headers_list = next(results_reader)
			results_list = list(results_reader)
		return results_list, headers_list

	def __hash_data(self, results_list):
		"""Transform desired numeric data from string to int / float"""
		for row in results_list:
			for int_value in self.int_values:
				row[self.headers_list.index(int_value)] = int(row[self.headers_list.index(int_value)])
			for float_value in self.float_values:
				row[self.headers_list.index(float_value)] = float(row[self.headers_list.index(float_value)])
		return results_list

	def __generate_results_list_of_dicts(self):
		hashed_results_list = self.__hash_data(self.results_list)
		list_of_results_dicts = []
		for row in hashed_results_list:
			results_dicts = {head : row[self.headers_list.index(head)] for head in self.headers_list}
			list_of_results_dicts.append(results_dicts)
		return list_of_results_dicts

	def __transform_row_to_dict(self, row,  params):
		row_dict = {}
		for head in self.refactored_heads:
			if head in self.headers_list:
				row_dict[head] = row[head]
			else:
				row_dict[head] = params.get_refactored_param(head)
		return row_dict

	def get_refactored_list_of_results_dicts(self, in_megabytes=True):
		"""Returns list of dicts of refactored data.
		Args:
			in_megabytes (bool): does speed values need to be perfomed in megabytes
								 instead of bytes? (True by default)
		"""
		refactored_list_of_results_dicts = []
		results_fpga_array = []
		results_pc_array = []
		for row in self.list_of_results_dicts:
			results_fpga_array.append(row['SpeedFPGA [B/s]'])
			results_pc_array.append(row['SpeedPC [B/s]'])
			if row['StatisticalIter'] == self.stat_iterations:
				params = CounterParams(results_fpga_array, results_pc_array, in_megabytes)
				row_dict = self.__transform_row_to_dict(row, params)
				refactored_list_of_results_dicts.append(row_dict)
				results_fpga_array = []
				results_pc_array = []
		return refactored_list_of_results_dicts

	def check_errors(self):
		errors_occurance = 0
		for results_dict in self.list_of_results_dicts:
			errors = results_dict['Errors']
			if errors != 0:
				print(errors, "errors occured in row:\n", results_dict)
				errors_occurance += 1
		if errors_occurance == 0:
			print("No errors detected")

class CounterParams(object):
	def __init__(self, speed_fpga_list, speed_pc_list, in_megabytes):
		if in_megabytes is True:
			self.divider = 1000000
		else:
			self.divider = 1
		self.av_speed_fpga = stat.mean(speed_fpga_list)
		self.av_speed_pc = stat.mean(speed_pc_list)
		self.stdev_speed_fpga = stat.stdev(speed_fpga_list)
		self.stdev_speed_pc = stat.stdev(speed_pc_list)
		self.av_speed = self.__av_speed()
		self.stdev_speed = self.__stdev_speed()

	def __av_speed(self):
		return (self.av_speed_fpga + self.av_speed_pc) / 2

	def __stdev_speed(self):
		fpga_arg = math.pow(self.stdev_speed_fpga/2, 2)
		pc_arg = math.pow(self.stdev_speed_pc/2, 2)
		sum_of_args = fpga_arg + pc_arg
		return math.sqrt(sum_of_args)

	def get_refactored_param(self, head):
		if head == 'SpeedPC':
			return self.av_speed_pc / self.divider
		elif head == 'SpeedFPGA':
			return self.av_speed_fpga / self.divider
		elif head == 'u(PC)':
			return self.stdev_speed_pc / self.divider
		elif head == 'u(FPGA)':
			return self.stdev_speed_fpga / self.divider
		elif head == 'Average':
			return self.av_speed / self.divider
		elif head == 'u(av)':
			return self.stdev_speed / self.divider


class ResultsHandler(object):
	def __init__(self, list_of_results_dicts, plot_metadata, target_speed, basic_properties):
		self.list_of_results_dicts = list_of_results_dicts
		self.metadata = plot_metadata
		self.target_speed = target_speed
		self.basic_properties = basic_properties

	def __iterate_using_params(self, first_param_label, second_param_label, third_param_label, valid_modes):
		list_of_param_dicts = []
		for mode in self.basic_properties['Mode']:
			for direction in self.basic_properties['Direction']:
				for first_param in self.basic_properties[first_param_label]:
					for second_param in self.basic_properties[second_param_label]:
						results_dict = {}
						is_results_list_non_empty = False
						for third_param in self.basic_properties[third_param_label]:
							x_param = []
							y_param = []
							yerr_param = []
							for row in self.list_of_results_dicts:
								if ((mode == row['Mode']) and
									(mode in valid_modes) and
									(direction == row['Direction']) and
									(first_param == row[first_param_label]) and
									(second_param == row[second_param_label]) and
									(third_param == row[third_param_label])):
									x_param.append(row['PatternSize'])
									y_param.append(row['Average'])
									yerr_param.append(row['u(av)'])
							results_dict[third_param] = {
								'x': x_param,
								'y': y_param,
								'yerr': yerr_param
							}
							if y_param:
								is_results_list_non_empty = True
							else:
								is_results_list_non_empty = False
						param_dict = {
							'mode': mode,
							'direction': direction,
							'first_param': first_param,
							'second_param': second_param,
							'third_param': results_dict
						}
						if is_results_list_non_empty:
							list_of_param_dicts.append(param_dict)
		return list_of_param_dicts

	def list_of_results_with_parameters(self, plotting_option, option=None):
		first_param_label = plotting_option['first_param']
		second_param_label = plotting_option['second_param']
		third_param_label = plotting_option['third_param']
		valid_modes = plotting_option['valid_modes']
		list_of_param_dicts = self.__iterate_using_params(first_param_label,
														  second_param_label,
														  third_param_label,
														  valid_modes)
		return list_of_param_dicts

	def save_to_figs(self, plotting_option, plot_index, separate_third_parameters=False):
		list_of_param_dicts = self.list_of_results_with_parameters(plotting_option)
		figure = Figure(self.metadata, self.target_speed, plotting_option['title'], plotting_option['savefig'])
		for i, results_dict in enumerate(list_of_param_dicts):
			for j, result in enumerate(results_dict['third_param']):
				x = results_dict['third_param'][result]['x']
				y = results_dict['third_param'][result]['y']
				yerr = results_dict['third_param'][result]['yerr']
				symbol = plotting_option['legend'][result] # TODO: refactor plot option!
				label = result
				figure.plot_fig_with_errorbars(x, y, yerr, symbol, label)
				if separate_third_parameters:
					figure.set_title(results_dict['first_param'], results_dict['second_param'])
					figure.save_fig(str(plot_index) + '_' + str(i) + '_' + str(j), results_dict['mode'], results_dict['direction'])
			if not separate_third_parameters:
				figure.set_title(results_dict['first_param'], results_dict['second_param'])
				figure.save_fig(str(plot_index) + '_' + str(i), results_dict['mode'], results_dict['direction'])


class Figure(object):
	def __init__(self, metadata, target_ylabel, fig_title, fig_name):
		self.__fig, self.__ax = plt.subplots()
		self.__metadata = metadata
		self.__target = target_ylabel
		self.__fig_title = fig_title
		self.__fig_name = fig_name

	def __set_metadata(self):
		self.__ax.set_xlabel(self.__metadata['xlabel'])
		self.__ax.set_xscale(self.__metadata['xscale'])
		self.__ax.set_ylabel(self.__target_ylabel())
		self.__ax.set_yticks(self.__metadata['yticks'])
		self.__ax.grid(self.__metadata['grid'])

	def __target_ylabel(self):
		if self.__target == 'PC':
			return self.__metadata['ylabel_PC']
		elif self.__target == 'FPGA':
			return self.__metadata['ylabel_FPGA']
		elif self.__target == 'AV':
			return self.__metadata['ylabel_AV']
		else:
			raise ValueError(self.__metadata['error'])

	def plot_fig_with_errorbars(self, x, y, yerr, symbol, label):
		self.__ax.errorbar(x, y, yerr=yerr, fmt=symbol, label=label)

	def set_title(self, *args):
		if args:
			self.__ax.set_title(self.__fig_title.format(*args))
		else:
			self.__ax.title(self.__fig_title)

	def save_fig(self, *args):
		self.__set_metadata()
		self.__ax.legend(loc='upper left')
		if args:
			self.__fig.savefig(self.__fig_name.format(*args))
		else:
			self.__fig.savefig(self.__fig_name)
		self.__ax.clear()


if __name__ == "__main__":
	results = ResultsParser(CSV_FILE, SEPARATOR, INT_VALUES, FLOAT_VALUES, REFACTORED_HEADS, STATISTICAL_ITERATIONS)
	if CHECK_FOR_ERRORS:
		results.check_errors()

	parsed_list_of_results_dicts = results.get_refactored_list_of_results_dicts()
	rh = ResultsHandler(parsed_list_of_results_dicts, FIGURE_METADATA, TARGET_SPEED, BASIC_PROPERTIES)
	for i, plot_option in enumerate(PLOTTING_OPTIONS):
		rh.save_to_figs(PLOTTING_OPTIONS[plot_option], i, PARAMETERS_SEPARATED)
