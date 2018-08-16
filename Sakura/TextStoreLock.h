#pragma once
#include <functional>
#include <optional>
#include <atomic>
#include "shared_recursive_mutex.h"
namespace tignear::tsf {
	class TextStoreLock {
	public:
		TextStoreLock() {}
		bool IsLock(bool write) const{
			if (write) {
				return m_ts_write_lock;
			}
			else {
				return m_read_lock > 0;
			}
		}
		template <class R>
		std::optional<R> TryWriteLockToCallTS(std::function<R()> fn) {
			std::optional<std::optional<R>> r = TryWriteLockToCallBase < std::optional<R> >([this, fn] {
				auto exp = false;
				if (m_write_lock == 1 && m_ts_write_lock.compare_exchange_weak(exp, true)) {
					auto r = fn();
					m_ts_write_lock = false;
					return r;
				}
				return std::nullopt;
			});
			return r.value_or(std::nullopt);
		}
		bool TryWriteLockToCallTS(std::function<void()> fn) {
			std::optional<bool> r = TryWriteLockToCallBase<bool>([this, fn] {
				auto exp = false;
				if (m_write_lock == 1 && m_ts_write_lock.compare_exchange_weak(exp, true)) {
					fn();
					m_ts_write_lock = false;
					return true;
				}
				return false;
			});
			return r.value_or(false);
		}
		template <class R>
		std::optional<R> TryReadLockToCallTS(std::function<R()> fn) {
			return TryReadLockToCallBase(fn);
		}
		bool TryReadLockToCallTS(std::function<void()> fn) {
			return TryReadLockToCallBase(fn);
		}
		template <class R>
		R ReadLockToCallTS(std::function<R()> fn) {
			return ReadLockToCallBase(fn);

		}
		template <class R>
		R WriteLockToCallTS(std::function<R()> fn) {
			std::optional<bool> r = TryWriteLockToCallBase<bool>([this, fn] {
				auto exp = false;
				if (m_write_lock == 1 && m_ts_write_lock.compare_exchange_weak(exp, true)) {
					fn();
					m_ts_write_lock = false;
					return true;
				}
				return false;
			});
			return r.value_or(false);
		}
		template <class R>
		std::optional<R> TryWriteLockToCallApp(std::function<R()> fn) {
			return TryWriteLockToCallBase([this, fn]() {
				if (this->m_ts_write_lock)throw _T("Illegal operation.Deadlock when the function continues processing as it is.");
				auto r = fn();
				m_ts_write_lock = false;
				return r;
			});
		}
		bool TryWriteLockToCallApp(std::function<void()> fn) {
			return TryWriteLockToCallBase([this, fn]() {
				if (this->m_ts_write_lock)throw _T("Illegal operation.Deadlock when the function continues processing as it is.");
				fn();
			});
		}
		template <class R>
		std::optional<R> TryReadLockToCallApp(std::function<R()> fn) {
			return TryReadLockToCallBase(fn);
		}
		bool TryReadLockToCallApp(std::function<void()> fn) {
			return TryReadLockToCallBase(fn);
		}
		template <class R>
		R ReadLockToCallApp(std::function<R()> fn) {
			return ReadLockToCallBase(fn);
		}
		template <class R>
		R WriteLockToCallApp(std::function<R()> fn) {
			return WriteLockToCallBase<R>([this, fn]() {
				if (this->m_ts_write_lock)throw _T("Locking TS");
				return fn();
			});
		}
		void ReadLockToCallApp(std::function<void()> fn) {
			return ReadLockToCallBase(fn);
		}
		void WriteLockToCallApp(std::function<void()> fn) {
			return WriteLockToCallBase([this, fn]() {
				if (this->m_ts_write_lock)throw _T("Locking TS");
				fn();
			});
		}
	private:
		template <class R>
		std::optional<R> TryWriteLockToCallBase(std::function<R()> fn) {
			auto l = std::unique_lock <tignear::stdex::shared_recursive_mutex >(m_lock, std::try_to_lock);
			if (l) {
				m_write_lock++;
				m_read_lock++;
				auto r = fn();
				m_read_lock--;
				m_write_lock--;
				return r;
			}
			else {
				return std::nullopt;
			}
		}
		bool TryWriteLockToCallBase(std::function<void()> fn) {
			auto r = std::unique_lock <tignear::stdex::shared_recursive_mutex >(m_lock, std::try_to_lock);
			if (r) {
				m_write_lock++;
				m_read_lock++;
				fn();
				m_read_lock--;
				m_write_lock--;
				return true;
			}
			else {
				return false;
			}
		}
		template <class R>
		std::optional<R> TryReadLockToCallBase(std::function<R()> fn) {
			auto r = std::shared_lock <tignear::stdex::shared_recursive_mutex >(m_lock, std::try_to_lock);
			if (r) {
				m_read_lock++;
				auto r = fn();
				m_read_lock--;
				return r;
			}
			else {
				return std::nullopt;
			}
		}
		bool TryReadLockToCallBase(std::function<void()> fn) {
			auto r = std::shared_lock <tignear::stdex::shared_recursive_mutex >(m_lock, std::try_to_lock);
			if (r) {
				m_read_lock++;
				fn();
				m_read_lock--;
				return true;
			}
			else {
				return false;
			}

		}
		template <class R>
		R ReadLockToCallBase(std::function<R()> fn) {
			std::shared_lock lock(m_lock);
			m_read_lock++;
			auto r = fn();
			m_read_lock--;
			return r;
		}
		void ReadLockToCallBase(std::function<void()> fn) {
			std::shared_lock lock(m_lock);
			m_read_lock++;
			fn();
			m_read_lock--;
		}
		template <class R>
		R WriteLockToCallBase(std::function<R()> fn) {
			std::lock_guard lock(m_lock);
			m_write_lock++;
			m_read_lock++;
			auto r = fn();
			m_read_lock--;
			m_write_lock--;
			return r;
		}
		void WriteLockToCallBase(std::function<void()> fn) {
			std::lock_guard lock(m_lock);
			m_write_lock++;
			m_read_lock++;
			fn();
			m_read_lock--;
			m_write_lock--;
		}
		//コピー不可
		void operator =(const TextStoreLock& src) {}
		TextStoreLock(const TextStoreLock& src) {}
		tignear::stdex::shared_recursive_mutex m_lock;
		std::atomic<unsigned int> m_write_lock;
		std::atomic<unsigned int> m_read_lock;
		std::atomic<bool> m_ts_write_lock;
	};
}