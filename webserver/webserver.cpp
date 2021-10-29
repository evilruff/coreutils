// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "webserver.h"

#include <cctype>

#include "spdlog/spdlog.h"

#include "constants.h"
#include "stringutils.h"
#include "timeutils.h"
#include "context.h"

#define LWS_OK                      0
#define LWS_ERROR                   1
#define LWS_CLOSE_CONNECTION       -1

#define HTTP_BLOCK_SIZE             2048

static const struct lws_protocol_vhost_options extra_mimetypes = {
        NULL,				    /* "next" pvo linked-list */
        NULL,				    /* "child" pvo linked-list */
        ".wasm",				/* file suffix to match */
        "application/wasm"		/* mimetype to use */
};

static const struct lws_http_mount mime_mount_options = {
    /* .mount_next */		NULL,		/* linked-list "next" */
    /* .mountpoint */		"/",		/* mountpoint URL */
    /* .origin */			"/",        /* serve from dir */
    /* .def */			    "",	        /* default filename */
    /* .protocol */			NULL,
    /* .cgienv */			NULL,
    /* .extra_mimetypes */		&extra_mimetypes,
    /* .interpret */		NULL,
    /* .cgi_timeout */		0,
    /* .cache_max_age */		0,
    /* .auth_mask */		0,
    /* .cache_reusable */		0,
    /* .cache_revalidate */		0,
    /* .cache_intermediaries */	0,
    /* .origin_protocol */		LWSMPRO_FILE,	/* files in a dir */
    /* .mountpoint_len */		1,		/* char count */
    /* .basic_auth_login_file */	NULL,
};



int lws_callback_http(
    struct lws *wsi,
    enum lws_callback_reasons reasons,
    void *user, void *in, size_t len);
int lws_callback_lcs(
    struct lws *wsi,
    enum lws_callback_reasons reasons,
    void *user, void *in, size_t len);

static struct lws_protocols
protocols[] = {
    /* first protocol must always be HTTP handler */
    {
        "http-only",            /* name */
        lws_callback_http,      /* callback */
        sizeof(httpSessionData),
        1024 * 2048,            //1024*100
        1
    },
    {
        "lcs-protocol",        // protocol name - very important!
        lws_callback_lcs, // callback
        0,                     // we don't use any per session data
        1024 * 4048, //1024 * 64
        2,
        NULL
    },
    {NULL, NULL, 0, 0} /* terminator */
};


