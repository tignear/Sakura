#pragma once
#include "BasicShellContextAttributeText.h"
namespace tignear::sakura {
	class BasicShellContextLineText{
	public:
		BasicShellContextLineText(const ColorTable& ct_sys,const ColorTable& ct_256):m_ct_sys(ct_sys),m_ct_256(ct_256){}
		const std::list<AttributeTextImpl>& Value()const;
		int32_t Remove();
		int32_t RemoveAfter(int32_t p);
		int32_t RemoveBefore(int32_t p);

		int32_t Insert(int32_t,const icu::UnicodeString&, const Attribute& attr);
	private:
		std::list<AttributeTextImpl> m_value;
		const ColorTable& m_ct_sys;
		const ColorTable& m_ct_256;
		static const std::list<AttributeTextImpl> empty;
	};
}