// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "mqttclient.h"

#include "context.h"
#include "spdlog/spdlog.h"
#include "timeutils.h"



void onDisconnectHandler(struct mosquitto * context, void * obj, int retCode) {
    MQTTClient * pObj = static_cast<MQTTClient*>(obj);
    if (!pObj)
        return;

    if (retCode > 0) {
        spdlog::error("MQTT connection lost to {}:{}", (std::string)pObj->m_brokerHost, pObj->m_brokerPort);
    } else {
        spdlog::info("MQTT disconnected from {}:{}", pObj->brokerHost(), pObj->brokerPort());
    }

    pObj->m_bConnected = false;
    if (pObj->onDisconnectCallback) {
        pObj->onDisconnectCallback();
    }       
}

void onMessageHandler(struct mosquitto * context, void * obj, const struct mosquitto_message * msg) {
    MQTTClient * pObj = static_cast<MQTTClient*>(obj);
    if (!pObj || !msg)
        return;


    if (!pObj->onReceiveCallbacks.count(std::string(msg->topic)))
        return;

    MQTTClient::messageCallback callback = pObj->onReceiveCallbacks[std::string(msg->topic)];
    if (!callback) {
        spdlog::error("MQTT Message received for topic {}, but no handler installed, skipping", msg->topic);
        return;
    }

    ByteArray   data((const char *)msg->payload, msg->payloadlen);

    callback(msg->topic,data);
}

void          MQTTClient::subscribeOnTopics() {
    spdlog::info("MQTT connection restored, resubscribing on topics");
    for (const std::pair<std::string, messageCallback> & item : onReceiveCallbacks) {
        mosquitto_subscribe(m_context, nullptr, item.first.c_str(), 0);
    }
}

MQTTClient &  MQTTClient::onMessage(const char * pattern, messageCallback callback) {
    if (m_bExit || !m_context)
        return *this;

    onReceiveCallbacks[std::string(pattern)] = callback;
    mosquitto_subscribe(m_context, nullptr, pattern, 0);

    return *this;
}

MQTTClient::MQTTClient() {
    if (mosquitto_lib_init() != MOSQ_ERR_SUCCESS) {
        spdlog::error("Unable to initialize MQTT subsystem");
        return;
    } 

    m_bInitialized = true;
    int major    = 0;
    int minor    = 0;
    int revision = 0;
    
    mosquitto_lib_version(&major, &minor, &revision);

    spdlog::info("MQTT version {}.{}.{} initialized", major, minor, revision);

    unsigned int deviceId;
    {
        ContextGuard    ctx;
        deviceId = ctx->deviceId();
    }

    char    mqttId[24];
    sprintf(mqttId, "LCS-%X", deviceId);

    m_context = mosquitto_new(mqttId, true, this);
    mosquitto_int_option(m_context, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5);
    mosquitto_threaded_set(m_context, true);

    if (m_context) {
        mosquitto_log_callback_set(m_context, [](struct mosquitto * context, void * obj, int logLevel, const char * info) {
            if (logLevel == MOSQ_LOG_INFO) {
                spdlog::info("MQTT: {}",info);
            }

            if (logLevel == MOSQ_LOG_ERR) {
                spdlog::info("MQTT: {}", info);
            }
        });
       
        mosquitto_disconnect_callback_set(m_context, onDisconnectHandler);
        mosquitto_message_callback_set(m_context, onMessageHandler);           
    }
}

MQTTClient::~MQTTClient() {
    quit();
    mosquitto_destroy(m_context);
    mosquitto_lib_cleanup();
    spdlog::info("MQTT client destroyed");
}


void          MQTTClient::setBroker(const char * host) {
    setBroker(host, m_brokerPort);
}

void          MQTTClient::setBroker(const char * host, int port) {
    if (m_brokerHost != host || m_brokerPort != port) {
        if (m_context) {
            mosquitto_disconnect(m_context);
        }        
    }

    m_brokerHost = std::string(host);
    m_brokerPort = port;
}

bool          MQTTClient::connect() {
    if (m_bExit || !m_context)
        return false;

    spdlog::info("MQTT attempting connect to broker at {}:{}", (std::string)m_brokerHost, m_brokerPort);

    m_bDoConnectAttempt = true;

    return true;
}

void          MQTTClient::start(int nLoopTimeout) {
    m_nLoopTimeout = nLoopTimeout;
    m_executionThread = std::thread(&MQTTClient::threadFunc, this);

#ifdef WIN32
    SetThreadDescription(m_executionThread.native_handle(), L"MQTT Thread");
#endif
}

void          MQTTClient::quit() {
    m_bExit = true;
    if (m_executionThread.joinable()) {
        m_executionThread.join();
    }  
}

void          MQTTClient::threadFunc() {
    while (!m_bExit) {
        processEvents(m_nLoopTimeout);
    }
}

bool           MQTTClient::publish(const char * channel, const ByteArray & data, int ttl, bool bRetain) {
    if (m_bExit || !m_context)
        return false;

    if (!m_bConnected) {
        connect();
        return false;
    }

    mosquitto_property *proplist = nullptr;
    if (ttl > 0) {
        mosquitto_property_add_int32(&proplist, MQTT_PROP_MESSAGE_EXPIRY_INTERVAL, ttl);
    }

    bool bRes = (mosquitto_publish_v5(m_context, nullptr, channel, data.size(), data.data(), 0, bRetain, proplist) == MOSQ_ERR_SUCCESS);
    
    if (proplist)
        mosquitto_property_free_all(&proplist);

    return bRes;
}

bool          MQTTClient::publish(const char * channel, const char * str, int ttl, bool bRetain) {
    if (m_bExit || !m_context)
        return false;

    if (!m_bConnected) {
        connect();
        return false;
    }

    mosquitto_property *proplist = nullptr;
    if (ttl > 0) {
        mosquitto_property_add_int32(&proplist, MQTT_PROP_MESSAGE_EXPIRY_INTERVAL, ttl);
    }

    bool bRes = (mosquitto_publish_v5(m_context, nullptr, channel, strlen(str), str, 0, bRetain, proplist) == MOSQ_ERR_SUCCESS);

    if (proplist) 
        mosquitto_property_free_all(&proplist);

    return bRes;
}

void          MQTTClient::processEvents(int nPeriod) {
    ElapsedTimer et;

    if (m_bDoConnectAttempt) {
        if (m_bConnected) {
            mosquitto_disconnect(m_context);
            m_bConnected = false;
        }

        if (mosquitto_connect(m_context, ((std::string)m_brokerHost).c_str(), m_brokerPort, 30000) ==
            MOSQ_ERR_SUCCESS) {
            m_bConnected = true;
            spdlog::debug("MQTT connected to broker at {}:{}", (std::string)m_brokerHost, m_brokerPort);
            subscribeOnTopics();
            if (onConnectCallback) {                                
                onConnectCallback(((std::string)m_brokerHost).c_str(), m_brokerPort);
            } 
        } else {
            spdlog::debug("MQTT unable to connect to broker at {}:{}", (std::string)m_brokerHost, m_brokerPort);
            if (onDisconnectCallback) {
                onDisconnectCallback();
            }
        }
        m_bDoConnectAttempt = false;
    }

    mosquitto_loop(m_context, nPeriod, 1);

    unsigned long long elapsed = et.elapsed();

    if (nPeriod > elapsed) {
        int diff = (int)((unsigned long long)nPeriod - elapsed);
        if (diff > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(diff));
        }
    }
}



