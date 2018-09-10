#pragma once
#include "BasicShellContextAttributeText.h"
#include "ShellContext.h"

namespace tignear::sakura {
	class attrtext_iterator_impl :public ShellContext::attrtext_iterator_innner {
	public:
		explicit attrtext_iterator_impl(std::list<AttributeTextImpl>::iterator elem)  :
			m_elem(elem) {
		}
		void operator++()override;
		attrtext_iterator_impl* operator++(int) override;
		reference operator*()const override;
		pointer operator->()const override;
		bool operator==(const attrtext_iterator_innner& iterator)const override;
		bool operator!=(const attrtext_iterator_innner& iterator)const override;
		attrtext_iterator_impl* clone()const override;
	private:
		attrtext_iterator_impl(const attrtext_iterator_impl&);
		std::list<AttributeTextImpl>::iterator m_elem;
	};
	class BasicShellContextLineText:public ShellContext::attrtext_line {
		std::list<AttributeTextImpl>& Value();
		std::shared_ptr<void> m_resource;

	public:
		BasicShellContextLineText(const ColorTable& ct_sys,const ColorTable& ct_256, const std::vector<std::wstring>& fontmap):
			m_ct_sys(ct_sys),
			m_ct_256(ct_256),
			m_fontmap(fontmap){}
		const std::list<AttributeTextImpl>& Value()const;

		int32_t Remove();
		int32_t RemoveAfter(int32_t p);
		int32_t RemoveBefore(int32_t p);
		ShellContext::attrtext_iterator begin() override;
		ShellContext::attrtext_iterator end() override;
		std::shared_ptr<void>& resource()override;
		bool operator==(const attrtext_line&)const override;
		bool operator!=(const attrtext_line&)const override;

		int32_t Insert(int32_t,const icu::UnicodeString&, const Attribute& attr);
	private:
		std::list<AttributeTextImpl> m_value;
		const ColorTable& m_ct_sys;
		const ColorTable& m_ct_256;
		const std::vector<std::wstring>& m_fontmap;
		static std::list<AttributeTextImpl> empty;
	};
}