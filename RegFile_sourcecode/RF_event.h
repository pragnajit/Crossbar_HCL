/*******************************************************************************
 * -- Copyright(c) HCL Technologies Ltd.
 * -- All Rights Reserved
 * --
 * -- THIS IS PROPRIETARY INFORMATION OF HCL Technologies LTD.
 * -- The Copyright notice above does not evidence any
 * -- actual or intended publication of such information
 * ---------------------------------------------------------------------
 * File        : RF_event.h
 * Date        : 3-Dec-2005
 * Purpose     : Declaration and Implementation of specialized events for RegFile
*******************************************************************************/
#ifndef RF_EVENT_H
#define RF_EVENT_H
#include "systemc.h"
#include "RF_TypeDefs.h"
template <class T>
class rf_event:public sc_event
{
  private:
    //Pointer to "ev_notify_imm" or "ev_notify_delayed" or "callback"
    void (rf_event::*pEvCbFn)(void);
    T *PtrUserModule;  //Pointer to User Module
    typedef void (T::*CbFnPtr)(void);
    CbFnPtr pCbFn;  //Pointer to Callback Function
  
    void ev_notify_imm()  //Function for immediate event notification
    {
      notify();
    }
    void ev_notify_delayed()  //Function for Delta Delayed Event Notification
    {
      notify(SC_ZERO_TIME);
    }
    void callback()  //Function for Callback
    {
      (*PtrUserModule.*pCbFn)();
    }
  public:
    rf_event(bool _IsDeltaDelay=0)  //Constructor
    { 
      if(_IsDeltaDelay)
        pEvCbFn=&rf_event::ev_notify_delayed;
      else
        pEvCbFn=&rf_event::ev_notify_imm;
    }
    void register_callback(T* _PtrUserModule, CbFnPtr CbFn)  //Function for registering the callback function
    {
      pEvCbFn=&rf_event::callback;
      pCbFn=CbFn;
      PtrUserModule=_PtrUserModule;
    }
    void DoAction()  //Function to be called in the event of read, write, value change etc..
    {
      (*this.*pEvCbFn)();
    }
};
#endif
