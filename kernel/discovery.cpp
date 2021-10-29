// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "discovery.h"

#include "spdlog/spdlog.h"
#include "json.hpp"

#ifdef WIN32 
    #include <processthreadsapi.h>
    #include <winsock2.h>
    #include <windows.h>
    #include <ws2tcpip.h>
#else
    #include <ifaddrs.h>
    #include <sys/ioctl.h>
    #include <net/if.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

#include "containerutils.h"
#include "timeutils.h"

#define RECEIVE_BUFFER_LENGTH   1024

DiscoveryService::DiscoveryService(int nPort, int nTimeout, int deviceId) {
    m_port      = nPort;
    m_timeout   = nTimeout;
    
    m_id = deviceId > 0 ? deviceId : std::rand();

#ifdef WIN32
    WSADATA wsaData = { 0 };
    WSAStartup(MAKEWORD(2, 2), &wsaData);    
#endif
};

DiscoveryService::~DiscoveryService() {
    quit();
    spdlog::info("DiscoveryService destroyed");

#ifdef WIN32
    WSACleanup();
#endif
}

void    DiscoveryService::sendDeviceInfo(unsigned long long time, const std::string & hwVersion, const std::string & swVersion, int mode, int httpPort) {    
    spdlog::debug("Discovery service, broadcasting device information");
    nlohmann::json  discoveryPacket = { { "id", m_id }, { "dt", std::to_string(time) }, { "hw", hwVersion }, { "sw", swVersion }, 
    { "mode", mode }, {"http", httpPort } };
    std::string packetString = discoveryPacket.dump();

    std::lock_guard locker(m_socketLock);
    m_toSend.push(packetString);
}

void DiscoveryService::sendData(const std::string & data) {
#ifdef WIN32
    struct sockaddr_in addressInfo;    
    
    addressInfo.sin_family = AF_INET;
    addressInfo.sin_port = htons(m_port);
    addressInfo.sin_addr.s_addr  = INADDR_BROADCAST;
    
    if (sendto(m_socket, data.c_str(), data.size(), 0, (sockaddr *)&addressInfo, sizeof(addressInfo)) < 0) {
        spdlog::error("Discovery service, broadcasting error: {} - {}", errno, strerror(errno));
    }
#else
   struct ifaddrs * ifap;
   if (getifaddrs(&ifap) == 0) {
      struct ifaddrs * p = ifap;
      while(p)
      {
          if (p->ifa_addr->sa_family != AF_INET) {
              p = p->ifa_next;
              continue;
          }

          if ((p->ifa_flags & IFF_UP) == 0 ||
              (p->ifa_flags & IFF_LOOPBACK) ||
              (p->ifa_flags & (IFF_BROADCAST | IFF_POINTOPOINT) == 0)) {
              p = p->ifa_next;
              continue;
          }

         unsigned int broadcastAddr = ntohl(((struct sockaddr_in *)p->ifa_broadaddr)->sin_addr.s_addr);
         if (broadcastAddr > 0)
         {
		struct sockaddr_in addressInfo;

        	addressInfo.sin_family = AF_INET;
        	addressInfo.sin_port = htons(m_port);
        	addressInfo.sin_addr.s_addr = htonl(broadcastAddr);

        	if (sendto(m_socket, data.c_str(), data.size(), 0, (sockaddr *)&addressInfo, sizeof(addressInfo)) < 0) {
            		spdlog::error("Discovery service, broadcasting error: {} - {}", errno, strerror(errno));
        	}

         }
	 
         p = p->ifa_next;
      }
      freeifaddrs(ifap);
   }


#endif
}

int     DiscoveryService::deviceId() const {
    std::lock_guard locker(m_socketLock);
    return m_id;
}

void    DiscoveryService::processEvents() {
    if (m_socket < 0) {
        return;
    }
 
    {
        std::string     sendBuffer;
        m_socketLock.lock();
        while (m_toSend.size()) {
            
            sendBuffer = m_toSend.front();
            m_toSend.pop();
            sendData(sendBuffer);

        };
        m_socketLock.unlock();
    }

    char buffer[RECEIVE_BUFFER_LENGTH];
    struct sockaddr_in addressInfo = { 0 };
    int addressInfoLength = sizeof(struct sockaddr_in);
#ifdef WIN32
    int nBytesReceived = recvfrom(m_socket, buffer, RECEIVE_BUFFER_LENGTH-1, 0, (sockaddr *)&addressInfo, &addressInfoLength);
#else
    int nBytesReceived = recvfrom(m_socket, buffer, RECEIVE_BUFFER_LENGTH - 1, 0, (sockaddr *)&addressInfo, (socklen_t*)&addressInfoLength);
#endif
    if (nBytesReceived > 0) {        
        buffer[nBytesReceived] = 0;
        spdlog::debug("Discovery service, node information received: {}", buffer);
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addressInfo.sin_addr), str, INET_ADDRSTRLEN);

        processReceivedData(buffer, str);
    }
}

