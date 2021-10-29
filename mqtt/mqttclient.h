// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include "mosquitto.h"

#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <map>

#include "bytearray.h"
#include "syncvalue.h"
#include "syncvalue.h"

enum mqtt5_property {
    MQTT_PROP_PAYLOAD_FORMAT_INDICATOR = 1,		/* Byte :				PUBLISH, Will Properties */
    MQTT_PROP_MESSAGE_EXPIRY_INTERVAL = 2,		/* 4 byte int :			PUBLISH, Will Properties */
    MQTT_PROP_CONTENT_TYPE = 3,					/* UTF-8 string :		PUBLISH, Will Properties */
    MQTT_PROP_RESPONSE_TOPIC = 8,				/* UTF-8 string :		PUBLISH, Will Properties */
    MQTT_PROP_CORRELATION_DATA = 9,				/* Binary Data :		PUBLISH, Will Properties */
    MQTT_PROP_SUBSCRIPTION_IDENTIFIER = 11,		/* Variable byte int :	PUBLISH, SUBSCRIBE */
    MQTT_PROP_SESSION_EXPIRY_INTERVAL = 17,		/* 4 byte int :			CONNECT, CONNACK, DISCONNECT */
    MQTT_PROP_ASSIGNED_CLIENT_IDENTIFIER = 18,	/* UTF-8 string :		CONNACK */
    MQTT_PROP_SERVER_KEEP_ALIVE = 19,			/* 2 byte int :			CONNACK */
    MQTT_PROP_AUTHENTICATION_METHOD = 21,		/* UTF-8 string :		CONNECT, CONNACK, AUTH */
    MQTT_PROP_AUTHENTICATION_DATA = 22,			/* Binary Data :		CONNECT, CONNACK, AUTH */
    MQTT_PROP_REQUEST_PROBLEM_INFORMATION = 23,	/* Byte :				CONNECT */
    MQTT_PROP_WILL_DELAY_INTERVAL = 24,			/* 4 byte int :			Will properties */
    MQTT_PROP_REQUEST_RESPONSE_INFORMATION = 25,/* Byte :				CONNECT */
    MQTT_PROP_RESPONSE_INFORMATION = 26,		/* UTF-8 string :		CONNACK */
    MQTT_PROP_SERVER_REFERENCE = 28,			/* UTF-8 string :		CONNACK, DISCONNECT */
    MQTT_PROP_REASON_STRING = 31,				/* UTF-8 string :		All except Will properties */
    MQTT_PROP_RECEIVE_MAXIMUM = 33,				/* 2 byte int :			CONNECT, CONNACK */
    MQTT_PROP_TOPIC_ALIAS_MAXIMUM = 34,			/* 2 byte int :			CONNECT, CONNACK */
    MQTT_PROP_TOPIC_ALIAS = 35,					/* 2 byte int :			PUBLISH */
    MQTT_PROP_MAXIMUM_QOS = 36,					/* Byte :				CONNACK */
    MQTT_PROP_RETAIN_AVAILABLE = 37,			/* Byte :				CONNACK */
    MQTT_PROP_USER_PROPERTY = 38,				/* UTF-8 string pair :	All */
    MQTT_PROP_MAXIMUM_PACKET_SIZE = 39,			/* 4 byte int :			CONNECT, CONNACK */
    MQTT_PROP_WILDCARD_SUB_AVAILABLE = 40,		/* Byte :				CONNACK */
    MQTT_PROP_SUBSCRIPTION_ID_AVAILABLE = 41,	/* Byte :				CONNACK */
    MQTT_PROP_SHARED_SUB_AVAILABLE = 42,		/* Byte :				CONNACK */
};


class MQTTClient {

public:

    typedef      std::function<void()>                                  networkEventCallback;
    typedef      std::function<void(const char *, int)>                 networkConnectedEventCallback;
    typedef      std::function<void(char *, const ByteArray & data)>    messageCallback;

    MQTTClient();
    ~MQTTClient();

    void          setBroker(const char * host, int port);
    void          setBroker(const char * host);

    std::string   brokerHost()  const {  return (std::string)m_brokerHost; };
    int           brokerPort()  const {  return m_brokerPort; };
    bool          isValid()     const {  return m_bInitialized; };

    bool          isConnected()   const {  return m_bConnected; };    
    bool          connect();

    void          start(int nLoopTimeout = 101);
    void          quit();

    MQTTClient &  onMessage(const char * pattern, messageCallback callback);
   
    MQTTClient &  onConnect(networkConnectedEventCallback handler) { onConnectCallback = handler; return *this; }
    MQTTClient &  onDisconnect(networkEventCallback handler) { onDisconnectCallback = handler; return *this; }

    bool          publish(const char * channel, const ByteArray & data, int ttl = -1, bool bRetain = false);
    bool          publish(const char * channel, const char * str, int ttl = -1, bool bRetain = false);
    
    friend      void onDisconnectHandler(struct mosquitto * context, void * obj, int retCode);
    friend      void onMessageHandler(struct mosquitto *, void *, const struct mosquitto_message *);
    
protected:

    void            threadFunc();
    void            processEvents(int nPeriod);
    void            subscribeOnTopics();

protected:

    std::map<std::string, messageCallback>         onReceiveCallbacks;

    networkConnectedEventCallback                  onConnectCallback    = nullptr;
    networkEventCallback                           onDisconnectCallback = nullptr;
    
    mosquitto *                                    m_context        = nullptr;
    bool                                           m_bInitialized   = false;
    int                                            m_nLoopTimeout   = 100;

    std::atomic_bool                               m_bDoConnectAttempt  = false;
    std::atomic_bool                               m_bExit              = false;
    std::atomic_bool                               m_bConnected         = false;

    std::thread                                    m_executionThread;

    SyncValue<std::string>  m_brokerHost;
    std::atomic_int         m_brokerPort;
};

