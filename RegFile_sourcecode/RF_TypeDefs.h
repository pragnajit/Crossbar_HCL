#ifndef RF_TYPEDEFS_H
#define RF_TYPEDEFS_H
typedef unsigned int UINT;
enum rf_event_types
{
  RF_READ_EVENT=0,
  RF_WRITE_EVENT,
  RF_ACCESS_EVENT,
  RF_VALUE_CHANGED_EVENT
};
enum rf_access_types
{ 
  RF_ACC_TYPE_RO=0,  // Read Only
  RF_ACC_TYPE_WO,    // Write Only
  RF_ACC_TYPE_RW,    // Read Write
  RF_ACC_TYPE_RC,    // Read to clear
  RF_ACC_TYPE_RW1C,  // Write 1 to clear
  RF_ACC_TYPE_RW1S,  // Write 1 to set
  RF_ACC_TYPE_RSVD,   // Reserved       
  RF_ACC_TYPE_ALIAS_HW,  // Alias Reg for special read       
  RF_ACC_TYPE_ALIAS_SW   // Alias Reg for special read 
};
#endif
