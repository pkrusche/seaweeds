replacement_CC = 'icl'
replacement_CXX = 'icl'
replacement_LINK = 'xilink'
replacement_LIB = 'xilib /LTCG'
win32_boostdir = 'c:\\Boost\\include';
win32_lokidir = 'c:\\Boost\\loki';
win32_tbbdir = 'C:\\Users\\peter\\Documents\\Code\\tbb30_018oss';
win32_bsponmpidir = 'C:\\Users\\peter\\workspace\\bsponmpi'
BRCCFLAGS = " -p cpu ";
use_ati_brook = 0;
BROOKROOT='C:\\Program Files\\ATI\\ATI Brook+ 1.4.0_beta'
configure = 1
sequential=1
debuginfo = 1
additional_cflags = "/GL /O3 /Qipo /fast"
# additional_cflags = "/O2 /GL /EHsc"
additional_lflags = "/LTCG /LARGEADDRESSAWARE:NO libboost_regex-vc90-mt.lib libboost_program_options-vc90-mt.lib"