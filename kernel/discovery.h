// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <bytearray.h>

#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <map>

#include "lcstypes.h"

class DiscoveryService {
public:
    struct NodeInfo {
        time_t              deviceTime      = 0;
        std::string         hardwareVersion;
        std::string         softwareVersion;
        std::string         ip;
        DeviceRole          role            = roleUndefined;
        int                 id              = -1;
        int                 httpPort        = 0;
        unsigned long long  updated         = 0;

        bool        isChanged(const NodeInfo & other) const {
            return ((role != other.role) || (ip != other.ip));
        }

        bool        isNull() const { return id == -1; };
    };

    DiscoveryService(int nPort, int nTimeout = 1000, int deviceId = -1);
    ~DiscoveryService();
  
    void    sendDeviceInfo(unsigned long long time, const std::string & hwVersion, const std::string & swVersion, int mode, int httpPort);
    void    onNodeDiscovered(std::function<void(const NodeInfo & info)> callback = nullptr) {
        onNodeDiscoveredCallback = callback;
    }

    bool    start();
    void    quit();
    int     deviceId() const;

    std::vector<NodeInfo>   nodes() const;
    NodeInfo                lastDiscoveredMaster() const;

protected:

    void    processReceivedData(const char * pData, const char * addrInfo);
    void    processEvents();
    void    threadFunc();
    void    shutdownSocket();
    void    sendData(const std::string & data);
    
protected:

    std::function<void(const NodeInfo & info)>     onNodeDiscoveredCallback = nullptr;

    std::queue<std::string>                        m_toSend;
    std::atomic_bool                               m_bExit = false;
    std::thread                                    m_executionThread;

    std::map<int, NodeInfo>                        m_nodes;
    mutable std::mutex                             m_socketLock;
    mutable std::recursive_mutex                   m_containerLock;

    int     m_id            = 0;
    int     m_socket        = -1;
    int     m_timeout;
    int     m_port;
};
