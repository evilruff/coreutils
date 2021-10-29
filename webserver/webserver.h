// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <functional>
#include <string>
#include <vector>
#include <list>
#include <thread>
#include <atomic>

#include "json.hpp"

#include "libwebsockets.h"

struct wssCommand {
    std::string command;
};

struct httpContentResponse {
    std::string     content;
    std::string     mimeType = "text/plain";
};

struct httpSessionData {
    std::string                         body;
    std::string                         url;
    std::string                         contentType;
    std::string                         method;
    std::map<std::string, std::string>  arguments;
};

typedef std::function<std::string(const std::string & request)> wsiCallback;
typedef std::function<httpContentResponse(const std::string & request, const httpSessionData & sessionData)> httpCallback;
class WebServer
{
public:
  
    WebServer(unsigned short port = 9000);
    ~WebServer();

    void setHttpRoot(const std::string & root)  { m_httpRoot = root; };
    std::string     httpRoot() const            { return m_httpRoot; };

    void            setDefaultDocument(const std::string & defaultDocument) { m_defaultDocument = defaultDocument; };
    std::string     defaultDocument() const { return m_defaultDocument; };

    void            registerWssCallback(const std::string & command, wsiCallback f);
    void            registerHttpCallback(const std::string & command, httpCallback f);

    bool            isValid() const;
    
    void            loggingInit();

    void            start();
    void            quit();

  
protected:

    void            threadFunc();

    void            processEvents(int nPeriod = 0);

    int             serveContent(struct lws * wsi, const httpSessionData & httpSessionData);
    int             despatchWSSRequest(struct lws * wsi, const std::string & request);
    std::string     fetchNextResponse(struct lws * wsi);
    bool            hasNextResponse(struct lws * wsi);
    void            onConnectionClose(struct lws * wsi);
    
protected:
    
    friend int lws_callback_http(
		        struct lws *wsi,
                enum lws_callback_reasons reasons,
		        void *user, void *in, size_t len);

    friend int lws_callback_lcs(
		        struct lws *wsi,
                enum lws_callback_reasons reasons,
		        void *user, void *in, size_t len);

    lws_context * createWebServerContext(unsigned short port);

protected:

    std::map<lws *, std::string>                   m_httpResponsesToSend;
    std::map<lws *, std::list<std::string>>        m_responses;
    std::string                                    m_httpRoot;
    std::string                                    m_defaultDocument;
    lws_context                                  * m_context = nullptr;

    std::atomic_bool                               m_bExit = false;

    std::thread                                    m_executionThread;
        
    std::map<std::string, httpCallback>              m_httpCommands;
    std::map<std::string, wsiCallback>              m_wssCommands;
};

void from_json(const nlohmann::json & j, wssCommand & command);