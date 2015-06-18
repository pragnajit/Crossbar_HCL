/******************************************************************************
 * -- Copyright(c) HCL Technologies Ltd.
 * -- All Rights Reserved
 * --
 * -- THIS IS PROPRIETARY INFORMATION OF HCL Technologies LTD.
 * -- The Copyright notice above does not evidence any
 * -- actual or intended publication of such information
 * --------------------------------------------------------------------------
 * File            : RegFile.h
 * Date            : 27-Jun-2005
 * Purpose         : RegFile class definition.
******************************************************************************/ 
#ifndef REGFILE_H
#define REGFILE_H
#include <string>
#include <vector>
using namespace std;
#include "systemc.h"
#include "RF_TypeDefs.h"
#include "RegField.h"


class RegFile 
{
  protected:
    string regfile_name;
    static const UINT NUM_ACCESS_TYPES_READ = 9;
    static const UINT NUM_ACCESS_TYPES_WRITE = 7;
    static const UINT NUM_INPUT_PER_FIELD = 8;   
   
    vector<UINT>* pRegFieldVectors; // array of vector which store field indices                      
    vector<UINT> ResetFieldIndexVector;
    vector<UINT> ResetValuesVector;
    
    UINT* pRegArray;         // Main register array
    UINT num_of_reg;         // Num of Registers in RegFile
    UINT num_of_field;       // Num of Fields in RegFile

    RegFile **pBaseRegFile;     //Array of Base RegFile Pointers
    RegField **pRegFieldArray;  //Array of Reg and Field containing attributes and events
    UINT *pRegFieldIndexArray;  //array storing the index in pRegFieldArray of every reg and field
    
    UINT _read(UINT _address);                  // read functions
    UINT WO_read(UINT _address); 

    UINT RC_read(UINT _address);    

    void _write(UINT _address, UINT _data);     // write functions
    void RO_write(UINT _address, UINT _data);
    void RW1C_write(UINT _address, UINT _data);
    void RW1S_write(UINT _address, UINT _data);
    
    typedef UINT (RegFile::*pRF)(UINT);       // definition of read function pointer
    typedef void (RegFile::*pWF)(UINT,UINT);  // definition of write function pointer
   
    typedef void (RF_CURRENT_USER_MODULE::*FnPtr)(void);     //definition of Function Ptr for CallBack Functions
 
    pRF pReadFuncTable[NUM_ACCESS_TYPES_READ];   // read function pointer table                   
    pWF pWriteFuncTable[NUM_ACCESS_TYPES_WRITE];  // write function pointer table
   
    // take value & store in variable
    void check_construction();
    UINT _extractBits(UINT _data,UINT _address);
    void _insert(UINT& _data, UINT _insertvalue);
    void setBits(UINT _address, UINT _sbit, UINT _offset);
    void resetBits(UINT _address, UINT _sbit, UINT _offset);

    UINT hw_sw_read(UINT _address, bool _is_hw_read);
    void hw_sw_write(UINT _address, UINT _data, bool _is_hw_write);

    UINT alias_reg_hread(UINT _address);
    UINT alias_reg_sread(UINT _address);
    UINT alias_reg_read(UINT _address, bool _is_hw_read);


  public:
    //Constructor
    RegFile(string _name, UINT _num_of_reg, UINT _num_of_field, const UINT* _regvaluelist, const UINT num_of_rows);
    
    //Destructor
    ~RegFile();                                         
    void reset();
    
    UINT hread(UINT _address);              // read data with HW Access
    UINT sread(UINT _address);              // read data with SW Access
    UINT get(UINT _address);                // read data without restriction 

    void hwrite(UINT _address, UINT _data); // write data with HW Access
    void swrite(UINT _address, UINT _data); // write data with SW Access
    void put(UINT _address, UINT _data);    // write directly, No event generated.
    
    const sc_event& value_changed_event(UINT _address);  // default or value changed event
    const sc_event& data_read_event(UINT _address);      // read event
    const sc_event& data_written_event(UINT _address);   // write event
    const sc_event& data_accessed_event(UINT _address);  // access event (read or write)
 
    // Function for Registering Callback Functions
    void set_callback(void *pUserModule, FnPtr _pCbFn,UINT _address,rf_event_types _event_type);
    
    //Function for setting alias
    void set_alias(UINT _alias_index, RegFile* _real_RegFile, UINT _real_index);
};	

/*******************************************************************************
 * Function        : value_changed_event
 * Parameters      : UINT
         _address  : Field No.
 * Return Value    : sc_event, default event for given field
 * Purpose         : return default event for given field
*******************************************************************************/
inline const sc_event& RegFile::value_changed_event(UINT _address)
{
  UINT _index=pRegFieldIndexArray[_address];
  return pBaseRegFile[_address]->pRegFieldArray[_index]->ValueChangedEvent;
}

/*******************************************************************************
 * Function        : data_read_event
 * Parameters      : UINT
         _address  : Field No.
 * Return Value    : sc_event, read event for given field
 * Purpose         : return read event for given field
*******************************************************************************/
inline const sc_event& RegFile::data_read_event(UINT _address)
{
  UINT _index=pRegFieldIndexArray[_address];
  return pBaseRegFile[_address]->pRegFieldArray[_index]->DataReadEvent;
}

/*******************************************************************************
 * Function        : data_written_event
 * Parameters      : UINT
         _address  : Field No.
 * Return Value    : sc_event, write event for given field
 * Purpose         : return write event for given field
*******************************************************************************/
inline const sc_event& RegFile::data_written_event(UINT _address)
{
  UINT _index=pRegFieldIndexArray[_address];
  return pBaseRegFile[_address]->pRegFieldArray[_index]->DataWrittenEvent;
}

/*******************************************************************************
 * Function        : data_accessed_event
 * Parameters      : UINT
         _address  : Field No.
 * Return Value    : sc_event, Access event for given field
 * Purpose         : return Access event for given field
*******************************************************************************/
inline const sc_event& RegFile::data_accessed_event(UINT _address)
{
  UINT _index=pRegFieldIndexArray[_address];
  return pBaseRegFile[_address]->pRegFieldArray[_index]->DataAccessedEvent;
}
#endif //REGFILE_H