std::vector<DiscoveryService::NodeInfo>   DiscoveryService::nodes() const {
    std::lock_guard locker(m_containerLock);
    return MapUtils::values(m_nodes);
}

DiscoveryService::NodeInfo               DiscoveryService::lastDiscoveredMaster() const {

    DiscoveryService::NodeInfo      result;

    std::lock_guard locker(m_containerLock);
    for (const std::pair<int, NodeInfo> & item : m_nodes) {
        if ((item.second.role == roleStandalone) && ((item.second.updated > result.updated) ||(result.isNull())))
            result = item.second;
    }

    return result;
}

void    DiscoveryService::processReceivedData(const char * pData, const char * addrInfo) {
    try {
        nlohmann::json  json = nlohmann::json::parse(pData);
        unsigned int id = 0;
        json["id"].get_to(id);
        if (id == m_id) {
            // own packet received
            return;
        }

        std::string     timeString;
        std::string     hwVersion;
        std::string     swVersion;
        int             role = 0;
        int             httpPort = 0;

        json["dt"].get_to(timeString);
        json["hw"].get_to(hwVersion);
        json["sw"].get_to(swVersion);
        json["mode"].get_to(role);

        if (json.contains("http")) {
            json["http"].get_to(httpPort);
        }

        time_t  deviceTime = atol(timeString.c_str());
        
        {
            bool bNotify = false;
            m_containerLock.lock();
                       
            NodeInfo info;
            info.deviceTime = deviceTime;
            info.hardwareVersion = hwVersion;
            info.softwareVersion = swVersion;
            info.id              = id;
            info.role            = (DeviceRole)role;
            info.updated         = LocalClock::currentSystemMsSinceEpoch();
            info.ip              = std::string(addrInfo);
            info.httpPort        = httpPort;
     
            if (!m_nodes.count(id)) {
                bNotify = true;
            } else {
                const NodeInfo & previousNode = m_nodes[id];
                if (previousNode.isChanged(info)) {
                    bNotify = true;
                }
            }
            
            m_nodes[id] = info;
            m_containerLock.unlock();

            if (bNotify) {
                spdlog::debug("Device id 0x{:X} detected, software version: {}, hardware version: {}", id, swVersion, hwVersion);
                if (onNodeDiscoveredCallback) {
                    onNodeDiscoveredCallback(info);
                }
            } else {
                spdlog::info("Device id 0x{:X} information updated", id);
            }
        }
    }

    catch (nlohmann::json::parse_error & e) {
        spdlog::error("Unable to parse JSON data, {}", e.what());
    }
    catch (nlohmann::json::out_of_range & e) {
        spdlog::error("Unable to fetch JSON data, {}", e.what());
    }
    catch (nlohmann::json::type_error & e) {
        spdlog::error("Unable to fetch JSON data, {}", e.what());
    }
}

void    DiscoveryService::quit() {
    m_bExit = true;
    if (m_executionThread.joinable()) {
        m_executionThread.join();
    }   
}

void            DiscoveryService::threadFunc() {
    while (!m_bExit) {
        processEvents();
    }

    if (m_socket > 0) {
 	shutdownSocket();
        m_socket = -1;
    }
}

void DiscoveryService::shutdownSocket() {
#ifdef WIN32
	closesocket(m_socket);
#else
	close(m_socket);
#endif
}

bool DiscoveryService::start() {

    if ((m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        spdlog::error("Unable to initialized discovery socket, socket(): {} - {}", errno, strerror(errno));
        return false;
    }

    int broadcast = 1;
    
    if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast)) < 0) {
        spdlog::error("Unable to initialized broadcast mode on discovery socket: {} - {}", errno, strerror(errno));
	    shutdownSocket();
        m_socket = -1;
        return false;
    }
    
#ifdef WIN32
    DWORD tv = m_timeout;
#else
    struct timeval tv;
    tv.tv_sec = m_timeout / 1000;
    tv.tv_usec = (m_timeout % 1000) * 1000;
#endif
    if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
        spdlog::error("Unable to initialized broadcast timeout on discovery socket: {} - {}", errno, strerror(errno));
        shutdownSocket();
        m_socket = -1;
        return false;
    }

    struct sockaddr_in addressInfo;

    addressInfo.sin_family = AF_INET;
    addressInfo.sin_port = htons(m_port);
    addressInfo.sin_addr.s_addr = INADDR_ANY;


    if (bind(m_socket, (sockaddr*)&addressInfo, sizeof(addressInfo)) < 0) {
        spdlog::error("Unable to start listen on discovery socket: {} - {}", errno, strerror(errno));
	    shutdownSocket();
        m_socket = -1;
        return false;
    }

    m_executionThread = std::thread(&DiscoveryService::threadFunc, this);

#ifdef WIN32
    SetThreadDescription(m_executionThread.native_handle(), L"Discovery Thread");
#endif

    return true;
}


