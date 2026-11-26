#pragma once
#include "../menu_objects.h"

class MenuControllerExpert: public Menu
{
   public:
      MenuControllerExpert();
      virtual void onShow(); 
      virtual void Render();
      virtual void onReturnFromChild(int iChildMenuId, int returnValue);      
      virtual void onSelectItem();

   private:

      int m_IndexVersions;
      int m_IndexReboot;
      int m_iIndexPriorities;


      void addTopInfo();


};