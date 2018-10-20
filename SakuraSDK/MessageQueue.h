#pragma once
#include <stack>
#include <queue>
#include <mutex>
#include <future>
#include <optional>
namespace tignear::thread {
	enum class MessageQueueMode {
		FIFO, LIFO
	};
	namespace impl {
		template<class MessageType, MessageQueueMode mode>
		struct ContainerTypeIf {};
		template<class MessageType>
		struct ContainerTypeIf<MessageType, MessageQueueMode::FIFO> {
			using type = std::queue<MessageType>;
		};
		template<class MessageType>
		struct ContainerTypeIf<MessageType, MessageQueueMode::LIFO> {
			using type = std::stack<MessageType>;
		};
	}
	template <class MessageType, MessageQueueMode mode=MessageQueueMode::FIFO>
	class MessageQueue {
	private:
		using ctype =typename impl::ContainerTypeIf<MessageType,mode>::type;
		std::mutex m_mtx;
		ctype m_messages;
		std::optional<std::promise<MessageType>> m_wait_promise=std::nullopt;
	public:
		[[nodiscard]]std::future<MessageType> message() {
			std::promise<MessageType> promise;
			std::scoped_lock<std::mutex> lock(m_mtx);
			if (m_messages.empty()) {
				m_wait_promise = std::move(promise);
				return m_wait_promise.value().get_future();
			}
			promise.set_value(std::move(m_messages.front()));
			m_messages.pop();
			return promise.get_future();
		}
		void postMessage(MessageType&& message) {
			std::scoped_lock<std::mutex> lock(m_mtx);
			if (m_wait_promise) {
				m_wait_promise.value().set_value(std::forward<MessageType>(message));
				m_wait_promise = std::nullopt;
				return;
			}
			m_messages.emplace(std::forward<MessageType>(message));
		}
	};
}