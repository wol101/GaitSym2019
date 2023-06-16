#!/bin/sh
# install cppclean with
# sudo pip install --upgrade cppclean
# TCP.cpp UDP.cpp cause cppclean to crash

find . -name '*.cpp' -not -name 'TCP.cpp' -not -name 'UDP.cpp' -exec cppclean --verbose --include-path ../ode-0.15/include --include-path ../pystring {} \; > cppclean.log
