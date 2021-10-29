// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <vector>
#include <time.h>

#include "mqttclient.h"
#include "context.h"
#include "calcutils.h"

class MQTTHandler {
public:
    MQTTHandler(int defaultPort);
    ~MQTTHandler() = default;

    void         quit();
    MQTTClient * client() const { return m_pClient; };

    bool          isLocalConnected() const;

    void         setReconnectDelay(int timeout) {
        m_reconnectDelay = timeout;
    }
    int          reconnectDelay() const { return m_reconnectDelay; };
  
    void        publishRole(const char * role);
    void        publishTime(time_t  time);
    void        publishChannels(const ChannelValues & channels);
    void        publishCurrentChannels(const ChannelValues & channels);
    void        publishDailyCycle(const char * dailyCycleInfo);

protected:

    void        initHandlers();
   
protected:

    int             m_reconnectDelay = 10000;
    MQTTClient *    m_pClient;

};