int lws_callback_http(
    struct lws *wsi,
    enum lws_callback_reasons reason,
    void *user, void *in, size_t len)
{       

    void * proxyPtr = lws_context_user(lws_get_context(wsi));

    WebServer *proxy = reinterpret_cast<WebServer *>(proxyPtr);
    if (!proxy) {
        spdlog::error("No WebServer proxy installed, abort request");
        return LWS_ERROR;
    }

    switch (reason) {        
        case LWS_CALLBACK_HTTP: {
            
                httpSessionData * pSessionData = new httpSessionData();
                lws_set_wsi_user(wsi, pSessionData);
                spdlog::debug("LWS_CALLBACK_HTTP");
                                 
                int fragmentLength = 0;
                int fragmentIndex  = 0;            
              
                bool    bodyRequired = false;

                fragmentLength = lws_hdr_fragment_length(wsi, WSI_TOKEN_OPTIONS_URI, 0);
                if (fragmentLength) {
                    spdlog::debug("options received");
                    char  * uri = (char*)malloc(fragmentLength + 2);
                    lws_hdr_copy_fragment(wsi, uri, fragmentLength + 1, WSI_TOKEN_OPTIONS_URI, 0);
                    if (uri) {
                        pSessionData->url = std::string((const char *)uri);
                    }
                    pSessionData->method = "OPTIONS";
                    free(uri);
                }
                 
                fragmentLength = lws_hdr_fragment_length(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE, 0);
                if (fragmentLength) {
                    char  * uri = (char*)malloc(fragmentLength + 2);
                    lws_hdr_copy_fragment(wsi, uri, fragmentLength + 1, WSI_TOKEN_HTTP_CONTENT_TYPE, 0);
                    if (uri) {
                        pSessionData->contentType = std::string((const char *)uri);
                    }
                    free(uri);
                }

                fragmentLength = lws_hdr_fragment_length(wsi, WSI_TOKEN_POST_URI, 0);
                if (fragmentLength) {
                    char  * uri = (char*)malloc(fragmentLength + 2);
                    lws_hdr_copy_fragment(wsi, uri, fragmentLength + 1, WSI_TOKEN_POST_URI, 0);
                    if (uri) {
                        pSessionData->url = std::string((const char *)uri);
                    }
                    pSessionData->method = "GET";
                    free(uri); 
                    bodyRequired = true;
                }

                fragmentLength = lws_hdr_fragment_length(wsi, WSI_TOKEN_GET_URI, 0);
                if (fragmentLength) {
                    char  * uri = (char*)malloc(fragmentLength + 2);
                    lws_hdr_copy_fragment(wsi, uri, fragmentLength + 1, WSI_TOKEN_GET_URI, 0);
                    if (uri) {
                        pSessionData->url = std::string((const char *)uri);
                    }
                    pSessionData->method = "POST";
                    free(uri);
                }

                if (pSessionData->url.size() == 0) {
                    spdlog::error("Empty URL received, closing connection");
                    return LWS_CLOSE_CONNECTION;
                }

                while ((fragmentLength = lws_hdr_fragment_length(wsi, WSI_TOKEN_HTTP_URI_ARGS, fragmentIndex)) > 0) {
                
                char  * uri = (char*)malloc(fragmentLength + 2);
                
                lws_hdr_copy_fragment(wsi, uri, fragmentLength + 1, WSI_TOKEN_HTTP_URI_ARGS, fragmentIndex);
                               
                if (uri) {
                    std::string     uriItem(uri);
                    std::pair<std::string, std::string>     argsItem = StringUtils::splitPair(uriItem, '=');
                    if (argsItem.first.size()) {
                        StringUtils::toLower(argsItem.first);

                        pSessionData->arguments[argsItem.first] = argsItem.second;
                    }
                }               

                fragmentIndex++;
                free(uri);
            };

                if (!bodyRequired) {
                    spdlog::info("URL requested: {}, request length: {}", pSessionData->url, len);
                    {
                        ContextGuard    ctx;
                        ctx->applicationStatistics().registerHttpRequestReceived();
                    }

                    return proxy->serveContent(wsi, *pSessionData);
                }
        }
        break;
    case LWS_CALLBACK_HTTP_FILE_COMPLETION:
        spdlog::debug("LWS_CALLBACK_HTTP_FILE_COMPLETION");
            return LWS_CLOSE_CONNECTION;
        break;
    case LWS_CALLBACK_HTTP_BODY: {
            if (len) {
                httpSessionData * pSessionData = reinterpret_cast<httpSessionData*>(user);
                if (pSessionData) {
                    pSessionData->body += std::string((char*)in, len);
                }
            }
            spdlog::debug("LWS_CALLBACK_HTTP_BODY");
        }
        break;
    case LWS_CALLBACK_HTTP_BODY_COMPLETION: {
        httpSessionData * pSessionData = reinterpret_cast<httpSessionData*>(user);
        if (pSessionData) {
            if (len) {
                pSessionData->body += std::string((char*)in, len);
            }

            spdlog::info("URL requested: {}, request length: {}", pSessionData->url, len);
            {
                ContextGuard    ctx;
                ctx->applicationStatistics().registerHttpRequestReceived();
            }

            return proxy->serveContent(wsi, *pSessionData);
        }

        spdlog::debug("LWS_CALLBACK_HTTP_BODY_COMPLETION");
        }
        break;
    case LWS_CALLBACK_HTTP_WRITEABLE: {            
            if (proxy->m_httpResponsesToSend.count(wsi)) {
                std::string & data = proxy->m_httpResponsesToSend[wsi];
                std::string response = data.substr(0, HTTP_BLOCK_SIZE);
                data.erase(0, response.size());

                response = std::string(LWS_PRE, ' ') + response;
                lws_write(wsi, (unsigned char*)(response.c_str() + LWS_PRE), response.size()-LWS_PRE, LWS_WRITE_TEXT);
                if (data.size()) {
                    lws_callback_on_writable(wsi);
                }
                else {
                    proxy->m_httpResponsesToSend.erase(wsi);
                    if (lws_http_transaction_completed(wsi))
                        return LWS_CLOSE_CONNECTION;
                }

            }
            else {
                // nothing more to send (empty response)
                if (lws_http_transaction_completed(wsi))
                    return LWS_CLOSE_CONNECTION;
            }

            spdlog::debug("LWS_CALLBACK_HTTP_WRITEABLE");
        }
        break;
        case LWS_CALLBACK_CLOSED_HTTP: {
            httpSessionData * pSessionData = reinterpret_cast<httpSessionData*>(user);
            delete pSessionData;
        }
        break;
    case LWS_CALLBACK_WSI_DESTROY: {
            spdlog::debug("LWS_CALLBACK_WSI_DESTROY");
            proxy->m_httpResponsesToSend.erase(wsi);            
        }
        break;
    default:
        // spdlog::debug("Unhandled HTTP callback: {}", reason);
        break;
    }
    
    return LWS_OK;
}

