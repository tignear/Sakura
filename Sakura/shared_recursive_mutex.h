#include <thread>
#include <shared_mutex>
#include <atomic>
#include <condition_variable>
/// <summary>
/// https://stackoverflow.com/questions/36619715/a-shared-recursive-mutex-in-standard-c
/// </summary>
namespace tignear::stdex {
	class shared_recursive_mutex : public std::shared_mutex
	{
	public:
		void lock_shared() {
			std::thread::id this_id = std::this_thread::get_id();
			if (owner == this_id) {
				if (read_count==0) {
					read_owner = this_id;
					read_variable.lock();
				}
				++read_count;
				return;
			}
			shared_mutex::lock_shared();

		}
		void unlock_shared() {
			std::thread::id this_id = std::this_thread::get_id();
			if (read_owner==this_id) {
				--read_count;
				if (read_count == 0) {
					read_owner = std::thread::id();
					read_variable.unlock();
				}
				return;
			}
			shared_mutex::unlock_shared();
		}
		void lock(void) {
			std::thread::id this_id = std::this_thread::get_id();
			if (this_id!=read_owner&&read_count > 0) {
				std::lock_guard lock(read_variable);
			}
			if (owner == this_id) {
				// recursive locking
				++count;
			}
			else {
				// normal locking
				shared_mutex::lock();
				owner = this_id;
				count = 1;
			}
		}
		void unlock(void) {
			if (count > 1) {
				// recursive unlocking
				--count;
			}
			else {
				// normal unlocking
				owner = std::thread::id();
				count = 0;
				shared_mutex::unlock();
			}
		}

	private:
		std::atomic<std::thread::id> owner;
		std::atomic<std::thread::id> read_owner;
		unsigned int read_count=0;
		std::mutex read_variable;
		int count;
	};
}