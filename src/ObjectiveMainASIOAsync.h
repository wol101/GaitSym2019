/*
 *  ObjectiveMainASIOAsync.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 24/12/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef OBJECTIVEMAINASIOASYNC_H
#define OBJECTIVEMAINASIOASYNC_H


#include "XMLConverter.h"
#include "ArgParse.h"

#include "asio.hpp"

#include <string>
#include <vector>
#include <random>
#include <deque>
#include <map>
#include <memory>
#include <iostream>
#include <system_error>
#include <mutex>

class Simulation;

//----------------------------------------------------------------------
//
// This class manages socket timeouts by running the io_context using the timed
// io_context::run_for() member function. Each asynchronous operation is given
// a timeout within which it must complete. The socket operations themselves
// use member functions as completion handlers. For a given socket operation, the client
// object runs the io_context to block thread execution until the operation
// completes or the timeout is reached. If the io_context::run_for() function
// times out, the socket is closed and the outstanding asynchronous operation
// is cancelled.
//
class AsioClient
{
public:
    void connect(const std::string& host, const std::string& service, std::chrono::steady_clock::duration timeout)
    {
        // Resolve the host name and service to a list of endpoints.
        asio::ip::tcp::tcp::resolver::results_type endpoints = asio::ip::tcp::tcp::resolver(m_ioContext).resolve(host, service);

//        for (auto endpoint = endpoints.begin(); endpoint != endpoints.end(); endpoint++)
//        {
//            std::cerr << std::distance(endpoints.begin(), endpoint) << " endpoint->endpoint().address() " << endpoint->endpoint().address() << "\n";
//            std::cerr << std::distance(endpoints.begin(), endpoint) << " endpoint->endpoint().port() " << endpoint->endpoint().port() << "\n";
//        }

        // Start the asynchronous operation itself.
        // Uses std::bind to allow a member function to act as a callback.
        m_resultError = {};
        asio::async_connect(m_socket, endpoints, std::bind(&AsioClient::connectHandler, this, std::placeholders::_1, std::placeholders::_2));

        // Run the operation until it completes, or until the timeout.
        run(timeout);

        // Determine whether a connection was successfully established.
        if (m_resultError)
            throw std::system_error(m_resultError);

        // now set some options
        m_socket.set_option(asio::ip::tcp::tcp::no_delay(true));
        m_socket.set_option(asio::socket_base::linger(false, 0));
    }

    std::string readLine(std::chrono::steady_clock::duration timeout, char delimiter = '\n')
    {
        // Start the asynchronous operation itself.
        // Uses std::bind to allow a member function to act as a callback.
        m_resultError = {};
        m_resultN = 0;
        asio::async_read_until(m_socket, asio::dynamic_buffer(m_inputBuffer), delimiter, std::bind(&AsioClient::readHandler, this, std::placeholders::_1, std::placeholders::_2));

        // Run the operation until it completes, or until the timeout.
        run(timeout);

        // Determine whether the read completed successfully.
        if (m_resultError)
            throw std::system_error(m_resultError);

        std::string line(m_inputBuffer.substr(0, m_resultN - 1));
        m_inputBuffer.erase(0, m_resultN);
        return line;
    }

    void writeLine(const std::string& line, std::chrono::steady_clock::duration timeout, char delimiter = '\n')
    {
        std::string data = line + delimiter;

        // Start the asynchronous operation itself.
        // Uses std::bind to allow a member function to act as a callback.
        m_resultError = {};
        m_resultN = 0;
        asio::async_write(m_socket, asio::buffer(data), std::bind(&AsioClient::writeHandler, this, std::placeholders::_1, std::placeholders::_2));

        // Run the operation until it completes, or until the timeout.
        run(timeout);

        // Determine whether the read completed successfully.
        if (m_resultError)
            throw std::system_error(m_resultError);
    }

    void readBuffer(char *buffer, size_t size, std::chrono::steady_clock::duration timeout)
    {
        // Start the asynchronous operation itself.
        // Uses std::bind to allow a member function to act as a callback.
        m_resultError = {};
        m_resultN = 0;
        asio::async_read(m_socket, asio::buffer(buffer, size), std::bind(&AsioClient::readHandler, this, std::placeholders::_1, std::placeholders::_2));

        // Run the operation until it completes, or until the timeout.
        run(timeout);

        // Determine whether the read completed successfully.
        if (m_resultError)
            throw std::system_error(m_resultError);
    }

    void writeBuffer(const char *buffer, size_t size, std::chrono::steady_clock::duration timeout)
    {
        // Start the asynchronous operation itself.
        // Uses std::bind to allow a member function to act as a callback.
        m_resultError = {};
        m_resultN = 0;
        asio::async_write(m_socket, asio::buffer(buffer, size), std::bind(&AsioClient::writeHandler, this, std::placeholders::_1, std::placeholders::_2));

        // Run the operation until it completes, or until the timeout.
        run(timeout);

        // Determine whether the read completed successfully.
        if (m_resultError)
            throw std::system_error(m_resultError);
    }

    asio::error_code &resultError() { return m_resultError; }
    std::size_t resultN() { return m_resultN; }
    asio::ip::tcp::tcp::socket &socket() { return m_socket; }

private:
    void run(std::chrono::steady_clock::duration timeout)
    {
        // Restart the io_context, as it may have been left in the "stopped" state
        // by a previous operation.
        m_ioContext.restart();

        // Block until the asynchronous operation has completed, or timed out. If
        // the pending asynchronous operation is a composed operation, the deadline
        // applies to the entire operation, rather than individual operations on
        // the socket.
        m_ioContext.run_for(timeout);

        // If the asynchronous operation completed successfully then the io_context
        // would have been stopped due to running out of work. If it was not
        // stopped, then the io_context::run_for call must have timed out.
        if (!m_ioContext.stopped())
        {
            // Close the socket to cancel the outstanding asynchronous operation.
            m_socket.close();

            // Run the io_context again until the operation completes.
            m_ioContext.run();
        }
    }

    void readHandler(const asio::error_code& resultError, std::size_t resultN)
    {
        m_resultError = resultError;
        m_resultN = resultN;
    }

    void writeHandler(const asio::error_code& resultError, std::size_t resultN)
    {
        m_resultError = resultError;
        m_resultN = resultN;
    }

    void connectHandler(const asio::error_code& resultError, const asio::ip::tcp::tcp::endpoint& /* resultEndpoint */)
    {
        m_resultError = resultError;
    }

    asio::io_context m_ioContext;
    asio::ip::tcp::tcp::socket m_socket{m_ioContext};
    std::string m_inputBuffer;

    asio::error_code m_resultError = {};
    std::size_t m_resultN = 0;
};

