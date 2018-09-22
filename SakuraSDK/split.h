#pragma once
#include <string>
namespace tignear::stdex{
	template <class T,class R>
	R split(const std::basic_string<T>& string,std::basic_string_view<T> separator) {
		auto separator_length = separator.length(); // ‹æØ‚è•¶š‚Ì’·‚³

		auto list = R();

		if (separator_length == 0) {
			list.push_back(string);
		}
		else {
			auto offset = std::string::size_type(0);
			while (true) {
				auto pos = string.find(separator, offset);
				if (pos == std::string::npos) {
					list.push_back(string.substr(offset));
					break;
				}
				list.push_back(string.substr(offset, pos - offset));
				offset = pos + separator_length;
			}
		}
		return list;
	}
	template <class T,class R>
	R split(std::basic_string_view<T> string, std::basic_string_view<T> separator) {
		auto separator_length = separator.length(); // ‹æØ‚è•¶š‚Ì’·‚³

		auto list = R();

		if (separator_length == 0) {
			list.push_back(string);
		}
		else {
			auto offset = std::string::size_type(0);
			while (true) {
				auto pos = string.find(separator, offset);
				if (pos == std::string::npos) {
					list.push_back(string.substr(offset));
					break;
				}
				list.push_back(string.substr(offset, pos - offset));
				offset = pos + separator_length;
			}
		}

		return list;
	}
}