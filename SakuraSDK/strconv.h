#ifndef STRCONV_H
#define STRCONV_H

#include <windows.h>
#include <string>
#include <vector>
#pragma warning(disable:4505)

//https://qiita.com/javacommons/items/9ea0c8fd43b61b01a8da
//�����ς��Ďg�p
#if __cplusplus >= 201103L || _MSC_VER >=1500
static inline std::wstring cp_to_wide(const std::string &s, UINT codepage)
{
	int in_length = (int)s.length();
	int out_length = MultiByteToWideChar(codepage, 0, s.c_str(), in_length, 0, 0);
	std::wstring result(out_length, L'\0');
	if (out_length) MultiByteToWideChar(codepage, 0, s.c_str(), in_length, &result[0], out_length);
	return result;
}
static inline std::string wide_to_cp(const std::wstring &s, UINT codepage)
{
	int in_length = (int)s.length();
	int out_length = WideCharToMultiByte(codepage, 0, s.c_str(), in_length, 0, 0, 0, 0);
	std::string result(out_length, '\0');
	if (out_length) WideCharToMultiByte(codepage, 0, s.c_str(), in_length, &result[0], out_length, 0, 0);
	return result;
}
static inline std::wstring cp_to_wide(const std::string &s, UINT codepage, std::string::size_type in_length)
{
	auto out_length = MultiByteToWideChar(codepage, 0, s.c_str(), static_cast<int>(in_length), 0, 0);
	std::wstring result(out_length, L'\0');
	if (out_length) MultiByteToWideChar(codepage, 0, s.c_str(), static_cast<int>(in_length), &result[0], out_length);
	return result;
}
static inline std::string wide_to_cp(const std::wstring &s, UINT codepage, std::wstring::size_type in_length)
{
	auto out_length = WideCharToMultiByte(codepage, 0, s.c_str(), static_cast<int>(in_length), 0, 0, 0, 0);
	std::string result(out_length, '\0');
	if (out_length) WideCharToMultiByte(codepage, 0, s.c_str(), static_cast<int>(in_length), &result[0], out_length, 0, 0);
	return result;
}
static inline void cp_to_wide(const std::string &s, UINT codepage, std::string::size_type in_length,std::wstring& out)
{
	auto out_length = MultiByteToWideChar(codepage, 0, s.c_str(), static_cast<int>(in_length), 0, 0);
	out.assign(out_length, L'\0');
	if (out_length) MultiByteToWideChar(codepage, 0, s.c_str(), static_cast<int>(in_length), &out[0], out_length);
}
static inline void wide_to_cp(const std::wstring &s, UINT codepage, std::wstring::size_type in_length, std::string& out)
{
	auto out_length = WideCharToMultiByte(codepage, 0, s.c_str(), static_cast<int>(in_length), 0, 0, 0, 0);
	out.assign(out_length, '\0');
	if (out_length) WideCharToMultiByte(codepage, 0, s.c_str(), static_cast<int>(in_length), &out[0], out_length, 0, 0);
}
#else /* __cplusplus < 201103L */
static inline std::wstring cp_to_wide(const std::string &s, UINT codepage)
{
	int in_length = (int)s.length();
	int out_length = MultiByteToWideChar(codepage, 0, s.c_str(), in_length, 0, 0);
	std::vector<wchar_t> buffer(out_length);
	if (out_length) MultiByteToWideChar(codepage, 0, s.c_str(), in_length, &buffer[0], out_length);
	std::wstring result(buffer.begin(), buffer.end());
	return result;
}
static inline std::string wide_to_cp(const std::wstring &s, UINT codepage)
{
	int in_length = (int)s.length();
	int out_length = WideCharToMultiByte(codepage, 0, s.c_str(), in_length, 0, 0, 0, 0);
	std::vector<char> buffer(out_length);
	if (out_length) WideCharToMultiByte(codepage, 0, s.c_str(), in_length, &buffer[0], out_length, 0, 0);
	std::string result(buffer.begin(), buffer.end());
	return result;
}
#endif
static inline std::string cp_to_utf8(const std::string &s, UINT codepage)
{
	if (codepage == CP_UTF8) return s;
	std::wstring wide = cp_to_wide(s, codepage);
	return wide_to_cp(wide, CP_UTF8);
}
static inline std::string utf8_to_cp(const std::string &s, UINT codepage)
{
	if (codepage == CP_UTF8) return s;
	std::wstring wide = cp_to_wide(s, CP_UTF8);
	return wide_to_cp(wide, codepage);
}

static std::wstring ansi_to_wide(const std::string &s)
{
	return cp_to_wide(s, CP_ACP);
}
static std::string wide_to_ansi(const std::wstring &s)
{
	return wide_to_cp(s, CP_ACP);
}

static std::wstring sjis_to_wide(const std::string &s)
{
	return cp_to_wide(s, 932);
}
static std::string wide_to_sjis(const std::wstring &s)
{
	return wide_to_cp(s, 932);
}

static std::wstring utf8_to_wide(const std::string &s)
{
	return cp_to_wide(s, CP_UTF8);
}
static std::string wide_to_utf8(const std::wstring &s)
{
	return wide_to_cp(s, CP_UTF8);
}

static std::string ansi_to_utf8(const std::string &s)
{
	return cp_to_utf8(s, CP_ACP);
}
static std::string utf8_to_ansi(const std::string &s)
{
	return utf8_to_cp(s, CP_ACP);
}

static std::string sjis_to_utf8(const std::string &s)
{
	return cp_to_utf8(s, 932);
}
static std::string utf8_to_sjis(const std::string &s)
{
	return utf8_to_cp(s, 932);
}
#pragma warning(default:4505)

#endif /* STRCONV_H */