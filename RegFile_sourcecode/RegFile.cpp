/*******************************************************************************
 * -- Copyright(c) HCL Technologies Ltd.
 * -- All Rights Reserved
 * --
 * -- THIS IS PROPRIETARY INFORMATION OF HCL Technologies LTD.
 * -- The Copyright notice above does not evidence any
 * -- actual or intended publication of such information
 * ---------------------------------------------------------------------
 * File        : RegFile.cpp
 * Date        : 27-Jun-2005
 * Author      : Mehul Rathod.  
 * Purpose     : Definition of member functions of RegFile class.
*******************************************************************************/
#include <string>
using namespace std;
#include "systemc.h"
#include "RF_TypeDefs.h"
#include "RegFile.h"
#include "RF_event.h"
/*******************************************************************************
 * Function        : RegFile
 * Parameters      : UINT
 *       _num_of_reg: Num of Reg in RegFile
 * _num_of_field: Num of Fields in RegFile	
 * Return Value    : None
 * Purpose         : Constructor
*******************************************************************************/
RegFile::RegFile(string _name, UINT _num_of_reg, UINT _num_of_field, const UINT* _regvaluelist, const UINT _num_of_rows)
  :regfile_name(_name),
   num_of_reg(_num_of_reg),
   num_of_field(_num_of_field)
{
  UINT _total;
  _total = num_of_reg + num_of_field;

  pRegArray        = new UINT[num_of_reg];            // allocate memory
  pRegFieldVectors = new vector<UINT>[num_of_reg];
  pBaseRegFile     = new RegFile*[_total];
  pRegFieldArray   = new RegField*[_total];
  pRegFieldIndexArray = new UINT[_total];
  if(!pRegArray || !pRegFieldVectors || !pBaseRegFile || !pRegFieldArray || !pRegFieldIndexArray)
  {
    cout<<"ERROR:: REGFILE: "<<regfile_name<<" Memory Allocation Failed\n";
#ifndef CONT_ON_ERROR
    exit(1);
#endif
  }

  memset(pRegArray,0,sizeof(UINT)*_num_of_reg); // initialize memory
  for(UINT i=0; i < _total; i++)
  {
    pBaseRegFile[i] = this;
    pRegFieldArray[i] = new RegField(i);
    pRegFieldIndexArray[i]=i;
    if(!pRegFieldArray[i])
    {
      cout<<"ERROR:: REGFILE: "<<regfile_name<<" Memory Allocation Failed\n";
#ifndef CONT_ON_ERROR
      exit(1);
#endif
    }
  }  
 
  pReadFuncTable[ RF_ACC_TYPE_RO ]  = &RegFile::_read;       // RO
  pReadFuncTable[ RF_ACC_TYPE_WO ]  = &RegFile::WO_read;     // WO
  pReadFuncTable[ RF_ACC_TYPE_RW ]  = &RegFile::_read;       // RW
  pReadFuncTable[ RF_ACC_TYPE_RW1C] = &RegFile::_read;       // RW1C
  pReadFuncTable[ RF_ACC_TYPE_RW1S] = &RegFile::_read;       // RW1S
  pReadFuncTable[ RF_ACC_TYPE_RC ]  = &RegFile::RC_read;     // RC
  pReadFuncTable[RF_ACC_TYPE_RSVD]  = &RegFile::_read;       // RSVD
  pReadFuncTable[RF_ACC_TYPE_ALIAS_HW]  = &RegFile::alias_reg_hread;       // Alias Reg special read HW
  pReadFuncTable[RF_ACC_TYPE_ALIAS_SW]  = &RegFile::alias_reg_sread;       // Alias Reg special read SW

  pWriteFuncTable[ RF_ACC_TYPE_RO ] = &RegFile::RO_write;    // RO
  pWriteFuncTable[ RF_ACC_TYPE_WO ] = &RegFile::_write;      // WO
  pWriteFuncTable[ RF_ACC_TYPE_RW ] = &RegFile::_write;      // RW
  pWriteFuncTable[ RF_ACC_TYPE_RW1C]= &RegFile::RW1C_write;  // RW1C
  pWriteFuncTable[ RF_ACC_TYPE_RW1S]= &RegFile::RW1S_write;  // RW1S
  pWriteFuncTable[ RF_ACC_TYPE_RC ] = &RegFile::RO_write;    // RC
  pWriteFuncTable[RF_ACC_TYPE_RSVD] = &RegFile::RO_write;    // RSVD
  
  for(UINT src_index=0; src_index < (_num_of_rows*NUM_INPUT_PER_FIELD); )  
  {
    UINT _index         = _regvaluelist[src_index++];
    if(_index > _total)
    {
      cout<<"ERROR:: REGFILE: "<<regfile_name<<": Index out of range of Reg/Field indices at Reg/Field: "<< _index<<endl;
      exit(1);
    }
    pRegFieldArray[_index]->BaseReg    = _regvaluelist[src_index++];
    pRegFieldArray[_index]->StartBit   = _regvaluelist[src_index++];
    pRegFieldArray[_index]->StopBit    = _regvaluelist[src_index++];
    pRegFieldArray[_index]->HType      = static_cast<rf_access_types> (_regvaluelist[src_index++]);
    pRegFieldArray[_index]->SType      = static_cast<rf_access_types> (_regvaluelist[src_index++]);
    pRegFieldArray[_index]->IsDeltaDelay = _regvaluelist[src_index++];
    if(_regvaluelist[src_index]!=0)
    {
      ResetFieldIndexVector.push_back(_index);
      ResetValuesVector.push_back(_regvaluelist[src_index++]);
    }
    else
    {
      src_index++;
    }
  }
  // link register with field
  for(UINT field_index = num_of_reg; 
              field_index < num_of_reg+num_of_field; field_index++)
  {
    pRegFieldVectors[pRegFieldArray[field_index]->BaseReg].push_back(field_index); 
  }
  check_construction();
}

