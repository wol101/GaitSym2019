#!/bin/sh
scripts/sync_shared_files.py src/UDP.cpp ../AsynchronousGA2019/UDP.cpp src/UDP.h ../AsynchronousGA2019/UDP.h
scripts/sync_shared_files.py src/ThreadedUDP.cpp ../AsynchronousGA2019/ThreadedUDP.cpp src/ThreadedUDP.h ../AsynchronousGA2019/ThreadedUDP.h
scripts/sync_shared_files.py src/DataFile.cpp ../AsynchronousGA2019/DataFile.cpp src/DataFile.h ../AsynchronousGA2019/DataFile.h

