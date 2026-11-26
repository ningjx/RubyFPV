#pragma once
#include "../menu_objects.h"
#include "../widgets/menu_item_select.h"
#include "../widgets/menu_item_slider.h"
#include "../widgets/menu_item_range.h"
#include "../widgets/menu_item_checkbox.h"


class MenuVehicleFunctions: public Menu
{
   public:
      MenuVehicleFunctions();
      virtual ~MenuVehicleFunctions();
      virtual void valuesToUI();
      virtual void Render();
      virtual void onReturnFromChild(int iChildMenuId, int returnValue);
      virtual void onSelectItem();
            
   private:
      void sendParams();

      MenuItemSlider* m_pItemsSlider[4];
      MenuItemSelect* m_pItemsSelect[14];
      MenuItemRange* m_pItemsRange[14];
};