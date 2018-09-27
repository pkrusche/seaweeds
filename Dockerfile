FROM centos:6.10

RUN yum -y update && yum -y groupinstall 'Development Tools'
RUN yum -y install centos-release-scl && yum -y install devtoolset-7
RUN yum -y install scons curl wget

RUN mkdir -p /opt
WORKDIR /opt

# make boost
RUN wget https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.gz && \
    tar xf boost_1_68_0.tar.gz && rm -f boost_1_68_0.tar.gz && \
    source scl_source enable devtoolset-7 && \
    cd boost_1_68_0 && ./bootstrap.sh && \
    ./b2 --prefix=/opt/boost_1_68_0_install link=static --without-python -j4 && \
    ./b2 --prefix=/opt/boost_1_68_0_install link=static --without-python install -j4 && \
    rm -rf /opt/boost_1_68_0

# make tbb
RUN wget https://github.com/01org/tbb/archive/2019.tar.gz && tar xf 2019.tar.gz && \
    source scl_source enable devtoolset-7 && \
    cd tbb-2019 && make && ln -s /opt/tbb-2019 /opt/tbb && \
    mkdir lib && cp build/linux_intel64_gcc*_release/* lib && \
    rm -f 2019.tar.gz

# make bsponmpi
RUN wget https://github.com/pkrusche/bsponmpi/archive/master.zip && unzip master.zip && mv bsponmpi-master bsponmpi && rm master.zip
WORKDIR /opt/bsponmpi
RUN echo "sequential = 1" > opts.py && \
    echo "tbbdir = \"/opt/tbb\"" >> opts.py && \
    echo "additional_cflags=\"-I/opt/boost_1_68_0_install/include -I/opt/tbb/include\"" >> opts.py && \
    echo "additional_lflags=\"-L/opt/boost_1_68_0_install/lib -L/opt/tbb/lib \"" >> opts.py && \
    source scl_source enable devtoolset-7 && \
    scons mode=release

# install yasm
RUN yum -y --enablerepo=extras install epel-release && yum -y install yasm

# install vectorclass and asmlib (optional)
# These libraries are distributed under the Gnu general public license by Agner Fog, https://www.agner.org
# Please see https://www.agner.org/optimize/ before using these! They make this code faster on newer
# processor architectures (if you are able to use them).
# WORKDIR /opt
# RUN wget https://www.agner.org/optimize/asmlib.zip && mkdir -p /opt/asmlib && cd /opt/asmlib && unzip /opt/asmlib.zip
# WORKDIR /opt
# RUN wget https://www.agner.org/optimize/vectorclass.zip && mkdir -p /opt/vectorclass && cd /opt/vectorclass && unzip /opt/vectorclass.zip

ADD . /opt/seaweeds-source
WORKDIR /opt/seaweeds-source
ENV LD_LIBRARY_PATH /opt/tbb/lib
RUN echo "sequential = 1" > opts.py && \
    echo "tbbdir = \"/opt/tbb\"" >> opts.py && \
    echo "additional_cflags=\"-I/opt/bsponmpi/include -I/opt/boost_1_68_0_install/include -I/opt/tbb/include\"" >> opts.py && \
    echo "additional_lflags=\"-L/opt/bsponmpi/lib -L/opt/boost_1_68_0_install/lib -L/opt/tbb/lib \"" >> opts.py && \
    echo "asmlibdir = '/opt/asmlib'" >> opts.py && \
    echo "veclibdir = '/opt/vectorclass'" >> opts.py && \
    rm -f opts_Linux_x86_64.py && \
    source scl_source enable devtoolset-7 && \
    scons mode=release configure=1 -j4

# run tests
RUN bin/unit_tests/seaweedtests_posix_default_release

