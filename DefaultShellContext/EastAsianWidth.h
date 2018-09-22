#pragma once
#include <unicode/unistr.h>
#include <unicode/uchar.h>
#include <unicode/schriter.h>
#include <unicode/locid.h>
#include <unicode/brkiter.h>

namespace tignear::icuex {
	inline unsigned int EastAsianWidth(UChar uc,unsigned char ambiguous_size) {
		UEastAsianWidth cell_width = (UEastAsianWidth)u_getIntPropertyValue(uc, UCHAR_EAST_ASIAN_WIDTH);
		switch (cell_width)
		{
		case U_EA_WIDE:
		case U_EA_FULLWIDTH:
			return 2;
		case U_EA_NEUTRAL:
		case U_EA_HALFWIDTH:
		case U_EA_NARROW:
			return 1;
		case U_EA_AMBIGUOUS:
			return ambiguous_size;
		}
		return 1;
	}
	inline uint32_t EastAsianWidth(const icu::UnicodeString& ustr,unsigned char ambiguous_size) {
		auto itr=icu::StringCharacterIterator(ustr);
		uint32_t r=0;
		for (UChar uc = itr.first(); uc != itr.DONE; uc = itr.next()) {
			if ((u_getIntPropertyValue(uc, UCHAR_CANONICAL_COMBINING_CLASS) != 0)) {
				itr.next();
				continue;
			}
			r += EastAsianWidth(uc, ambiguous_size);
		}
		return r;
	}

}