/*******************************************************************************
 * Function        : check_construction
 * Parameters      : None
 * Return Value    : None
 * Purpose         : To check if regfile has been constructed properly  
*******************************************************************************/
void RegFile::check_construction()
{
  for(UINT reg_field_index=0; reg_field_index < num_of_reg + num_of_field; reg_field_index++)
  {
    if(pRegFieldArray[reg_field_index]->BaseReg>=num_of_reg)
    {
      cout<<"ERROR:: REGFILE: "<<regfile_name<<": BaseReg Index is out of range of Register Indices at Reg/Field: "
	<< reg_field_index << endl;
#ifndef CONT_ON_ERROR
      exit(1);
#endif
    }
    if(pRegFieldArray[reg_field_index]->StartBit > 31)
    {
      cout<< "ERROR:: REGFILE: "<<regfile_name<<": Invalid StartBit in Field/Reg: "<<reg_field_index<<endl;
#ifndef CONT_ON_ERROR
      exit(1);
#endif
    }
    if(pRegFieldArray[reg_field_index]->StopBit > 31)
    {
      cout<< "ERROR:: REGFILE: "<<regfile_name<<": Invalid StopBit in Field/Reg: "<<reg_field_index<<endl;
#ifndef CONT_ON_ERROR
      exit(1);
#endif
    }
    if(pRegFieldArray[reg_field_index]->StopBit <  pRegFieldArray[reg_field_index]->StartBit)
    {
      cout << "ERROR:: REGFILE: "<<regfile_name<<": Stop Bit is less than Start Bit in Field/Reg: " << reg_field_index
           << endl;
#ifndef CONT_ON_ERROR
      exit(1);
#endif
    }
    
    switch(pRegFieldArray[reg_field_index]->HType)
    {
      case RF_ACC_TYPE_RO:
      case RF_ACC_TYPE_RC:
      case RF_ACC_TYPE_RW1C:
	if(
	    pRegFieldArray[reg_field_index]->SType==RF_ACC_TYPE_RO ||
	    pRegFieldArray[reg_field_index]->SType==RF_ACC_TYPE_RC ||
	    pRegFieldArray[reg_field_index]->SType==RF_ACC_TYPE_RW1C
	    )
	{
	  cout<<"ERROR:: REGFILE: "<<regfile_name<<" Illogical HW SW access type combination at Reg/Field: "<<reg_field_index<<endl;
#ifndef CONT_ON_ERROR
	  exit(1);
#endif
	}
	break;
      case RF_ACC_TYPE_RW1S:
	if(
	    pRegFieldArray[reg_field_index]->SType==RF_ACC_TYPE_RW1S
	    )
	{
	  cout<<"ERROR:: REGFILE: "<<regfile_name<<" Illogical HW SW access type combination at Reg/Field: "<<reg_field_index<<endl;
#ifndef CONT_ON_ERROR
	  exit(1);
#endif
	}
	break;
      case RF_ACC_TYPE_WO:
	if(pRegFieldArray[reg_field_index]->SType==RF_ACC_TYPE_WO)
	{
	  cout<<"ERROR:: REGFILE: "<<regfile_name<<" Illogical HW SW access type combination at Reg/Field: "<<reg_field_index<<endl;
#ifndef CONT_ON_ERROR
	  exit(1);
#endif
	}
	break;
      default:
	;
    }
    if(
	(pRegFieldArray[reg_field_index]->SType==RF_ACC_TYPE_RW1S && pRegFieldArray[reg_field_index]->HType==RF_ACC_TYPE_RO) ||
	(pRegFieldArray[reg_field_index]->HType==RF_ACC_TYPE_RW1S && pRegFieldArray[reg_field_index]->SType==RF_ACC_TYPE_RO)
	)
    {
      cout<<"ERROR:: REGFILE: "<<regfile_name<<" Illogical HW SW access type combination at Reg/Field: "<<reg_field_index<<endl;
#ifndef CONT_ON_ERROR
      exit(1);
#endif
    }
    
    
    if(reg_field_index<num_of_reg)    //register index
    {
      bool reg_bits[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
      //register defined, no fields defined. ERROR
      if(pRegFieldVectors[reg_field_index].size()==0 && 
	  (pRegFieldArray[reg_field_index]->HType != RF_ACC_TYPE_RSVD || pRegFieldArray[reg_field_index]->SType != RF_ACC_TYPE_RSVD)
	  )
      {
        cout<<"ERROR:: REGFILE: "<<regfile_name<<": Register: "<<reg_field_index<<" has no fields\n";
#ifndef CONT_ON_ERROR
        exit(1);
#endif
      }
      if(pRegFieldArray[reg_field_index]->StartBit!=0)
      {
        cout<<"ERROR:: REGFILE: "<<regfile_name<<": StartBit of Reg not zero at Register : "<<reg_field_index<<endl;
#ifndef CONT_ON_ERROR
	exit(1);
#endif
      }
      if(pRegFieldArray[reg_field_index]->BaseReg!=reg_field_index)
      {
        cout << "ERROR:: REGFILE: "<<regfile_name<<": Register: "<<reg_field_index<< " does not have BaseReg Index as self index\n";
#ifndef CONT_ON_ERROR
        exit(1);
#endif
      }
      //Conflicting attribute check: Register is R2C. ERROR
      if((pRegFieldArray[reg_field_index]->HType==RF_ACC_TYPE_RC)||
        (pRegFieldArray[reg_field_index]->SType==RF_ACC_TYPE_RC)
        )
      {
         cout<<"ERROR:: REGFILE: "<<regfile_name<<": Register is R2C, not allowed.\n";
         cout<<"  Reg Index  : "<<reg_field_index<<endl;
#ifndef CONT_ON_ERROR
         exit(1);
#endif
      }
      for(UINT field_count=0;field_count<pRegFieldVectors[reg_field_index].size();field_count++)
      {
	UINT field_index=pRegFieldVectors[reg_field_index][field_count];
	UINT field_StartBit=pRegFieldArray[field_index]->StartBit;
	UINT field_StopBit=pRegFieldArray[field_index]->StopBit;
	if(field_StopBit > pRegFieldArray[reg_field_index]->StopBit)
	{
	  cout<<"ERROR:: REGFILE: "<<regfile_name<<": Field Bits lie out of register bits\n";
          cout<<"Register: "<<reg_field_index<<endl;
          cout<<"Field: "<<field_index<<endl;
#ifndef CONT_ON_ERROR
	  exit(1);
#endif
	}
	else
	{  
          for(UINT reg_bit_count=field_StartBit;reg_bit_count<=field_StopBit;reg_bit_count++)
          {
            if(reg_bits[reg_bit_count]==true)
            {
              cout<<"ERROR:: REGFILE: "<<regfile_name<<": Overlapping Fields at Register: "<< reg_field_index<<endl;
#ifndef CONT_ON_ERROR
	      exit(1);
#endif
            }
            else
	    {
              reg_bits[reg_bit_count]=true;
	    }
          }
	}
      	//Conflicting attribute check: Register is WO, fields are not or vice-versa. ERROR
        if((pRegFieldArray[reg_field_index]->HType==RF_ACC_TYPE_WO && pRegFieldArray[field_index]->HType != RF_ACC_TYPE_WO)||
	   (pRegFieldArray[reg_field_index]->HType!=RF_ACC_TYPE_WO && pRegFieldArray[field_index]->HType == RF_ACC_TYPE_WO)||
	   (pRegFieldArray[reg_field_index]->SType==RF_ACC_TYPE_WO && pRegFieldArray[field_index]->SType != RF_ACC_TYPE_WO)||
	   (pRegFieldArray[reg_field_index]->SType==RF_ACC_TYPE_WO && pRegFieldArray[field_index]->SType != RF_ACC_TYPE_WO)
	   )
        {
          cout<<"ERROR:: REGFILE: "<<regfile_name<<": Conflicting Attributes: Register is WO, fields are not.\n";
          cout<<"  Reg Index  : "<<reg_field_index<<endl;
          cout<<"  Field Index: "<<pRegFieldVectors[reg_field_index][field_count]<<endl;
#ifndef CONT_ON_ERROR
          exit(1);
#endif
        }
      }//end of for loop on fields of each register
    }//end of if on registers
  }//end of loop on all fields and registers
}


/*******************************************************************************
 * Function        : set_alias
 * Parameters      : None
 * Return Value    : None
 * Purpose         : To create an alias of Reg/Field present in some other RegFile 
*******************************************************************************/
void RegFile::set_alias(UINT _alias_index, RegFile* _real_RegFile, UINT _real_index)
{
  UINT _alias_StartBit;
  UINT _alias_StopBit;
  UINT _real_StartBit;
  UINT _real_StopBit;

  UINT _alias_size;
  UINT _real_size;
  
  _alias_StartBit = pRegFieldArray[_alias_index]->StartBit;
  _alias_StopBit  = pRegFieldArray[_alias_index]->StopBit;
  _real_StartBit  = _real_RegFile->pRegFieldArray[_real_index]->StartBit;
  _real_StopBit   = _real_RegFile->pRegFieldArray[_real_index]->StopBit;
  _alias_size     = _alias_StopBit- _alias_StartBit; 
  _real_size      = _real_StopBit - _real_StartBit; 
  
  if(_alias_size != _real_size)
  {
    cout<<"ERROR:: REGFILE: "<<regfile_name<<": Size not matching between Real and Alias Reg/Field: "<<_alias_index<<endl;
#ifndef CONT_ON_ERROR
    exit(1);
#endif
  }
  else
  {
    if(_alias_index < num_of_reg && _real_index < _real_RegFile->num_of_reg)    //Alias = Reg , Real = Reg EXACT MAP
    {
      UINT _alias_field_index;
      UINT _real_field_index;
      UINT _alias_field_StartBit;
      UINT _alias_field_StopBit;
      UINT _real_field_StartBit;
      UINT _real_field_StopBit;
      bool field_match_found;
      if(pRegFieldVectors[_alias_index].size() != _real_RegFile->pRegFieldVectors[_real_index].size())
      {
        cout<<"ERROR:: REGFILE: "<<regfile_name<<": Num of fields not matching between Real and Alias Reg: "<<_alias_index<<endl;
#ifndef CONT_ON_ERROR
        exit(1);
#endif
      }
      for(UINT alias_field_count=0;alias_field_count<pRegFieldVectors[_alias_index].size();alias_field_count++)
      {
	field_match_found=false;
	_alias_field_index = pRegFieldVectors[_alias_index][alias_field_count];
	_alias_field_StartBit = pRegFieldArray[_alias_field_index]->StartBit;
	_alias_field_StopBit  = pRegFieldArray[_alias_field_index]->StopBit;
	for(UINT real_field_count=0;real_field_count<_real_RegFile->pRegFieldVectors[_real_index].size();real_field_count++)
	{
	  _real_field_index = _real_RegFile->pRegFieldVectors[_real_index][real_field_count];
	  _real_field_StartBit = _real_RegFile->pRegFieldArray[_real_field_index]->StartBit;
	  _real_field_StopBit  = _real_RegFile->pRegFieldArray[_real_field_index]->StopBit;
	  if(_alias_field_StartBit==_real_field_StartBit && _alias_field_StopBit==_real_field_StopBit)
	  {
	    field_match_found=true;
            pBaseRegFile[_alias_field_index]=_real_RegFile->pBaseRegFile[_real_field_index];
            pRegFieldIndexArray[_alias_field_index]=_real_field_index;
	  }
	}
	if(!field_match_found)
	{
          cout<<"ERROR:: REGFILE: "<<regfile_name<<": Matching field not found in Real Register for field: "<<_alias_field_index<<endl;
#ifndef CONT_ON_ERROR
          exit(1);
#endif
	}
      }
      pBaseRegFile[_alias_index]=_real_RegFile->pBaseRegFile[_real_index];
      pRegFieldIndexArray[_alias_index]=_real_index;
    }
    
    if(_alias_index < num_of_reg && _real_index >= _real_RegFile->num_of_reg)   //Alias = Reg , Real = Field 
    {
      UINT _alias_field_index;
      UINT _alias_field_size;
      if(pRegFieldVectors[_alias_index].size()!=1)
      {
        cout<<"ERROR:: REGFILE: "<<regfile_name<<": Reg Aliased to Field is not single field: "<<_alias_index<<endl;
#ifndef CONT_ON_ERROR
        exit(1);
#endif
      }
      _alias_field_index = pRegFieldVectors[_alias_index][0];
      _alias_field_size = pRegFieldArray[_alias_field_index]->StopBit - pRegFieldArray[_alias_field_index]->StartBit;
      if(_alias_field_size != _real_size)
      {
        cout<<"ERROR:: REGFILE: "<<regfile_name<<": Alias Field Size not matching Real Reg Size. Alias Field index: "<<_alias_field_index<<endl;
#ifndef CONT_ON_ERROR
        exit(1);
#endif
      }
      pBaseRegFile[_alias_field_index]=_real_RegFile->pBaseRegFile[_real_index];
      pRegFieldIndexArray[_alias_field_index]=_real_index;
      pBaseRegFile[_alias_index]=_real_RegFile->pBaseRegFile[_real_index];
      pRegFieldIndexArray[_alias_index]=_real_index;
    }

    if(_alias_index >= num_of_reg && _real_index < _real_RegFile->num_of_reg)   //Alias = Field , Real = Reg 
    {
      UINT _alias_basereg_index;
      UINT _real_field_index;
      UINT _real_field_size;
      if(_real_RegFile->pRegFieldVectors[_real_index].size()!=1)
      {
        cout<<"ERROR:: REGFILE: "<<regfile_name<<": Field Aliased to Reg that is not single field: "<<_alias_index<<endl;
#ifndef CONT_ON_ERROR
        exit(1);
#endif
      }
      _real_field_index=_real_RegFile->pRegFieldVectors[_real_index][0];
      _real_field_size=_real_RegFile->pRegFieldArray[_real_field_index]->StopBit - 
	                 _real_RegFile->pRegFieldArray[_real_field_index]->StartBit;
      if(_alias_size !=_real_field_size)
      {
        cout<<"ERROR:: REGFILE: "<<regfile_name<<": Size of single field of real Reg not matching that of Alias field: "<<_alias_index<<endl;
#ifndef CONT_ON_ERROR
        exit(1);
#endif
      }
      _alias_basereg_index=pRegFieldArray[_alias_index]->BaseReg;
      pRegFieldArray[_alias_basereg_index]->HType=RF_ACC_TYPE_ALIAS_HW;
      pRegFieldArray[_alias_basereg_index]->SType=RF_ACC_TYPE_ALIAS_SW;
      pBaseRegFile[_alias_index]=_real_RegFile->pBaseRegFile[_real_field_index];
      pRegFieldIndexArray[_alias_index]=_real_field_index;
    }
    if(_alias_index >= num_of_reg && _real_index >= _real_RegFile->num_of_reg)  //Alias = Field , Real = Field 
    {
      cout<<"ERROR:: REGFILE: "<<regfile_name<<": Individual Field to Field aliasing not allowed: "<<_alias_index<<endl;
#ifndef CONT_ON_ERROR
      exit(1);
#endif
    }
  }
}

/*******************************************************************************
 * Function        : ~RegFile
 * Parameters      : None
 * Return Value    : None
 * Purpose         : Destructor 
*******************************************************************************/
RegFile::~RegFile()
{
  delete []pRegArray; 
  delete []pRegFieldVectors; 
  delete []pRegFieldArray;
  delete []pRegFieldIndexArray;
}	

void RegFile::reset()
{
  memset(pRegArray,0,sizeof(UINT)*num_of_reg); // initialize memory
  for(UINT i=0;i < ResetFieldIndexVector.size();i++)
  {
    UINT _reg_field_index;
    UINT _reset_value;
    _reg_field_index = ResetFieldIndexVector[i];
    _reset_value = ResetValuesVector[i];
    put(pRegFieldIndexArray[_reg_field_index], _reset_value);
  }
}

/*******************************************************************************
 * Function        : set_callback
 * Parameters      : Callback Functions Pointer
 * Return Value    : None
 * Purpose         : Register Callback Functions
*******************************************************************************/
void RegFile::set_callback(void* pUserModule, FnPtr _pFunction,UINT _address,rf_event_types _event_type)
{
  UINT reg_field_index;
  reg_field_index=pRegFieldIndexArray[_address];
  switch(_event_type)
  {
    case RF_READ_EVENT:
      pRegFieldArray[reg_field_index]->DataReadEvent.register_callback
	(
	 static_cast<RF_CURRENT_USER_MODULE*>(pUserModule),
	 reinterpret_cast<void (RF_CURRENT_USER_MODULE::*)()>(_pFunction)
	);
      break;
    case RF_WRITE_EVENT:
      pRegFieldArray[reg_field_index]->DataWrittenEvent.register_callback
	(
	 static_cast<RF_CURRENT_USER_MODULE*>(pUserModule),
	 reinterpret_cast<void (RF_CURRENT_USER_MODULE::*)()>(_pFunction)
	);
      break;
    case RF_ACCESS_EVENT:
      pRegFieldArray[reg_field_index]->DataAccessedEvent.register_callback
	(
	 static_cast<RF_CURRENT_USER_MODULE*>(pUserModule),
	 reinterpret_cast<void (RF_CURRENT_USER_MODULE::*)()>(_pFunction)
	);
      break;
    case RF_VALUE_CHANGED_EVENT:
      pRegFieldArray[reg_field_index]->ValueChangedEvent.register_callback
	(
	 static_cast<RF_CURRENT_USER_MODULE*>(pUserModule),
	 reinterpret_cast<void (RF_CURRENT_USER_MODULE::*)()>(_pFunction)
	);
      break;
    default:
      cout<<"ERROR:: RegFile: "<<regfile_name<<": Incorrect Fn. Call\n";
  }
}

UINT RegFile::alias_reg_hread(UINT _address)
{
  return alias_reg_read(_address, true);
}

UINT RegFile::alias_reg_sread(UINT _address)
{
  return alias_reg_read(_address, false);
}
/*******************************************************************************
 * Function        : alias_reg_read
 * Parameters      : UINT, bool
          _address : Register/Field No.
       _is_hw_read : flag indicating hw/sw read
 * Return Value    : UINT,data of corresponding Reg/Field
 * Purpose         : read data from regfile using sw or hw access
*******************************************************************************/
UINT RegFile::alias_reg_read(UINT _address, bool _is_hw_read)
{
  UINT reg_field_index;
  bool field_value_changed=false;
  reg_field_index=pRegFieldIndexArray[_address];
  UINT _data=0;
  // call all read method
  for(UINT field_count = 0;
           field_count < pRegFieldVectors[_address].size(); field_count++)
  {
    UINT field_address;
    UINT field_index;
    UINT field_data;
    UINT field_start_bit;
    UINT field_data_before_read;
    UINT field_data_after_read;
    
    field_address = pRegFieldVectors[_address][field_count];
    field_index= pRegFieldIndexArray[field_address];
    field_data_before_read=  get(field_address);
    // read data from field to perform read to clear function
    if(_is_hw_read)
      field_data=(*this.*pReadFuncTable[pBaseRegFile[_address]->pRegFieldArray[field_index]->HType])(field_address);
    else
      field_data=(*this.*pReadFuncTable[pBaseRegFile[_address]->pRegFieldArray[field_index]->SType])(field_address);
    field_start_bit=pRegFieldArray[field_address]->StartBit;
    _data |= (field_data<<field_start_bit); 

    field_data_after_read=  get(field_address);
    
    // notify events of each field
    pBaseRegFile[_address]->pRegFieldArray[field_index]->DataReadEvent.DoAction();
    pBaseRegFile[_address]->pRegFieldArray[field_index]->DataAccessedEvent.DoAction();
    if(field_data_before_read!=field_data_after_read)  // value changed
    {
      pBaseRegFile[_address]->pRegFieldArray[field_index]->ValueChangedEvent.DoAction();
      field_value_changed=true;
    }
  }
  // notify events of register
  pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->DataReadEvent.DoAction();
  pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->DataAccessedEvent.DoAction();
  if(field_value_changed)  // value changed
    pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->ValueChangedEvent.DoAction();
  return _data;
}

/*******************************************************************************
 * Function        : hw_sw_read
 * Parameters      : UINT,bool
          _address : Register/Field No.
       _is_hw_read : hw/sw read
 * Return Value    : UINT,data of corresponding Reg/Field
 * Purpose         : read data from regfile using sw or hw access
*******************************************************************************/
UINT RegFile::hw_sw_read(UINT _address,bool _is_hw_read)
{
  UINT reg_field_index;
  UINT data_before_read;
  UINT data_after_read;
  UINT _data;

  reg_field_index=pRegFieldIndexArray[_address];

  
  data_before_read=  get(_address);
  // read data from address
  
  if(_is_hw_read)
    _data = (*this.*pReadFuncTable[pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->HType])(_address);
  else
    _data = (*this.*pReadFuncTable[pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->SType])(_address);
  
  if(_address >= num_of_reg)  // Field Address
  {
    UINT base_reg_index;
    base_reg_index = pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->BaseReg;

    data_after_read=  get(_address);

    // notify events of field
    pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->DataReadEvent.DoAction();
    pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->DataAccessedEvent.DoAction();
    if(data_before_read!=data_after_read)  // value changed
      pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->ValueChangedEvent.DoAction();
  
    // notify events for base register
    pBaseRegFile[_address]->pRegFieldArray[base_reg_index]->DataReadEvent.DoAction();
    pBaseRegFile[_address]->pRegFieldArray[base_reg_index]->DataAccessedEvent.DoAction();
    if(data_before_read!=data_after_read)  // value changed
      pBaseRegFile[_address]->pRegFieldArray[base_reg_index]->ValueChangedEvent.DoAction();
  }
  else // Register Address
  {
    // call all read method
    for(UINT field_count = 0;
             field_count < pRegFieldVectors[_address].size(); field_count++)
    {
      UINT field_address;
      UINT field_index;
      UINT field_data;
      UINT field_data_before_read;
      UINT field_data_after_read;
      
      field_address = pRegFieldVectors[_address][field_count];
      field_index= pRegFieldIndexArray[field_address];
      field_data_before_read=  get(field_address);
      // read data from field to perform read to clear function
      if(_is_hw_read)
	   field_data=(*this.*pReadFuncTable[pBaseRegFile[_address]->pRegFieldArray[field_index]->HType])(field_address);
      else
	   field_data=(*this.*pReadFuncTable[pBaseRegFile[_address]->pRegFieldArray[field_index]->SType])(field_address);

      field_data_after_read=  get(field_address);
      
      // notify events of each field
      pBaseRegFile[_address]->pRegFieldArray[field_index]->DataReadEvent.DoAction();
      pBaseRegFile[_address]->pRegFieldArray[field_index]->DataAccessedEvent.DoAction();
      if(field_data_before_read!=field_data_after_read)  // value changed
        pBaseRegFile[_address]->pRegFieldArray[field_index]->ValueChangedEvent.DoAction();
    }
    data_after_read=  (*this.*pReadFuncTable[RF_ACC_TYPE_RW])(reg_field_index);
    // notify events of register
    pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->DataReadEvent.DoAction();
    pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->DataAccessedEvent.DoAction();
    if(data_before_read!=data_after_read)  // value changed
      pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->ValueChangedEvent.DoAction();
  }
  return _data;
}


/*******************************************************************************
 * Function        : hread
 * Parameters      : UINT
         _address  : Register/Field No.
 * Return Value    : UINT,data of corresponding Reg
 * Purpose         : Read data from regfile using H/W access
*******************************************************************************/
UINT RegFile::hread(UINT _address)
{
  return hw_sw_read(_address,true);
}

/*******************************************************************************
 * Function        : sread
 * Parameters      : UINT
         _address  : Register/Field No.
 * Return Value    : UINT,data of corresponding Reg
 * Purpose         : Read data from regfile using S/W access
*******************************************************************************/
UINT RegFile::sread(UINT _address)
{
  return hw_sw_read(_address,false);
}

/*******************************************************************************
 * Function        : get
 * Parameters      : UINT
         _address  : Register/Field No.
 * Return Value    : UINT,data of corresponding Reg
 * Purpose         : Read data from regfile using RW access type.
                     No events are generated.
*******************************************************************************/
UINT RegFile::get(UINT _address)
{
  return (*this.*pReadFuncTable[RF_ACC_TYPE_RW])(_address);
}


/*******************************************************************************
 * Function        : put
 * Parameters      : UINT, UINT
         _address  : Register/Field No.
         _data     : data to be written
 * Return Value    : None
 * Purpose         : Write data to regfile using RW access type.
                     No events are generated.
*******************************************************************************/
void RegFile::put(UINT _address, UINT _data)
{
  (*this.*pWriteFuncTable[RF_ACC_TYPE_RW]) (_address,_data);
}

/*******************************************************************************
 * Function        : hw_sw_write
 * Parameters      : UINT, UINT, bool
         _address  : Register/Field No.
         _data     : data to be written
       is_hw_write : hw/sw write
 * Return Value    : None
 * Purpose         : Write data to regfile using HW/SW access
*******************************************************************************/
void RegFile::hw_sw_write(UINT _address, UINT _data, bool is_hw_write)
{
  UINT reg_field_index;
  UINT data_before_write;
  UINT data_after_write;

  reg_field_index= pRegFieldIndexArray[_address];
  
  data_before_write= get(_address);
  // Field address
  if(_address >= num_of_reg)
  {
    UINT base_reg_index=pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->BaseReg;
    // write to that field 
    if(is_hw_write==true)
      (*this.*pWriteFuncTable[pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->HType]) (_address,_data);
    else
      (*this.*pWriteFuncTable[pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->SType]) (_address,_data);

    data_after_write=get(_address);
    
    // notify events of given field address 
    pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->DataWrittenEvent.DoAction();
    pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->DataAccessedEvent.DoAction(); 
    if(data_before_write!=data_after_write)  // value changed
      pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->ValueChangedEvent.DoAction();
    
    // notify events of base register of field  
    pBaseRegFile[_address]->pRegFieldArray[base_reg_index]->DataWrittenEvent.DoAction();   
    pBaseRegFile[_address]->pRegFieldArray[base_reg_index]->DataAccessedEvent.DoAction();
    if(data_before_write!=data_after_write)	//value changed
      pBaseRegFile[_address]->pRegFieldArray[base_reg_index]->ValueChangedEvent.DoAction();
  }
  else // register address
  {
    // write data according to their field
    for(UINT field_count = 0; 
             field_count < pRegFieldVectors[_address].size(); field_count++)
    {
      UINT field_address;
      UINT field_index;
      UINT field_data_before_write;
      UINT field_data_after_write;
      UINT field_data;              
      
      field_address = pRegFieldVectors[_address][field_count];
      field_index= pRegFieldIndexArray[field_address];
      field_data_before_write=get(field_address);
      // write into field  
      field_data = _extractBits(_data,field_address);
      if(is_hw_write==true)
        (*this.*pWriteFuncTable[pBaseRegFile[field_address]->pRegFieldArray[field_index]->HType])
                                       (field_address,field_data);
      else
        (*this.*pWriteFuncTable[pBaseRegFile[field_address]->pRegFieldArray[field_index]->SType])
                                       (field_address,field_data);

      field_data_after_write=get(field_address);
      // notify events of the field
      pBaseRegFile[field_address]->pRegFieldArray[field_index]->DataWrittenEvent.DoAction();
      pBaseRegFile[field_address]->pRegFieldArray[field_index]->DataAccessedEvent.DoAction();
      if(field_data_before_write!=field_data_after_write)  // value changed
        pBaseRegFile[field_address]->pRegFieldArray[field_index]->ValueChangedEvent.DoAction();
    }
    data_after_write=get(_address);
    // notify events of register
    pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->DataWrittenEvent.DoAction();
    pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->DataAccessedEvent.DoAction();
    if(data_before_write!=data_after_write)  // value of register changed
      pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->ValueChangedEvent.DoAction();
  }
}

/*******************************************************************************
 * Function        : hwrite
 * Parameters      : UINT, UINT
         _address  : Register/Field No.
         _data     : data to be written
 * Return Value    : None
 * Purpose         : Write data to regfile using H/W access type
*******************************************************************************/
void RegFile::hwrite(UINT _address, UINT _data)
{
  hw_sw_write(_address,_data,true);
}

/*******************************************************************************
 * Function        : swrite
 * Parameters      : UINT, UINT
         _address  : Register/Field No.
         _data     : data to be written
 * Return Value    : None
 * Purpose         : Write data to regfile using S/W access type
*******************************************************************************/
void RegFile::swrite(UINT _address, UINT _data)
{
  hw_sw_write(_address,_data,false);
}

/*******************************************************************************
 * Function        : setBits
 * Parameters      : UINT, UINT, UINT
         _address  : Registor No.
         _sbit     : start bit position
         _offset   : No of bit from start bit to be set
 * Return Value    : None
 * Purpose         : set no of bits in regfile
*******************************************************************************/
void RegFile::setBits(UINT _address, UINT _sbit, UINT _offset)
{
  UINT reg_field_index;
  UINT base_reg_index;
  reg_field_index=pRegFieldIndexArray[_address];
  base_reg_index= pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->BaseReg;
  pBaseRegFile[_address]->pRegArray[base_reg_index] = ((~((~0) << _offset)) << _sbit) | pBaseRegFile[_address]->pRegArray[base_reg_index];
}

/*******************************************************************************
 * Function        : resetBits
 * Parameters      : UINT, UINT, UINT
         _address  : Registor No.
         _sbit     : start bit position
         _offset   : No of bit from start bit to be reset
 * Return Value    : None
 * Purpose         : reset no of bits in regfile
*******************************************************************************/
void RegFile::resetBits(UINT _address, UINT _sbit, UINT _offset)
{
  UINT reg_field_index;
  UINT base_reg_index;
  reg_field_index=pRegFieldIndexArray[_address];
  base_reg_index= pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->BaseReg;
  pBaseRegFile[_address]->pRegArray[base_reg_index] = 
                     (~((~((~0) << _offset)) << _sbit)) & pBaseRegFile[_address]->pRegArray[base_reg_index];
}

/*******************************************************************************
 * Function        : _extractBits
 * Parameters      : UINT, UINT, UINT
         _data     : data
         _startBit : start bit position
         _stopBit  : stopbit
 * Return Value    : UINT, Extarct outed  bits
 * Purpose         : Extarct no of bits from Regfile
*******************************************************************************/
inline UINT RegFile::_extractBits(UINT _data,UINT _address)
{
  UINT _startBit;
  UINT _stopBit;
  UINT reg_field_index;
  reg_field_index=pRegFieldIndexArray[_address];
  _startBit=pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->StartBit;
  _stopBit=pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->StopBit;
  UINT _mask = (31 == (_stopBit - _startBit ))?     
	       0xFFFFFFFF:(~((~0)<<(_stopBit - _startBit +1))) << _startBit ;
  return ((_data & _mask) >> _startBit); 
 
}
/*******************************************************************************
 * Function        : WO_read
 * Parameters      : UINT
         _address  : Registor No.
 * Return Value    : UINT, Field value
 * Purpose         : Give warning, when try to read (write only) field 
*******************************************************************************/                               
UINT RegFile::WO_read(UINT _address)
{
  return 0;  
}	

/*******************************************************************************
 * Function        : _read
 * Parameters      : UINT
         _address  : Registor No.
 * Return Value    : UINT, field value
 * Purpose         : Read field
*******************************************************************************/                               
UINT RegFile::_read(UINT _address)
{
  UINT _startBit;
  UINT _stopBit;
  UINT _baseAddress;
  UINT _offset;
  UINT _mask;
  UINT reg_field_index;
  reg_field_index=pRegFieldIndexArray[_address];
  
  _startBit = pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->StartBit;
  _stopBit = pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->StopBit;
  _baseAddress = pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->BaseReg;
  _offset = _stopBit-_startBit+1;
  _mask = (32==_offset)? 0xffffffff: ~((~0)<<_offset);
                                          // problem with shifting 32 times 

  return (_mask & (pBaseRegFile[_address]->pRegArray[_baseAddress] >> _startBit));
}

/*******************************************************************************
 * Function        : RC_read
 * Parameters      : UINT
         _address  : Registor No.
 * Return Value    : UINT, field value
 * Purpose         : Read field, clear that bit
*******************************************************************************/
UINT RegFile::RC_read(UINT _address)
{
  UINT _startBit;
  UINT _stopBit;
  UINT _baseAddress;
  UINT _offset;
  UINT _mask;
  UINT _data;
  UINT reg_field_index;
  reg_field_index= pRegFieldIndexArray[_address];
  
  _startBit = pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->StartBit;
  _stopBit = pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->StopBit;
  _baseAddress = pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->BaseReg;
  _offset = _stopBit-_startBit+1;
  _mask = (32==_offset)? 0xffffffff: ~((~0)<<_offset);
                                          // problem with shifting 32 times

  _data = (_mask & (pBaseRegFile[_address]->pRegArray[_baseAddress] >> _startBit));
  _write(_address, 0);
  return _data;
}

/*******************************************************************************
 * Function        : RO_write
 * Parameters      : UINT, UINT
         _address  : Registor No.
         _data     : data to be written
 * Return Value    : None
 * Purpose         : Give ERROR, when trying to write (read only) field
*******************************************************************************/                               
void RegFile::RO_write(UINT _address, UINT _data)
{
}

/*******************************************************************************
 * Function        : RW1C_write
 * Parameters      : UINT, UINT
         _address  : Registor No.
         _data     : command pattern
 * Return Value    : None
 * Purpose         : clear bit if written as 1.
*******************************************************************************/                               
void RegFile::RW1C_write(UINT _address, UINT _data)
{
  _data = ~_data & _read(_address); 
  _write( _address, _data); 
}


/*******************************************************************************
 * Function        : RW1S_write
 * Parameters      : UINT, UINT
         _address  : Registor No.
         _data     : command pattern
 * Return Value    : None
 * Purpose         : Set bit if written as 1.
*******************************************************************************/
void RegFile::RW1S_write(UINT _address, UINT _data)
{
  _data |= _read(_address);
  _write(_address, _data);
}


/*******************************************************************************
 * Function        : _write
 * Parameters      : UINT, UINT
         _address  : Registor No.
         _data     : command pattern
 * Return Value    : None
 * Purpose         : write data to the field
*******************************************************************************/                               
void RegFile::_write(UINT _address, UINT _data)
{
  UINT reg_field_index;
  UINT _startBit;
  UINT _stopBit;
  UINT _baseAddress;
  UINT _offset;
  UINT _mask;
  
  reg_field_index= pRegFieldIndexArray[_address];
  _startBit=pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->StartBit;
  _stopBit=pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->StopBit;
  _baseAddress=pBaseRegFile[_address]->pRegFieldArray[reg_field_index]->BaseReg;
  _offset = _stopBit-_startBit+1;
  if(32 == _offset)         // Problem with shifting 32 times
  {
    pBaseRegFile[_baseAddress]->pRegArray[_baseAddress] = _data;
  }  
  else
  { 
    _mask = ((~((~0) << _offset)) << _startBit);
    pBaseRegFile[_address]->pRegArray[_baseAddress] = 
             (_mask & (_data << _startBit)) | ((~_mask) & pBaseRegFile[_address]->pRegArray[_baseAddress]);
  }
}