int lws_callback_lcs(
    struct lws * wsi,
    enum lws_callback_reasons reason,
    void *user, void *in, size_t len)
{
    if (user == nullptr)
        user = lws_context_user(lws_get_context(wsi));

    WebServer *proxy = reinterpret_cast<WebServer *>(user);
    if (!proxy) {
        spdlog::error("No WebServer proxy installed, abort request");
        return LWS_ERROR;
    }

    switch (reason) {
    case LWS_CALLBACK_ESTABLISHED: 
        spdlog::debug("LWS_CALLBACK_ESTABLISHED");
        break;
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        spdlog::debug("LWS_CALLBACK_CLIENT_CONNECTION_ERROR");
        break;
    case LWS_CALLBACK_CLOSED: {
            spdlog::debug("LWS_CALLBACK_CLOSED");
            proxy->onConnectionClose(wsi);
        }
        break;
    case LWS_CALLBACK_SERVER_WRITEABLE: {
            spdlog::debug("LWS_CALLBACK_CLIENT_WRITEABLE");
            if (proxy->hasNextResponse(wsi)) {
                spdlog::debug("Sending WSS response");
                std::string response = proxy->fetchNextResponse(wsi);
                int nLen = response.size();
                response = std::string( LWS_PRE, ' ') + response;
                lws_write(wsi, (unsigned char*)(response.c_str() + LWS_PRE), nLen, LWS_WRITE_TEXT);
            }

            if (proxy->hasNextResponse(wsi)) {
                lws_callback_on_writable(wsi);
            }
        }
        break;
    case LWS_CALLBACK_RECEIVE: {
            spdlog::debug("LWS_CALLBACK_RECEIVE");
            if (len > 0) {
                spdlog::debug("WSS request received");
                {
                    ContextGuard    ctx;
                    ctx->applicationStatistics().registerWSSRequestReceived();
                }
                std::string     websocketRequestString((const char *)in, len);
                if (proxy->despatchWSSRequest(wsi, websocketRequestString) != LWS_OK) {
                    return LWS_CLOSE_CONNECTION;
                }
            }
        }
        break;    
    default:
        // spdlog::debug("Unhandled HTTP callback: {}", reason);
        break;

    }
    return LWS_OK;
}

void WebServer::loggingInit()
{
    int debug_level = ( LLL_ERR | LLL_WARN );
    lws_set_log_level(debug_level, [](int level, const char *line) {
        spdlog::info("{}", line);
    });

    lwsl_notice("LightController WebSocket Server\n");
}

