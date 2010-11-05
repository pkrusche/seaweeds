toolset='icc'
use_ati_brook=0;
BROOKROOT = '/usr/local/atibrook'
additional_cflags = '-I../bsp_cpp/include -I../bsponmpi/include'
additional_lflags = '-L../bsp_cpp/lib -L../bsponmpi/lib -lboost_regex-mt -lboost_program_options-mt'
