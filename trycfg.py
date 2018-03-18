bitfiles_path = "../HDL/bitfiles/"
bitfiles_pat = 3

output:{
    logfile_name = "logs.txt",
    show_logs_stdout = "yes", # "yes"
    resultfile_name = "test_result.csv",
    result_sep = ";",
    results_path = "./results/",
    logs_path = "./results/"
# logs_path = ".\jfRjf_435 fgh5\ tgth6/njr /";
}

params:{
    width = [ "nonsym" ], ## "both";  "32bit" / "nonsym" / "both"
    direction = [ "read" ], ## "read" / "write" / "both" //TODO: Read/write
    memory = [ "blockram", "distributedram", "shiftregister" ], ## "blockram"
    depth = [ 16, 64, 256, 1024 ],
    pattern_sizes = [ ],
    patterns = [ "counter_8bit", "counter_32bit", "walking_1" ],
    statistic_iter = 5,
    iterations = 1
}