lws_context * WebServer::createWebServerContext(unsigned short port)
{
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(struct lws_context_creation_info));
    info.port = port;
    info.iface = NULL;
    info.protocols = protocols;
#if defined(LWS_WITH_TLS)
    info.ssl_cert_filepath = NULL;
    info.ssl_private_key_filepath = NULL;
#endif
    info.gid = -1;
    info.uid = -1;
    info.user = this;
    info.options = 0;

    // create lws context representing this server
    return lws_create_context(&info);
}

WebServer::WebServer(unsigned short port) {
    spdlog::info("WebServer initialized on port {}", port);
    
    loggingInit();

    m_context = createWebServerContext(port);
    if (m_context == nullptr)
        lwsl_err("WebServer context was not created.\n");
}

WebServer::~WebServer()
{
    quit();
    spdlog::info("WebServer destroyed");    
}

bool WebServer::isValid() const
{
    return (m_context != NULL);
}

int WebServer::serveContent(struct lws * wsi, const httpSessionData & sessionData) {

    uint8_t tempBuffer[LWS_PRE + 2048], *startTempBuffer = &tempBuffer[LWS_PRE], *tempPtr = startTempBuffer,
        *endTempBuffer = &tempBuffer[sizeof(tempBuffer) - LWS_PRE - 1];

    std::string fileName = sessionData.url;
    
    std::string normalizedUrl = StringUtils::toUpper(sessionData.url);

    if (sessionData.method == "OPTIONS") {

        if (lws_add_http_common_headers(wsi, HTTP_STATUS_OK, nullptr, 0,
            &tempPtr, endTempBuffer))
            return LWS_ERROR;

        std::string optionValue = "POST, GET, OPTIONS";

        if (lws_add_http_header_by_name(wsi, (const unsigned char*)"Access-Control-Allow-Methods:", (const unsigned char*)optionValue.c_str(), optionValue.size(),
            &tempPtr, endTempBuffer))
            return LWS_ERROR;

        optionValue = "Content-Type,Authorization";
        if (lws_add_http_header_by_name(wsi, (const unsigned char*)"Access-Control-Allow-Headers:", (const unsigned char*)optionValue.c_str(), optionValue.size(),
            &tempPtr, endTempBuffer))
            return LWS_ERROR;

        optionValue = "*";
        if (lws_add_http_header_by_name(wsi, (const unsigned char*)"Access-Control-Allow-Origin:", (const unsigned char*)optionValue.c_str(), optionValue.size(),
            &tempPtr, endTempBuffer))
            return LWS_ERROR;

        if (lws_finalize_write_http_header(wsi, startTempBuffer, &tempPtr, endTempBuffer))
            return LWS_ERROR;

        lws_callback_on_writable(wsi);
        return LWS_OK;
    }
    
    if (m_httpCommands.count(normalizedUrl)) {
        spdlog::info("Serving {} as dynamic contect", sessionData.url);
           
        httpContentResponse     contentToSend = (m_httpCommands[normalizedUrl])(sessionData.url, sessionData);

        if (lws_add_http_common_headers(wsi, HTTP_STATUS_OK, contentToSend.mimeType.c_str(), contentToSend.content.size(),         
            &tempPtr, endTempBuffer))
            return LWS_ERROR;
        
        std::string optionValue = "*";
        if (lws_add_http_header_by_name(wsi, (const unsigned char*)"Access-Control-Allow-Origin:", (const unsigned char*)optionValue.c_str(), optionValue.size(),
            &tempPtr, endTempBuffer))
            return LWS_ERROR;

        if (lws_finalize_write_http_header(wsi, startTempBuffer, &tempPtr, endTempBuffer))
            return LWS_ERROR;
       
        m_httpResponsesToSend[wsi] = contentToSend.content;

        lws_callback_on_writable(wsi);
        return LWS_OK;
    }
   

    if (StringUtils::endsWith(fileName, "/") ) {
        fileName += m_defaultDocument;
    }

    fileName = m_httpRoot + fileName;
    const char * mimeType = lws_get_mimetype(fileName.c_str(), &mime_mount_options);
    
    if (mimeType == nullptr) {
        spdlog::error("Mime type for file {} not found", fileName);            
    }

    spdlog::info("Serving {} as {}", sessionData.url, mimeType);

    std::string optionValue = "*";
    if (lws_add_http_header_by_name(wsi, (const unsigned char*)"Access-Control-Allow-Origin:", (const unsigned char*)optionValue.c_str(), optionValue.size(),
        &tempPtr, endTempBuffer))
        return LWS_ERROR;

    
    return lws_serve_http_file(wsi, fileName.c_str(), mimeType, NULL, 0);
}

