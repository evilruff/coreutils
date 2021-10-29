// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "mqtthandler.h"

#include "spdlog/spdlog.h"
#include "json.hpp"

#include "constants.h"
#include "timers.h"
#include "eventloop.h"
#include "events.h"
#include "systeminfo.h"

MQTTHandler::MQTTHandler(int defaultPort) {
    m_pClient = new MQTTClient();
    m_pClient->setBroker(LOCAL_ADDRESS, defaultPort);

    initHandlers();

    m_pClient->start(100);
}

bool           MQTTHandler::isLocalConnected() const {
    if (!m_pClient)
        return false;

    return (m_pClient->brokerHost() == std::string(LOCAL_ADDRESS));
}

void       MQTTHandler::initHandlers() {
    m_pClient->onDisconnect([this]() {
        
        EventLoop & mainLoop = Singleton<EventLoop>::instance();
        mainLoop.sendEvent<MQTTBrokerDisconnectEvent>();

     
    });

    m_pClient->onConnect([](const char * host, int port) {
        spdlog::info("MQTT Broker connected");
        EventLoop & mainLoop = Singleton<EventLoop>::instance();
        mainLoop.sendEvent<MQTTBrokerConnectedEvent>(host, port);
    });

    m_pClient->onMessage(MQTT_CHANNELS_TOPIC, [this](char *, const ByteArray & data) {
        try {
            nlohmann::json  json = nlohmann::json::parse(data.toString().c_str());
            
            unsigned int clientId = 0;
            json.at("cid").get_to(clientId);
            if (clientId == SystemInfo::deviceIdent())
                return;

            std::string         tempString;
            std::vector<int>    channels;

            json.at("channels").get_to(channels);
            ChannelValues   values(channels.size(), ChannelValue());
            
            std::transform(channels.begin(), channels.end(), values.begin(), [](int v) { return ChannelValue::fromFraction(v); });

            EventLoop & mainLoop = Singleton<EventLoop>::instance();
            mainLoop.sendEvent<MQTTChannelsReceivedEvent>(values);

            spdlog::info("CHANNELS received from client 0x{:X}", clientId);
        }
        catch (nlohmann::json::parse_error & e) {
            spdlog::error("Unable to parse JSON data, {}", e.what());
            return;
        }
        catch (nlohmann::json::out_of_range & e) {
            spdlog::error("Unable to fetch JSON data, {}", e.what());
            return;
        }
        catch (nlohmann::json::type_error & e) {
            spdlog::error("Unable to fetch JSON data, {}", e.what());
            return;
        }       
    });

    m_pClient->onMessage(MQTT_TIME_TOPIC, [this](char *, const ByteArray & data) {
        
        try {            
            nlohmann::json  json = nlohmann::json::parse(data.toString().c_str());
            
            unsigned int clientId = 0;
            json.at("cid").get_to(clientId);            
            if (clientId == SystemInfo::deviceIdent())
                return;
        
            std::string tempString;
            json.at("time").get_to(tempString);
            time_t t = std::atol(tempString.c_str());

            EventLoop & mainLoop = Singleton<EventLoop>::instance();
            mainLoop.sendEvent<MQTTTimeReceivedEvent>(t);
        }
        catch (nlohmann::json::parse_error & e) {
            spdlog::error("Unable to parse JSON data, {}", e.what());
            return;
        }
        catch (nlohmann::json::out_of_range & e) {
            spdlog::error("Unable to fetch JSON data, {}", e.what());
            return;
        }
        catch (nlohmann::json::type_error & e) {
            spdlog::error("Unable to fetch JSON data, {}", e.what());
            return;
        }
    });
}

void        MQTTHandler::publishRole(const char * role) {
    nlohmann::json   j;
    j["role"] = role;
    j["cid"]  = SystemInfo::deviceIdent();
    spdlog::info("MQTT publishing role");
    m_pClient->publish(MQTT_ROLE_TOPIC, j.dump().c_str(), -1, true);
}

void        MQTTHandler::publishTime(time_t  time) {
    nlohmann::json   j;
    j["time"] = std::to_string(time);
    j["cid"] = SystemInfo::deviceIdent();
    spdlog::info("MQTT publishing time");
    m_pClient->publish(MQTT_TIME_TOPIC, j.dump().c_str(), 5000);
}

void        MQTTHandler::publishChannels(const ChannelValues & channels) {

    nlohmann::json   j;
    nlohmann::json  jsonChannels = nlohmann::json::array();

    for (const ChannelValue & item : channels) {
        jsonChannels.push_back(item.fraction());
    }
    j["channels"] = jsonChannels;
    j["cid"]      = SystemInfo::deviceIdent();
    spdlog::info("MQTT publishing channels");
    m_pClient->publish(MQTT_CHANNELS_TOPIC, j.dump().c_str(), 5000);
}

void        MQTTHandler::publishDailyCycle(const char * dailyCycleInfo) {

    nlohmann::json   j;
    nlohmann::json  jsonChannels = nlohmann::json::array();

    j["channels"] = dailyCycleInfo ? dailyCycleInfo : "";
    j["cid"] = SystemInfo::deviceIdent();
    spdlog::info("MQTT publishing channels");
    m_pClient->publish(MQTT_CURRENT_DAILY_CYCLE_TOPIC, j.dump().c_str(), 5000);
}

void        MQTTHandler::publishCurrentChannels(const ChannelValues & channels) {

    nlohmann::json   j;
    nlohmann::json  jsonChannels = nlohmann::json::array();

    for (const ChannelValue & item : channels) {
        jsonChannels.push_back(item.fraction());
    }
    j["channels"] = jsonChannels;
    j["cid"] = SystemInfo::deviceIdent();
    spdlog::info("MQTT publishing channels");
    m_pClient->publish(MQTT_CURRENT_CHANNELS_TOPIC, j.dump().c_str(), 5000);
}

void         MQTTHandler::quit() {
    m_pClient->quit();
    delete m_pClient;
    m_pClient = nullptr;
}

