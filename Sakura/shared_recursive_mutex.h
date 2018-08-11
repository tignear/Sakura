#include <thread>
#include <shared_mutex>
#include <atomic>
/// <summary>
/// https://stackoverflow.com/questions/36619715/a-shared-recursive-mutex-in-standard-c
/// </summary>
namespace tignear::stdex {
	class shared_recursive_mutex : public std::shared_mutex
	{
	public:
		void lock(void) {
			std::thread::id this_id = std::this_thread::get_id();
			if (owner == this_id) {
				// recursive locking
				count++;
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
				count--;
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
		int count;
	};
}