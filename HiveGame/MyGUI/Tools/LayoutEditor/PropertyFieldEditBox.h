/*!
	@file
	@author		Albert Semenov
	@date		12/2010
*/
#ifndef __PROPERTY_FIELD_EDIT_BOX_H__
#define __PROPERTY_FIELD_EDIT_BOX_H__

#include "EditorToolTip.h"
#include "BaseLayout/BaseLayout.h"
#include "IPropertyField.h"

namespace tools
{
	class PropertyFieldEditBox :
		public wraps::BaseLayout,
		public IPropertyField
	{
	public:
		PropertyFieldEditBox(MyGUI::Widget* _parent);
		virtual ~PropertyFieldEditBox();

		virtual void initialise(const std::string& _type);

		virtual void setTarget(MyGUI::Widget* _currentWidget);
		virtual void setValue(const std::string& _value);
		virtual void setName(const std::string& _value);

		virtual void setVisible(bool _value);
		virtual bool getVisible();

		virtual MyGUI::IntSize getContentSize();
		virtual void setCoord(const MyGUI::IntCoord& _coord);

	private:
		void notifyApplyProperties(MyGUI::Widget* _sender, bool _force);
		void notifyTryApplyProperties(MyGUI::EditBox* _sender);
		void notifyForceApplyProperties(MyGUI::EditBox* _widget);

	protected:
		virtual bool onCheckValue();
		virtual void onAction(const std::string& _value, bool _final);

	protected:
		MyGUI::TextBox* mText;
		MyGUI::EditBox* mField;
		MyGUI::Widget* mCurrentWidget;
		std::string mType;
		std::string mName;
	};

} // namespace tools

#endif // __PROPERTY_FIELD_EDIT_BOX_H__