class ObjectiveMainASIOAsync
{
public:
    ObjectiveMainASIOAsync(int argc, const char **argv);

    int Run();

    static std::string encode(const std::string &input);
    static std::string decode(const std::string &input);
    static bool hashEqual(const uint32_t *hash1, const uint32_t *hash2, size_t hashSize);

private:

    struct DataMessage
    {
        char text[16];
        uint64_t evolveIdentifier;
        uint32_t senderIP;
        uint32_t senderPort;
        uint32_t runID;
        uint32_t genomeLength;
        uint32_t xmlLength;
        uint32_t md5[4];
        union
        {
            double genome[1];
            char xml[1];
        } payload;
    };

    struct RequestMessage
    {
        char text[16];
        uint64_t evolveIdentifier;
        uint32_t senderIP;
        uint32_t senderPort;
        uint32_t runID;
        double score;
    };

    int ReadGenome(std::string host, uint16_t port, std::string *rawMessage);
    int ReadXML(std::string host, uint16_t port, std::string *rawMessage);
    int WriteOutput(std::string host, uint16_t port, uint64_t evolveIdentifier, uint32_t runID, double score);
    void DoSimulation(const char *xmlPtr, size_t xmlLen, double *score, double *computeTime);

    std::vector<std::string> m_outputList;

    double m_runTimeLimit = 0;
    double m_outputModelStateAtTime = -1;
    double m_outputModelStateAtCycle = -1;
    double m_outputModelStateAtWarehouseDistance = -1;
    double m_simulationTimeLimit = -1;
    double m_warehouseFailDistanceAbort = 0;

    std::string m_configFilename;
    std::string m_outputWarehouseFilename;
    std::string m_outputModelStateFilename;
    std::string m_inputWarehouseFilename;
    std::string m_scoreFilename;

    XMLConverter m_XMLConverter;
    ArgParse m_argparse;

    std::string m_host;
    uint16_t m_port = 0;
    int m_sleepTime = 0;

    bool m_scoreToSend = false;
    double m_lastScore = 0;
    uint32_t m_lastRunID = std::numeric_limits<uint32_t>::max() - 2;
    uint64_t m_lastEvolveIdentifier = 0;
    std::string m_lastGenomeDataMessageRaw;
    bool m_lastGenomeValid = false;
    int m_statusDoSimulation = 0;
    std::vector<uint32_t> m_hash = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};

    AsioClient m_asioClient;
    std::chrono::steady_clock::duration m_timeout;

    std::mt19937_64 m_gen;
    std::uniform_real_distribution<double> m_distrib;

    bool m_debug = false;
};



#endif // OBJECTIVEMAINASIOASYNC_H
