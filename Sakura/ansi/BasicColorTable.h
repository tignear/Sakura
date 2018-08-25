#pragma once
#include <unordered_map>
namespace tignear::ansi {
	std::unordered_map<unsigned int,uint32_t> BasicSystemColorTable() {
		return {
		{30,0x000000},
		{31,0xff0000},
		{32,0x008000},
		{33,0xffff00},
		{34,0x0000ff},
		{35,0x800080},
		{36,0x00ffff},
		{37,0xffffff},
		{40,0x000000},
		{41,0xff0000},
		{42,0x008000},
		{43,0xffff00},
		{44,0x0000ff},
		{45,0x800080},
		{46,0x00ffff},
		{47,0xffffff},
		{90,0x000000},//TODO begin
		{91,0xff0000},
		{92,0x008000},
		{93,0xffff00},
		{94,0x0000ff},
		{95,0x800080},
		{96,0x00ffff},
		{97,0xffffff},
		{100,0x000000},
		{101,0xff0000},
		{102,0x008000},
		{103,0xffff00},
		{104,0x0000ff},
		{105,0x800080},
		{106,0x00ffff},
		{107,0xffffff}//TODO end
		};
	}
	constexpr int32_t rgb(uint_fast8_t r, uint_fast8_t g, uint_fast8_t b) {
		return r << 16 | g << 8 | b;
	}

	std::unordered_map<unsigned int, uint32_t> Basic256ColorTable() {
		std::unordered_map<unsigned int, uint32_t> r{
		{0,0x000000},
		{1,0x800000},
		{2,0x008000},
		{3,0x808000},
		{4,0x000080},
		{5,0x800080},
		{6,0x008080},
		{7,0xc0c0c0},
		{8,0x808080},
		{9,0xff0000},
		{10,0x00ff00},
		{11,0xffff00},
		{12,0x0000ff},
		{13,0xff00ff},
		{14,0x00ffff},
		{15,0xffffff}
		};
		uint_fast8_t t[]{ 0,95,135,175,215,255 };
		for (uint_fast8_t ri = 0, cnt = 16; ri < std::size(t);ri++) {
			for (uint_fast8_t gi = 0; gi < std::size(t); gi++) {
				for (uint_fast8_t bi = 0; bi < std::size(t); bi++) {
					r[cnt] = rgb(t[ri], t[gi], t[bi]);
					cnt++;
				}
			}
		}
		{
			auto cnt = 232;
			uint_fast8_t e=8;
			for (; cnt <= 255; cnt ++, e += 10) {
				r[cnt] = rgb(e, e, e);
			}
		}

		return r;
	}
}