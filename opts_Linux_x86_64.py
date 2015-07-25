use_yasm=1
additional_cflags = '-I../bsponmpi/include -O5 -msse2 -msse3 -finline -funroll-all-loops -DDSFMT_MEXP=19937'
additional_lflags = '-L../bsponmpi/lib'
asmlibdir = '/home/peterkrusche/workspace/asmlib'
veclibdir = '/home/peterkrusche/workspace/vectorclass'