int        WebServer::despatchWSSRequest(struct lws * wsi, const std::string & request) {
    wssCommand  command;

    ElapsedTimer    et;
    try {
        nlohmann::json  json = nlohmann::json::parse(request);
        json.get_to(command);
        StringUtils::toUpper(command.command);

        if (m_wssCommands.count(command.command)) {
            spdlog::debug("Despatching {} command", command.command);
            std::string response = m_wssCommands[command.command](request);

            if (response.size()) {
                m_responses[wsi].push_back(response);
                lws_callback_on_writable(wsi);
            }
            else {
                spdlog::info("Empty response received from handler for command {}", command.command);
                return LWS_OK;
            }
        }
        else {
            spdlog::error("Not supported WSS request {} received from UI", command.command);
            return LWS_OK;
        }
        spdlog::info("Request {} serviced for {} ms", command.command, et.elapsed());

    }
    catch (nlohmann::json::parse_error & e) {
        spdlog::error("Unable to parse JSON data, {}", e.what());
        return LWS_ERROR;
    }
    catch (nlohmann::json::out_of_range & e) {
        spdlog::error("JSON config section not found {}", e.what());
        return LWS_ERROR;
    }
    catch (nlohmann::json::type_error & e) {
        spdlog::error("Unable to fetch JSON data, {}", e.what());
        return LWS_ERROR;
    }
  
    return LWS_OK;
}

void       WebServer::registerHttpCallback(const std::string & command, httpCallback f) {
    std::string tempName = command;
    StringUtils::toUpper(tempName);
    m_httpCommands[tempName] = f;
}


void      WebServer::registerWssCallback(const std::string & command, wsiCallback f) {
    std::string tempName = command;
    StringUtils::toUpper(tempName);
    m_wssCommands[tempName] = f;
}

std::string     WebServer::fetchNextResponse(struct lws * wsi) {
    std::string response = m_responses[wsi].front();
    m_responses[wsi].pop_front();
    if (m_responses[wsi].size() == 0) {
        m_responses.erase(wsi);
    }

    return response;
}

bool            WebServer::hasNextResponse(struct lws * wsi) {
    if (m_responses.count(wsi) == 0)
        return false;

    if (m_responses[wsi].size() == 0) {
        m_responses.erase(wsi);
        return false;
    }

    return true;
}

void WebServer::start() {
    m_executionThread = std::thread(&WebServer::threadFunc, this);

#ifdef WIN32
    SetThreadDescription(m_executionThread.native_handle(), L"WebServer Thread");
#endif
}
void WebServer::quit() {
    m_bExit = true;

    if (m_context) {
        lws_cancel_service(m_context);
    }

    if (m_executionThread.joinable()) {
        m_executionThread.join();
    }

    if (m_context) {      
        lws_context_destroy(m_context);
        m_context = nullptr;
    }    
}

void            WebServer::threadFunc() {
    while (!m_bExit) {
        processEvents();
    }
}

void            WebServer::processEvents(int nPeriod) {
    lws_service(m_context, nPeriod);
}


void            WebServer::onConnectionClose(struct lws * wsi) {
    m_responses.erase(wsi);
}

void from_json(const nlohmann::json & j, wssCommand & command) {
    j.at("command").get_to(command.command);
}

