// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "systeminfo.h"
#include "cryptoutils.h"

#include <vector>
#include <algorithm>
#include <atomic>

#ifdef WIN32
    #include <winsock.h>
    #include <windows.h>
    #include <intrin.h>       
    #include <iphlpapi.h>

#else

    #include <unistd.h>
    #include <string.h>
    #include <sys/types.h>       
    #include <sys/socket.h>      
    #include <sys/ioctl.h>  
    #include <sys/resource.h>    
    #include <sys/utsname.h>       
    #include <netdb.h>           
    #include <netinet/in.h>      
    #include <netinet/in_systm.h>                 
    #include <netinet/ip.h>      
    #include <netinet/ip_icmp.h> 	
    #include <net/if.h>

#endif

#ifdef WIN32

unsigned int    SystemInfo::deviceMACHash() {
    IP_ADAPTER_INFO AdapterInfo[32];
    DWORD dwBufLen = sizeof(AdapterInfo);

    DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
    if (dwStatus != ERROR_SUCCESS)
        return 0; // no adapters.      

    std::vector<unsigned short>     ethernetMacAddresses;

    PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
    while (pAdapterInfo) {
        if (pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET)
            ethernetMacAddresses.push_back( CryptoUtils::crc16(pAdapterInfo->Address, pAdapterInfo->AddressLength));
        pAdapterInfo = pAdapterInfo->Next;
    }

    unsigned int value = 0;

    for (unsigned short v : ethernetMacAddresses) {
        value += v;
    }

    return value;
}

#else

unsigned int    SystemInfo::deviceMACHash() {
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) return 0;
    
    struct ifconf conf;
    char ifconfbuf[128 * sizeof(struct ifreq)];
    memset(ifconfbuf, 0, sizeof(ifconfbuf));
    conf.ifc_buf = ifconfbuf;
    conf.ifc_len = sizeof(ifconfbuf);
    if (ioctl(sock, SIOCGIFCONF, &conf)) {
        return 0;
    }

    std::vector<unsigned short>     ethernetMacAddresses;

    struct ifreq* ifr;
    for (ifr = conf.ifc_req; (char*)ifr < (char*)conf.ifc_req + conf.ifc_len; ifr++)
    {
        if (ifr->ifr_addr.sa_family != AF_INET)
            continue;

        if (ioctl(sock, SIOCGIFFLAGS, (char *)ifr) < 0) {
            continue;
        }

        if (ioctl(sock, SIOCGIFFLAGS, (char *)ifr) < 0) {
            continue;
        }

        if ((ifr->ifr_flags & IFF_UP) == 0 ||
            (ifr->ifr_flags & IFF_LOOPBACK) ||
            (ifr->ifr_flags & (IFF_BROADCAST |IFF_POINTOPOINT) == 0))
            continue;

        if (ioctl(sock, SIOCGIFHWADDR, ifr) < 0)
            continue;

        ethernetMacAddresses.push_back(CryptoUtils::crc16((unsigned char*)&(ifr->ifr_addr.sa_data), 6));
      
    }

    unsigned int value = 0;

    for (unsigned short v : ethernetMacAddresses) {
        value += v;
    }

    close(sock);

    return value;
}

#endif

unsigned int    SystemInfo::deviceIdent() {
    static unsigned int id = -1;
    if (id == -1) {
        id = deviceMACHash();
    }

    return id;
}



