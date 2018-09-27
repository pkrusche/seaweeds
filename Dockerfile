FROM centos:6.10

RUN yum -y groupinstall 'Development Tools'
RUN yum -y install centos-release-scl && yum -y install devtoolset-4
