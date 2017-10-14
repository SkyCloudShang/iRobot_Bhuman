//option(Demo)
//{
//  common_transition
//  {
//    switch(libDemo.demoGameState)
//      {
//      case LibDemo::wavingPlinking:
//        goto waving;
//      case LibDemo::normal:
//        goto normal;
//      default:
//        ASSERT(false);
//        break;
//    }
//  }
//
//  initial_state(normal)
//  {
//    action
//    {
//      Striker();
//      LookForward();
//    }
//  }
//
//  state(waving)
//  {
//    action
//    {
//      LookForward();
//      Waving();
//    }
//  }
//}

option(Demo)
{    
  initial_state(normalDoOwnRole)
  {
      transition
      {
          if(state_time>1000)
              goto judge;
      }
      action
      {
         LookForward();
         Stand();
      }
  }
  
  
  state(judge)
  {
    transition
    {
         if(theRobotInfo.number==1) 
         {
             goto keeper;
         }
         else if(theRobotInfo.number==2)
         {
             goto striker;
         }
         else if(theRobotInfo.number==5)
         {
             goto supporter;
         }
         else
         {
             goto stand;
         }
    }
  }

  state(keeper)
  {
      action
      {
         // Keeper();
          Robot1();
      }
  }
  
  state(striker)
  {
      action
      {
          //Striker();
          Robot2();
      }
  }
  
  state(supporter)
  {
      action
      {
          Stand();
          //Supporter();
      }
  }
  
  state(stand)
  {
      action
      {
          LookForward();
          Stand();
      }
  }
  
}