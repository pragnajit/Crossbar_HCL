/******************************************************************************
 * -- Copyright(c) HCL Technologies Ltd.
 * -- All Rights Reserved
 * --
 * -- THIS IS PROPRIETARY INFORMATION OF HCL Technologies LTD.
 * -- The Copyright notice above does not evidence any
 * -- actual or intended publication of such information
 * --------------------------------------------------------------------------
 * File            : RegFileUser.h
 * Date            : 3-Dec-2005
 * Purpose         : RegFileUser class definition.
******************************************************************************/ 
#ifndef REGFILEUSER_H
#define REGFILEUSER_H
#include <string>
using namespace std;
#include "systemc.h"
#include "RF_TypeDefs.h"

#define RF_HAS_CALLBACK( _UserModuleName ) \
typedef _UserModuleName RF_CURRENT_USER_MODULE

// Macro for registering callback function. Does type casting of function pointer.
#define RF_CALLBACK( _UserRegFileInstance, _CbFn , _address , _event_type ) \
_UserRegFileInstance.set_callback(\
    reinterpret_cast<void *>(this),\
    reinterpret_cast<void (RegFileUser::*)()>(&RF_CURRENT_USER_MODULE::_CbFn),\
    static_cast<UINT>(_address),\
    static_cast<rf_event_types>(_event_type)\
    )

//Macro for setting Alias

#define RF_ALIAS(_aliasRegFileInstance, _aliasIndex, _realRegFilePtr, _realIndex)\
_aliasRegFileInstance.set_alias\
  (\
    static_cast<UINT>(_aliasIndex),\
    _realRegFilePtr,\
    static_cast<UINT>(_realIndex)\
    )

#define REGFILE( UserRegFileClassName , Num_Reg , Num_Fields, RegValueList, Num_of_Rows) \
struct UserRegFileClassName:public RegFileUser\
{\
  UserRegFileClassName(string _name="default_name"):RegFileUser(_name, Num_Reg,Num_Fields,RegValueList,Num_of_Rows){}\
}

class RegFile;
class RegFileUser
{
  private:
    RegFile *RegFilePtr;
  public:
    RegFileUser(string _name, UINT num_reg, UINT num_field, const UINT* _regvaluelist, const UINT num_of_rows);      // constructor
    ~RegFileUser();                                  // destructor   
    void reset();
    const sc_event& value_changed_event(UINT _address); // default or value changed event
    const sc_event& data_read_event(UINT _address);        // read event
    const sc_event& data_written_event(UINT _address);       // write event
    const sc_event& data_accessed_event(UINT _address);      // access event (read or write)
 
    UINT hread(UINT _address);              // read data
    UINT sread(UINT _address);              // read data
    UINT get(UINT _address);                // read data without restriction 
    void hwrite(UINT _address, UINT _data); // write data
    void swrite(UINT _address, UINT _data); // write data
    void put(UINT _address, UINT _data);    // write directly, No event generated. 
    typedef void (RF_CURRENT_USER_MODULE::*FnPtr)();
    // Function for Registering Callback Functions
    void set_callback(void *pUserModule, FnPtr _pCbFn, UINT _address, rf_event_types _event_type);
    void set_alias(UINT _alias_index, RegFileUser* _real_RegFilePtr, UINT _real_index);
};
#endif //REGFILEUSER_H
