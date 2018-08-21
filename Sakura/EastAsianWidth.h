#pragma once
#include <unicode/unistr.h>
#include <unicode/uchar.h>
#include <unicode/schriter.h>
#include <unicode/locid.h>
#include <unicode/brkiter.h>

namespace tignear::icuex {
	inline unsigned int EastAsianWidth(UChar32 uc) {
		UEastAsianWidth cell_width = (UEastAsianWidth)u_getIntPropertyValue(uc, UCHAR_EAST_ASIAN_WIDTH);
		switch (cell_width)
		{
		case U_EA_WIDE:
		case U_EA_FULLWIDTH:
		case U_EA_AMBIGUOUS:
			return 2;
		case U_EA_NEUTRAL:
		case U_EA_HALFWIDTH:
		case U_EA_NARROW:
			return 1;
		}
		return 1;
	}
	inline uint32_t EastAsianWidth(const icu::UnicodeString& ustr) {
		uint32_t count=0;
		UErrorCode status = U_ZERO_ERROR;
		int32_t previous;
		int32_t current;
		icu::BreakIterator *it = icu::BreakIterator::createCharacterInstance(
			icu::Locale::getDefault(), status
		);
		it->setText(ustr);
		for (
			previous = it->first(), current = it->next();
			current != icu::BreakIterator::DONE;
			previous = current, current = it->next()
			) {
			auto size = current - previous;
			auto count32 = ustr.countChar32(previous, size);
			if (count32 == 1) {
				auto eaw = EastAsianWidth(ustr.char32At(previous));
				count += eaw;
			}
			else {
				count += 2;//書記素クラスタがUTF32で1文字で表されない場合は文字幅2(要検証)
			}
		}
		return count;
	}

}