/******************************************************************************
 * -- Copyright(c) HCL Technologies Ltd.
 * -- All Rights Reserved
 * --
 * -- THIS IS PROPRIETARY INFORMATION OF HCL Technologies LTD.
 * -- The Copyright notice above does not evidence any
 * -- actual or intended publication of such information
 * --------------------------------------------------------------------------
 * File            : RegField.h
 * Date            : 22-Jan-2006
 * Purpose         : RegField class definition. Contains attributes and Events of 
                     Register or Field
******************************************************************************/
#ifndef REG_FIELD_H
#define REG_FIELD_H
#include "RF_TypeDefs.h"
#include "RF_event.h"
class RF_CURRENT_USER_MODULE;
class RegField
{
  public:
    RegField(UINT _BaseReg)
      :BaseReg(_BaseReg),
       StartBit(0),
       StopBit(31),
       HType(RF_ACC_TYPE_RSVD),
       SType(RF_ACC_TYPE_RSVD),
       IsDeltaDelay(false),
       ValueChangedEvent(IsDeltaDelay),
       DataReadEvent(IsDeltaDelay),
       DataWrittenEvent(IsDeltaDelay),
       DataAccessedEvent(IsDeltaDelay)
    {
    }
    ~RegField()
    {
    }
    UINT BaseReg;           // Index of Base Register for fields, index of self for registers.
    UINT StartBit;          // start bit of Reg/field  
    UINT StopBit;           // stop bit of Reg/field
    rf_access_types HType;   // HW Access Type of field e.g. RF_ACC_TYPE_RO 
    rf_access_types SType;   // SW Access Type of field e.g. RF_ACC_TYPE_RO 
    bool IsDeltaDelay;      // Delta delayed notification or not(immediate notification)

    rf_event<RF_CURRENT_USER_MODULE> ValueChangedEvent; // array of(pointer to) pointers to default event or value changed event
    rf_event<RF_CURRENT_USER_MODULE> DataReadEvent;     // array of(pointer to) pointers for read event
    rf_event<RF_CURRENT_USER_MODULE> DataWrittenEvent;  // array of(pointer to) pointers for write event
    rf_event<RF_CURRENT_USER_MODULE> DataAccessedEvent; // array of(pointer to) pointers for access event  
};
#endif
