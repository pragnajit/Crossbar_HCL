/*******************************************************************************
 * -- Copyright(c) HCL Technologies Ltd.
 * -- All Rights Reserved
 * --
 * -- THIS IS PROPRIETARY INFORMATION OF HCL Technologies LTD.
 * -- The Copyright notice above does not evidence any
 * -- actual or intended publication of such information
 * ---------------------------------------------------------------------
 * File        : RegFileUser.cpp
 * Date        : 3-Dec-2005
 * Purpose     : Definition of member functions of RegFileUser class.
*******************************************************************************/
#include "systemc.h"
#include "RF_TypeDefs.h"
#include "RegFile.h"
#include "RegFileUser.h"

RegFileUser::RegFileUser(string _name, UINT _num_reg,UINT _num_fields,const UINT* _regvaluelist, const UINT _num_of_rows)
{
  RegFilePtr=new RegFile(_name, _num_reg,_num_fields,_regvaluelist, _num_of_rows);
}

RegFileUser::~RegFileUser()
{
  delete RegFilePtr;
}

void RegFileUser::set_alias(UINT _alias_index, RegFileUser* _real_RegFilePtr, UINT _real_index)
{
  RegFilePtr->set_alias(_alias_index,_real_RegFilePtr->RegFilePtr,_real_index);
}

void RegFileUser::reset()
{
  RegFilePtr->reset();
}

const sc_event& RegFileUser::value_changed_event(UINT _address)
{
  return RegFilePtr->value_changed_event(_address);
}

const sc_event& RegFileUser::data_read_event(UINT _address)
{
  return RegFilePtr->data_read_event(_address);
}

const sc_event& RegFileUser::data_written_event(UINT _address)
{
  return RegFilePtr->data_written_event(_address);
}

const sc_event& RegFileUser::data_accessed_event(UINT _address) 
{
  return RegFilePtr->data_accessed_event(_address);
}

UINT RegFileUser::hread(UINT _address)
{
  return RegFilePtr->hread(_address);
}

UINT RegFileUser::sread(UINT _address)
{
  return RegFilePtr->sread(_address);
}

UINT RegFileUser::get(UINT _address)
{
  return RegFilePtr->get(_address);
}

void RegFileUser::hwrite(UINT _address, UINT _data)
{
  RegFilePtr->hwrite(_address,_data);
}

void RegFileUser::swrite(UINT _address, UINT _data)
{
  RegFilePtr->swrite(_address,_data);
}

void RegFileUser::put(UINT _address, UINT _data)
{
  RegFilePtr->put(_address,_data);
}
    
void RegFileUser::set_callback(void *_PtrUserModule, FnPtr _PtrCbFn, UINT _address, rf_event_types _event_type)
{
  RegFilePtr->set_callback
    (
     _PtrUserModule,
     reinterpret_cast<void (RF_CURRENT_USER_MODULE::*)()>(_PtrCbFn),
     _address,
     _event_type
    );
}
