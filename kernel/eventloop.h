// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <typeindex>
#include <atomic>

#ifdef WIN32 
    #include <windows.h>
#endif

class Event {
public:
    virtual ~Event() {};
};

typedef std::function<void(Event&)> EventHandler;

template<typename T> std::type_index getEventType() {
	return std::type_index(typeid(T));
}

class EventLoop {

public:
	EventLoop(const char * name = nullptr) {
        if (name) {
            m_name = name;
        }
    };

	void start() {
		m_eventLoopThread = std::thread(&EventLoop::run, this);
#ifdef WIN32
        const WCHAR *pwcsName; 
        int size = MultiByteToWideChar(CP_ACP, 0, m_name.c_str(), -1, NULL, 0);
        pwcsName = new WCHAR[size];
        MultiByteToWideChar(CP_ACP, 0, m_name.c_str(), -1, (LPWSTR)pwcsName, size);
        SetThreadDescription(m_eventLoopThread.native_handle(), pwcsName);
        delete[] pwcsName;
#endif
	}

    void exec() {
        run();
    }

	void join() {
		if (m_eventLoopThread.joinable())
			m_eventLoopThread.join();
	}

	void quit() {
		m_bExit = true;
		m_eventWaitVar.notify_one();
	}

	template<class EventType>
	void onEvent(std::function<void(const EventType &)> handler) {
		std::lock_guard<std::mutex> guard(m_handlersLock);
		std::type_index eventTypeId = getEventType<EventType>();

		m_handlers[eventTypeId].push_back([handler](Event& e) {
			handler(static_cast<EventType&>(e));
		});
	}

	// Convert an event to a message to be handled by the event loop.
	template <class EventType, typename ...EventParamsType>
	void sendEvent(EventParamsType&&... event_params) {
		if (m_bExit)
			return;

		std::type_index eventTypeId = getEventType<EventType>();
		std::lock_guard<std::mutex> lock(m_eventsLock);

		std::unique_ptr<EventType> event = std::make_unique<EventType>(std::forward<EventParamsType>(event_params)...);

		m_events.emplace(std::move(event), eventTypeId);
		m_eventWaitVar.notify_one();
	}

    template <class EventType>
    void sendEvent(EventType && evt) {
        if (m_bExit)
            return;

        std::type_index eventTypeId = getEventType<EventType>();
		std::lock_guard<std::mutex> lock(m_eventsLock);

        std::unique_ptr<EventType> event = std::make_unique<EventType>(std::move(evt));

        m_events.emplace(std::move(event), eventTypeId);
        m_eventWaitVar.notify_one();
    }

protected:
	void run() {
		while (true) {
			std::unique_lock<std::mutex> lock(m_eventsLock);

			m_eventWaitVar.wait(lock, [&] { return (m_events.size() > 0) || m_bExit; });
			if (m_bExit)
				return;

			std::pair<std::unique_ptr<Event>, std::type_index> eventItem = std::move(m_events.front());
			m_events.pop();
			lock.unlock();

            if (m_handlers.count(eventItem.second)) {
                for (EventHandler handler : m_handlers[eventItem.second]) {
                    handler(*eventItem.first);
                }
            }
		}
	}


protected:

	std::queue<std::pair<std::unique_ptr<Event>, std::type_index>> m_events;
	std::mutex m_eventsLock;

	std::condition_variable	m_eventWaitVar;

	std::map<std::type_index, std::vector<EventHandler>> m_handlers;
	std::mutex	m_handlersLock;

	std::thread m_eventLoopThread;

    std::string m_name;

	std::atomic_bool	m_bExit = false;
};