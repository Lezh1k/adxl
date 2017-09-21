#include "modbus_rtu_client.h"
#include "modbus_common.h"
#include "heap_memory.h"
#include "commons.h"

#pragma pack(push)
#pragma pack(1)
typedef struct mb_adu {
  uint8_t addr;
  uint8_t fc;
  uint8_t* data;
  uint8_t dataLen;
  uint16_t crc; //should be little-endian
} mb_adu_t;
#pragma pack(pop)

typedef enum diagnostics_sub_code {
  dsc_return_query_data = 0,
  dsc_restart_communications_option,
  dsc_return_diagnostic_register,
  dsc_change_adcii_input_delimiter,
  dsc_force_listen_only_mode,
  dsc_reserved05,
  dsc_reserved06,
  dsc_reserved07,
  dsc_reserved08,
  dsc_reserved09,
  dsc_clean_counter_and_diagnostic_registers,
  dsc_return_bus_messages_count,
  dsc_return_bus_communication_error_count,
  dsc_return_bus_exception_error_count,
  dsc_return_server_messages_count,
  dsc_return_server_no_response_count,
  dsc_return_server_NAK_count,
  dsc_return_server_busy_count,
  dsc_return_bus_character_overrun_count,
  dsc_reserved19,
  dsc_clear_overrun_counter_and_flag,
  dsc_reserved
} diagnostics_sub_code_t;
//

enum {
  coin_state_on = 0xff00,
  coin_state_off = 0x0000
};

static inline uint16_t aduBufferLen(mb_adu_t* adu) {
  return adu->dataLen +
      sizeof(mb_adu_t) -
      sizeof(adu->data) -
      sizeof(adu->dataLen);
}
////////////////////////////////////////////////////////////////////////////

typedef struct mb_counters {
  uint16_t busMsg;       //cpt1 bus message count
  uint16_t busComErr;   //cpt2 bus communication error count
  uint16_t excErr;       //cpt3 slave exception error count
  uint16_t slaveMsg;     //cpt4 slave message count
  uint16_t slaveNoResp; //cpt5 return slave no response count
  uint16_t slaveNAK;     //cpt6 return slave NAK count
  uint16_t slaveBusy;    //cpt7 return slave busy count
  uint16_t busCharOverrrun; //cpt8 return bus character overrun count
} mb_counters_t;
//////////////////////////////////////////////////////////////////////////

typedef struct mb_request_handler {
  uint8_t   fc;
  uint16_t  fcValidationResult;
  uint16_t  (*pfCheckAddress)(mb_adu_t *adu);
  uint16_t  (*pfValidateDataValue)(mb_adu_t *adu);
  uint16_t  (*pfExecuteFunction)(mb_adu_t *adu);
} mb_request_handler_t;

static mb_adu_t aduFromStream(uint8_t *data, uint16_t len);
static uint8_t *aduSerialize(mb_adu_t *adu); //create on heap
static uint16_t mbSendResponse(mb_adu_t *adu);
static void mbSendExcResponse(mbec_exception_code_t exc_code, mb_adu_t *adu);
static mb_request_handler_t* mbValidateFunctionCode(mb_adu_t* adu);
static void handleBroadcastMessage(uint8_t *data, uint16_t len);
//////////////////////////////////////////////////////////////////////////

/*diagnostic handlers*/
static uint16_t diagReturnQueryData(mb_adu_t *adu);
static uint16_t diagRestartCommunicationsOption(mb_adu_t *adu);
static uint16_t diagReturnDiagnosticRegister(mb_adu_t *adu);
static uint16_t diagChangeAdciiInputDelimiter(mb_adu_t *adu);
static uint16_t diagForceListenOnlyMode(mb_adu_t *adu);
static uint16_t diagCleanCounterAndDiagnosticRegisters(mb_adu_t *adu);
static uint16_t diagReturnBusMessagesCount(mb_adu_t *adu);
static uint16_t diagReturnBusCommunicationErrorCount(mb_adu_t *adu);
static uint16_t diagReturnBusExceptionErrorCount(mb_adu_t *adu);
static uint16_t diagReturnServerMessagesCount(mb_adu_t *adu);
static uint16_t diagReturnServerNoResponseCount(mb_adu_t *adu);
static uint16_t diagReturnServerNakCount(mb_adu_t *adu);
static uint16_t diagReturnServerBusyCount(mb_adu_t *adu);
static uint16_t diagReturnBusCharacterOverrunCount(mb_adu_t *adu);
static uint16_t diagClearOverrunCounterAndFlag(mb_adu_t *adu);

typedef uint16_t (*pf_diagnostic_data_t)(mb_adu_t *adu);
static pf_diagnostic_data_t diagnostic_data_handlers[] = {
  diagReturnQueryData, diagRestartCommunicationsOption, diagReturnDiagnosticRegister,
  diagChangeAdciiInputDelimiter, diagForceListenOnlyMode,
  NULL, NULL, NULL, NULL, NULL,
  diagCleanCounterAndDiagnosticRegisters, diagReturnBusMessagesCount,
  diagReturnBusCommunicationErrorCount, diagReturnBusExceptionErrorCount,
  diagReturnServerMessagesCount, diagReturnServerNoResponseCount,
  diagReturnServerNakCount, diagReturnServerBusyCount,
  diagReturnBusCharacterOverrunCount, NULL,
  diagClearOverrunCounterAndFlag
};
/*diagnostic handlers END*/
//////////////////////////////////////////////////////////////////////////

/*check address functions*/
static uint16_t checkDiscreteInputAddress(mb_adu_t *adu);
static uint16_t checkCoilsAddress(mb_adu_t *adu);
static uint16_t checkInputRegistersAddress(mb_adu_t *adu);
static uint16_t checkHoldingRegistersAddress(mb_adu_t *adu);
static uint16_t checkAddressAndReturnOk(); //this is for action functions (not read/write)
/*check address functions END*/

/*check data functions*/
static uint16_t checkReadDiscreteInputData(mb_adu_t *adu);
static uint16_t checkReadCoilsData(mb_adu_t *adu);
static uint16_t checkWriteSingleCoilData(mb_adu_t *adu);
static uint16_t checkWriteMultipleCoilsData(mb_adu_t *adu);
static uint16_t checkReadInputRegistersData(mb_adu_t *adu);
static uint16_t checkWriteSingleRegisterData(mb_adu_t *adu);
static uint16_t checkReadHoldingRegistersData(mb_adu_t *adu);
static uint16_t checkWriteMultipleRegistersData(mb_adu_t *adu);
static uint16_t checkReadWriteMultipleRegistersData(mb_adu_t *adu);
static uint16_t checkMaskWriteRegistersData(mb_adu_t *adu);
static uint16_t checkReadFifoData(mb_adu_t *adu);
static uint16_t checkReadFileRecordData(mb_adu_t *adu);
static uint16_t checkWriteFileRecordData(mb_adu_t *adu);
static uint16_t checkReadExceptionStatusData(mb_adu_t *adu);
static uint16_t checkDiagnosticData(mb_adu_t *adu);
static uint16_t checkGetComEventCounterData(mb_adu_t *adu);
static uint16_t checkGetComEventLogData(mb_adu_t *adu);
static uint16_t checkReportDeviceIdData(mb_adu_t *adu);
static uint16_t checkEncapsulateTpInfoData(mb_adu_t *adu);
/*check data functions end*/

/*STANDARD FUNCTIONS HANDLERS*/

static uint16_t mbReadBits(mb_adu_t *adu, uint8_t *real_addr);
static uint16_t executeReadDiscreteInputs(mb_adu_t *adu);
static uint16_t executeReadCoils(mb_adu_t *adu);

static uint16_t executeWriteSingleCoil(mb_adu_t *adu);
static uint16_t executeWriteMultipleCoils(mb_adu_t *adu);

static uint16_t mbReadRegisters(mb_adu_t *adu, uint16_t *real_addr);
static uint16_t executeReadInputRegisters(mb_adu_t *adu);
static uint16_t executeReadHoldingRegisters(mb_adu_t *adu);
static uint16_t executeWriteSingleRegister(mb_adu_t *adu);
static uint16_t executeWriteMultipleRegisters(mb_adu_t *adu);
static uint16_t executeReadWriteMultipleRegisters(mb_adu_t *adu);
static uint16_t executeMaskWriteRegisters(mb_adu_t *adu);
static uint16_t executeReadFifo(mb_adu_t *adu);
static uint16_t executeReadFileRecord(mb_adu_t *adu);
static uint16_t executeWriteFileRecord(mb_adu_t *adu);
static uint16_t executeReadExceptionStatus(mb_adu_t *adu);
static uint16_t executeDiagnostic(mb_adu_t *adu);
static uint16_t executeGetComEventCounter(mb_adu_t *adu);
static uint16_t executeGetComEventLog(mb_adu_t *adu);
static uint16_t executeReportDeviceId(mb_adu_t *adu);
static uint16_t executeEncapsulateTpInfo(mb_adu_t *adu);
/*STANDARD FUNCTIONS HANDLERS END*/

/*local variables*/

static mb_client_device_t* m_device = NULL;
static mb_counters_t m_counters = {0};
static uint8_t m_exception_status = 0x00; //nothing is happened here.

/*local variables END*/
static inline void clear_counters() {
  m_counters.busCharOverrrun = 0;
  m_counters.busComErr = 0;
  m_counters.busMsg = 0;
  m_counters.excErr = 0;
  m_counters.slaveBusy = 0;
  m_counters.slaveMsg = 0;
  m_counters.slaveNAK = 0;
  m_counters.slaveNoResp = 0;
}
//////////////////////////////////////////////////////////////////////////

void mb_init(mb_client_device_t *dev) {
  m_device = dev;
  clear_counters();
}
////////////////////////////////////////////////////////////////////////////

void handleBroadcastMessage(uint8_t *data, uint16_t len) {
  UNUSED_ARG(data);
  UNUSED_ARG(len);
  //do something. maybe go to silent mode, I don't know
}

static volatile uint8_t is_busy = 0;
uint16_t mb_handle_request(uint8_t *data, uint16_t data_len) {
  uint16_t res = 0x00; //success
  mb_adu_t adu_req;
  mb_request_handler_t *rh = NULL;
  uint16_t expected_crc, real_crc;

  do {
    if (is_busy) {
      ++m_counters.slaveBusy;
      break; //maybe we need to handle this somehow?
    }

    is_busy = 1;
    if (data_len < 3) {
      ++m_counters.busComErr;
      break;
    }

    real_crc = U16_LSBFromStream(data + data_len - 2);
    expected_crc = crc16(data, data_len - 2);

    if (real_crc != expected_crc) {
      ++m_counters.busComErr;
      break;
    }

    ++m_counters.busMsg;

    adu_req = aduFromStream(data, data_len);    
    rh = mbValidateFunctionCode(&adu_req);

    if (adu_req.addr == 0) {
      handleBroadcastMessage(data, data_len);
      ++m_counters.slaveMsg;
      ++m_counters.slaveNoResp;
      break;
    }

    if (adu_req.addr != m_device->address)
      break; //silently.

    m_counters.slaveMsg++;
    if (!rh->fcValidationResult) {
      ++m_counters.excErr;
      mbSendExcResponse(res = mbec_illegal_function, &adu_req);
      break;
    }

    if (!rh->pfCheckAddress(&adu_req)) {
      ++m_counters.excErr;
      mbSendExcResponse(res = mbec_illegal_data_address, &adu_req);
      break;
    }

    if (!rh->pfValidateDataValue(&adu_req)) {
      ++m_counters.excErr;
      mbSendExcResponse(res = mbec_illegal_data_value, &adu_req);
      break;
    }

    if ((res = rh->pfExecuteFunction(&adu_req))) {
      ++m_counters.excErr;
      mbSendExcResponse(res, &adu_req);
      break;
    }

    res = mbSendResponse(&adu_req);
  } while(0);  

  is_busy = 0;
  return res;
}
////////////////////////////////////////////////////////////////////////////

uint16_t checkReadDiscreteInputData(mb_adu_t *adu) {
  uint16_t address = U16_MSBFromStream(adu->data);
  uint16_t quantity = U16_MSBFromStream(adu->data+2);
  uint16_t bl = nearestMultipleOf8(quantity) / 8;

  return (quantity >= 1 && quantity <= 0x07d0) &&
      (bl + address / 8 < m_device->inputDiscreteMap.endAddr &&
       address / 8 >= m_device->inputDiscreteMap.startAddr);
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkReadCoilsData(mb_adu_t *adu) {
  uint16_t address = U16_MSBFromStream(adu->data);
  uint16_t quantity = U16_MSBFromStream(adu->data+2);
  uint16_t bn = nearestMultipleOf8(quantity) / 8;

  return (quantity >= 1 && quantity <= 0x07d0) &&
      (bn + address / 8 < m_device->coilsMap.endAddr &&
       address >= m_device->coilsMap.startAddr);
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkWriteSingleCoilData(mb_adu_t *adu) {
  uint16_t address = U16_MSBFromStream(adu->data);
  uint16_t coil_state = U16_MSBFromStream(adu->data+2);
  if (coil_state != coin_state_off && coil_state != coin_state_on)
    return 0u;

  return address / 8 >= m_device->coilsMap.startAddr &&
      address / 8 < m_device->coilsMap.endAddr;
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkWriteMultipleCoilsData(mb_adu_t *adu) {
  uint16_t address = U16_MSBFromStream(adu->data);
  uint16_t quantity = U16_MSBFromStream(adu->data + 2);
  uint8_t byte_count = *(adu->data + 4);

  return (quantity >= 1 && quantity <= 0x07d0) &&
      (byte_count == nearestMultipleOf8(quantity) / 8) &&
      (address / 8 >= m_device->coilsMap.startAddr &&
       address / 8 + byte_count < m_device->coilsMap.endAddr);
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkReadInputRegistersData(mb_adu_t *adu) {
  uint16_t address = U16_MSBFromStream(adu->data);
  uint16_t quantity = U16_MSBFromStream(adu->data+2);

  return (quantity >= 1 && quantity <= 0x007d) &&
      (address >= m_device->inputRegistersMap.start_addr) &&
      (quantity + address < m_device->inputRegistersMap.end_addr);
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkReadHoldingRegistersData(mb_adu_t *adu) {
  uint16_t address = U16_MSBFromStream(adu->data);
  uint16_t quantity = U16_MSBFromStream(adu->data+2);

  return (quantity >= 1 && quantity <= 0x007d) &&
      (address >= m_device->holdingRegistersMap.start_addr &&
       quantity + address < m_device->holdingRegistersMap.end_addr);
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkWriteSingleRegisterData(mb_adu_t *adu) {
  uint16_t address = U16_MSBFromStream(adu->data);
  return address >= m_device->holdingRegistersMap.start_addr &&
      address < m_device->holdingRegistersMap.end_addr;
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkWriteMultipleRegistersData(mb_adu_t *adu) {
  uint16_t address = U16_MSBFromStream(adu->data);
  uint16_t quantity = U16_MSBFromStream(adu->data + 2);
  uint8_t byte_count = *(adu->data + 4);

  return quantity >= 1 &&
      quantity <= 0x0079 &&
      byte_count == quantity * 2 &&
      address >= m_device->holdingRegistersMap.start_addr &&
      address + quantity < m_device->holdingRegistersMap.end_addr;
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkReadWriteMultipleRegistersData(mb_adu_t *adu) {
  uint16_t read_start_addr = U16_MSBFromStream(adu->data);
  uint16_t read_quantity = U16_MSBFromStream(adu->data + 2);
  uint16_t write_start_addr = U16_MSBFromStream(adu->data + 4);
  uint16_t write_quantity = U16_MSBFromStream(adu->data + 6);
  uint8_t write_byte_count = *(adu->data + 8);

  return read_quantity >= 1 && read_quantity <= 0x007d &&
      write_quantity >= 1 && write_quantity <= 0x0079 &&
      write_byte_count == write_quantity * 2 &&
      read_start_addr + read_quantity < m_device->holdingRegistersMap.end_addr &&
      write_start_addr + write_quantity < m_device->holdingRegistersMap.end_addr;
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkMaskWriteRegistersData(mb_adu_t *adu) {
  uint16_t address = U16_MSBFromStream(adu->data);
  return address >= m_device->holdingRegistersMap.start_addr &&
      address < m_device->holdingRegistersMap.end_addr;
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkReadFifoData(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  return 0u;
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkReadFileRecordData(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  return 0u;
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkWriteFileRecordData(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  return 0u;
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkReadExceptionStatusData(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  return 1u;
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkDiagnosticData(mb_adu_t *adu) {
  uint16_t sub_function = U16_MSBFromStream(adu->data);
  return sub_function < sizeof(diagnostic_data_handlers) / sizeof(pf_diagnostic_data_t)
      && diagnostic_data_handlers[sub_function];
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkGetComEventCounterData(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  return 1u;
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkGetComEventLogData(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  return 0u;
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkReportDeviceIdData(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  return 1u; //always return 1 because there is no data in request
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkEncapsulateTpInfoData(mb_adu_t *adu) {
  uint8_t mei_type = *(adu->data);
  return mei_type == 0x0d || mei_type == 0x0e;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/*execute functions*/

uint16_t mbReadBits(mb_adu_t *adu, uint8_t *real_addr) {
  register uint16_t bc, rbn;
  register uint8_t i;
  register uint8_t rshift;
  register uint8_t* tmp ;
  uint16_t address = U16_MSBFromStream(adu->data);
  uint16_t quantity = U16_MSBFromStream(adu->data+2);

  bc = nearestMultipleOf8(quantity) / 8;
  adu->dataLen = bc + 1;

  if (!(adu->data = (uint8_t*) hm_malloc(adu->dataLen + 1)))
    return mbec_heap_error;

  adu->data[0] = bc;
  tmp = adu->data + 1;

  rshift = address % 8;
  rbn = address / 8;
  while (bc--) {
    for (i = 0; i < 8; ++i) {
      *tmp >>= 1;
      if (real_addr[rbn] & (0x80 >> rshift))
        *tmp |= 0x80;
      else
        *tmp &= ~0x80;

      if (++rshift != 8) continue;
      ++rbn;
      rshift = 0;
    }
    ++tmp;
  }

  return mbec_OK;
}

uint16_t executeReadDiscreteInputs(mb_adu_t *adu) {
  return mbReadBits(adu, m_device->inputDiscreteMap.realAddr);
}
//////////////////////////////////////////////////////////////////////////

uint16_t executeReadCoils(mb_adu_t *adu) {
  return mbReadBits(adu, m_device->coilsMap.realAddr);
}
//////////////////////////////////////////////////////////////////////////

uint16_t executeWriteSingleCoil(mb_adu_t *adu) {
  uint16_t address = U16_MSBFromStream(adu->data);
  uint16_t coil_state = U16_MSBFromStream(adu->data+2);
  if (coil_state == coin_state_off)
    m_device->coilsMap.realAddr[address / 8] &= ~(0x80 >> address % 8);
  else
    m_device->coilsMap.realAddr[address / 8] |= (0x80 >> address % 8);
  //we don't do anything with adu, should return it as is
  return mbec_OK ;
}
//////////////////////////////////////////////////////////////////////////

uint16_t executeWriteMultipleCoils(mb_adu_t *adu) {
  register int8_t i;
  register uint16_t ba;
  register uint8_t shift;

  uint16_t address = U16_MSBFromStream(adu->data);
  uint16_t quantity = U16_MSBFromStream(adu->data + 2);
  uint8_t byte_count = *(adu->data + 4);
  uint8_t *data = adu->data + 5;

  ba = address / 8;
  shift = address % 8;

  while (byte_count--) {
    for (i = 0; i < 8 && quantity--; ++i) {
      if (*data & 0x01)
        m_device->coilsMap.realAddr[ba] |= (0x80 >> shift);
      else
        m_device->coilsMap.realAddr[ba] &= ~(0x80 >> shift);
      *data >>= 1;

      if (++shift != 8) continue;
      shift = 0;
      ++ba;
    } //for
    ++data;
  } //while

  //we don't do anything with adu, should return it as is
  return mbec_OK;
}
//////////////////////////////////////////////////////////////////////////

uint16_t mbReadRegisters(mb_adu_t *adu,
                           uint16_t *real_addr) {
  int16_t i;
  uint8_t* tmp;
  uint16_t address = U16_MSBFromStream(adu->data);
  uint16_t quantity = U16_MSBFromStream(adu->data + 2);
  adu->dataLen = quantity*sizeof(mb_register) + 1;
  adu->data = (uint8_t*) hm_malloc(adu->dataLen);
  if (!adu->data)
    return mbec_heap_error;

  adu->data[0] = adu->dataLen - 1;
  tmp = adu->data + 1;

  for (i = 0; i < quantity; ++i, tmp += sizeof(mb_register)) {
    U16_MSB2Stream(real_addr[address + i], tmp);
  }
  return mbec_OK;
}

uint16_t executeReadInputRegisters(mb_adu_t *adu) {
  return mbReadRegisters(adu, m_device->inputRegistersMap.real_addr);
}
//////////////////////////////////////////////////////////////////////////

uint16_t executeReadHoldingRegisters(mb_adu_t *adu) {
  return mbReadRegisters(adu, m_device->holdingRegistersMap.real_addr);
}
//////////////////////////////////////////////////////////////////////////

uint16_t executeWriteSingleRegister(mb_adu_t *adu) {
  uint16_t address = U16_MSBFromStream(adu->data);
  uint16_t data = U16_MSBFromStream(adu->data+2);
  m_device->holdingRegistersMap.real_addr[address] = data;
  //we don't do anything with adu, should return it as is
  return mbec_OK;
}
//////////////////////////////////////////////////////////////////////////

uint16_t executeWriteMultipleRegisters(mb_adu_t *adu) {

  uint16_t address = U16_MSBFromStream(adu->data);
  uint16_t quantity = U16_MSBFromStream(adu->data + 2);
  uint8_t byte_count = *(adu->data + 4);
  uint8_t *data = adu->data + 5;

  uint8_t* tmp;
  if (!(adu->data = (uint8_t*) hm_malloc(4)))
    return mbec_heap_error;
  adu->dataLen = 4;

  tmp = (uint8_t*) &m_device->holdingRegistersMap.real_addr[address];
  while (byte_count--)
    *(tmp++) = *(data++);

  U16_MSB2Stream(address, adu->data);
  U16_MSB2Stream(quantity, adu->data+2);
  return mbec_OK;
}
//////////////////////////////////////////////////////////////////////////

uint16_t executeReadWriteMultipleRegisters(mb_adu_t *adu) {
  uint16_t read_start_addr = U16_MSBFromStream(adu->data);
  uint16_t read_quantity = U16_MSBFromStream(adu->data + 2);
  uint16_t write_start_addr = U16_MSBFromStream(adu->data + 4);
  uint8_t write_byte_count = *(adu->data + 8);
  uint8_t *write_data = adu->data + 9;
  uint8_t* tmp;
  uint16_t i;

  adu->dataLen = read_quantity*sizeof(mb_register) + 1;
  adu->data = (uint8_t*) hm_malloc(adu->dataLen);
  if (!adu->data)
    return mbec_heap_error;

  adu->data[0] = adu->dataLen - 1;
  tmp = adu->data + 1;
  for (i = 0; i < read_quantity; ++i, tmp += sizeof(mb_register)) {
    U16_MSB2Stream(m_device->holdingRegistersMap.real_addr[read_start_addr + i], tmp);
  }

  tmp = (uint8_t*) &m_device->holdingRegistersMap.real_addr[write_start_addr];
  while (write_byte_count--)
    *(tmp++) = *(write_data++);

  return mbec_OK;
}
//////////////////////////////////////////////////////////////////////////

//Result = (Current Contents AND And_Mask) OR (Or_Mask AND (NOT And_Mask))
uint16_t executeMaskWriteRegisters(mb_adu_t *adu) {
  uint16_t address = U16_MSBFromStream(adu->data);
  uint16_t and_mask = U16_MSBFromStream(adu->data+2);
  uint16_t or_mask = U16_MSBFromStream(adu->data+4);

  m_device->holdingRegistersMap.real_addr[address] =
      (m_device->holdingRegistersMap.real_addr[address] & and_mask) |
      (or_mask & ~and_mask);
  //we don't do anything with adu, should return it as is
  return mbec_OK;
}
//////////////////////////////////////////////////////////////////////////

uint16_t executeReadFifo(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  return 0u;
}
//////////////////////////////////////////////////////////////////////////

uint16_t executeReadFileRecord(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  return 0u;
}
//////////////////////////////////////////////////////////////////////////

uint16_t executeWriteFileRecord(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  return 0u;
}
//////////////////////////////////////////////////////////////////////////

uint16_t executeReadExceptionStatus(mb_adu_t *adu) {
  adu->dataLen = 1; //exception status
  adu->data = (uint8_t*) hm_malloc(adu->dataLen);
  if (!adu->data)
    return mbec_heap_error;
  *adu->data = m_exception_status;
  return mbec_OK;
}
//////////////////////////////////////////////////////////////////////////

uint16_t diagReturnQueryData(mb_adu_t *adu) {
  UNUSED_ARG(adu); //just return adu as is . is it kind of ping?
  return mbec_OK;
}
////////////////////////////////////////////////////////////////////////////

uint16_t diagRestartCommunicationsOption(mb_adu_t *adu) {
  uint16_t clear_communication_event_log = U16_MSBFromStream(adu->data+2);
  switch (clear_communication_event_log) {
    case 0xff00:
      //todo clear_communication_event_log
      break;
    case 0x0000:
      break;
    default:
      return mbec_illegal_data_value;
  }

  //todo restart communications
  clear_counters();
  return mbec_OK;
}
////////////////////////////////////////////////////////////////////////////

uint16_t diagReturnDiagnosticRegister(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  return mbec_illegal_function;
}
////////////////////////////////////////////////////////////////////////////

uint16_t diagChangeAdciiInputDelimiter(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  return mbec_illegal_function;
}
////////////////////////////////////////////////////////////////////////////

uint16_t diagForceListenOnlyMode(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  return mbec_illegal_function;
}
////////////////////////////////////////////////////////////////////////////

uint16_t diagCleanCounterAndDiagnosticRegisters(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  clear_counters();
  return mbec_OK;
}
////////////////////////////////////////////////////////////////////////////

static inline uint16_t diag_return_some_counter(mb_adu_t *adu, uint16_t val) {
  uint16_t sf = U16_MSBFromStream(adu->data); //sub function
  if (!(adu->data = (uint8_t*) hm_malloc(4)))
    return mbec_heap_error;
  adu->dataLen = 4;
  U16_MSB2Stream(sf, adu->data);
  U16_MSB2Stream(val, adu->data+2);
  return mbec_OK;
}

uint16_t diagReturnBusMessagesCount(mb_adu_t *adu) {
  return diag_return_some_counter(adu, m_counters.busMsg);
}

uint16_t diagReturnBusCommunicationErrorCount(mb_adu_t *adu) {
  return diag_return_some_counter(adu, m_counters.busComErr);
}

uint16_t diagReturnBusExceptionErrorCount(mb_adu_t *adu) {
  return diag_return_some_counter(adu, m_counters.excErr);
}

uint16_t diagReturnServerMessagesCount(mb_adu_t *adu) {
  return diag_return_some_counter(adu, m_counters.slaveMsg);
}

uint16_t diagReturnServerNoResponseCount(mb_adu_t *adu) {
  return diag_return_some_counter(adu, m_counters.slaveNoResp);
}

uint16_t diagReturnServerNakCount(mb_adu_t *adu) {
  return diag_return_some_counter(adu, m_counters.slaveNAK);
}

uint16_t diagReturnServerBusyCount(mb_adu_t *adu) {
  return diag_return_some_counter(adu, m_counters.slaveBusy);
}

uint16_t diagReturnBusCharacterOverrunCount(mb_adu_t *adu) {
  return diag_return_some_counter(adu, m_counters.busCharOverrrun);
}

uint16_t diagClearOverrunCounterAndFlag(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  m_counters.busCharOverrrun = 0;
  return mbec_OK;
}

uint16_t executeDiagnostic(mb_adu_t *adu) {
  uint16_t sub_function = U16_MSBFromStream(adu->data);
  pf_diagnostic_data_t handler = diagnostic_data_handlers[sub_function];
  return handler(adu);
}
//////////////////////////////////////////////////////////////////////////

uint16_t executeGetComEventCounter(mb_adu_t *adu) {
  UNUSED_ARG(adu); //todo implement this later
  return mbec_illegal_function;
}
//////////////////////////////////////////////////////////////////////////

uint16_t executeGetComEventLog(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  return mbec_illegal_function;
}
//////////////////////////////////////////////////////////////////////////

uint16_t executeReportDeviceId(mb_adu_t *adu) {
  adu->dataLen = 2;
  adu->data = (uint8_t*) hm_malloc(adu->dataLen);
  if (!adu->data)
    return mbec_heap_error;

  adu->data[0] = m_device->address; //should be some device specific data. now - nothing.
  adu->data[1] = 0xff; //0x00 -OFF, 0xff - ON. Run indicator status
  return mbec_OK;
}
//////////////////////////////////////////////////////////////////////////

uint16_t executeEncapsulateTpInfo(mb_adu_t *adu) {
  UNUSED_ARG(adu);
  return mbec_illegal_function;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

mb_request_handler_t* mbValidateFunctionCode(mb_adu_t* adu) {
  //maybe it's better to use switch, because this table takes ~ 19*(1+2+3*sizeof(funciton_pointer))B
  enum {fc_is_not_supported = 0, fc_is_supported = 1};

  static mb_request_handler_t handlers[] = {
    {mbfc_read_discrete_input, fc_is_supported, checkDiscreteInputAddress,
     checkReadDiscreteInputData, executeReadDiscreteInputs },

    {mbfc_read_coils, fc_is_supported, checkCoilsAddress,
     checkReadCoilsData, executeReadCoils },

    {mbfc_write_single_coil, fc_is_supported, checkCoilsAddress,
     checkWriteSingleCoilData, executeWriteSingleCoil },

    {mbfc_write_multiple_coils, fc_is_supported, checkCoilsAddress,
     checkWriteMultipleCoilsData, executeWriteMultipleCoils },
    /*rw registers*/

    {mbfc_read_input_registers, fc_is_supported, checkInputRegistersAddress,
     checkReadInputRegistersData, executeReadInputRegisters },

    {mbfc_read_holding_registers, fc_is_supported, checkHoldingRegistersAddress,
     checkReadHoldingRegistersData, executeReadHoldingRegisters },

    {mbfc_write_single_register, fc_is_supported, checkHoldingRegistersAddress,
     checkWriteSingleRegisterData, executeWriteSingleRegister },

    {mbfc_write_multiple_registers, fc_is_supported, checkHoldingRegistersAddress,
     checkWriteMultipleRegistersData, executeWriteMultipleRegisters },

    {mbfc_read_write_multiple_registers, fc_is_not_supported, checkHoldingRegistersAddress,
     checkReadWriteMultipleRegistersData, executeReadWriteMultipleRegisters },

    {mbfc_mask_write_registers, fc_is_supported, checkHoldingRegistersAddress,
     checkMaskWriteRegistersData, executeMaskWriteRegisters },

    /*r fifo*/
    {mbfc_read_fifo, fc_is_not_supported, checkAddressAndReturnOk,
     checkReadFifoData, executeReadFifo },
    /*diagnostic*/

    {mbfc_read_file_record, fc_is_not_supported, checkAddressAndReturnOk,
     checkReadFileRecordData, executeReadFileRecord },

    {mbfc_write_file_record, fc_is_not_supported, checkAddressAndReturnOk,
     checkWriteFileRecordData, executeWriteFileRecord },

    {mbfc_read_exception_status, fc_is_not_supported, checkAddressAndReturnOk,
     checkReadExceptionStatusData, executeReadExceptionStatus },

    {mbfc_diagnostic, fc_is_supported, checkAddressAndReturnOk,
     checkDiagnosticData, executeDiagnostic },

    {mbfc_get_com_event_counter, fc_is_not_supported, checkAddressAndReturnOk,
     checkGetComEventCounterData, executeGetComEventCounter },

    {mbfc_get_com_event_log, fc_is_supported, checkAddressAndReturnOk,
     checkGetComEventLogData, executeGetComEventLog },

    /*misc*/
    {mbfc_report_device_id, fc_is_supported, checkAddressAndReturnOk,
     checkReportDeviceIdData, executeReportDeviceId },

    //strange function. we will support only one parameter : 0x0e
    {mbfc_encapsulate_tp_info, fc_is_supported, checkAddressAndReturnOk,
     checkEncapsulateTpInfoData, executeEncapsulateTpInfo },

    {0xff, fc_is_not_supported, NULL, NULL, NULL} /*UNSUPPORTED FUNCTION HANDLER*/
  }; //handlers table

  mb_request_handler_t* res = handlers;
  for (; res->fc != 0xff; ++res) {
    if (res->fc == adu->fc) break;
  }

  return res;
}
////////////////////////////////////////////////////////////////////////////

uint16_t mbSendResponse(mb_adu_t* adu) {
  uint8_t* send_buff = aduSerialize(adu);
  if (!send_buff) return mbec_heap_error;
  m_device->tp_send(send_buff, aduBufferLen(adu));
  hm_free((memory_t)send_buff);
  return 0u;
}
////////////////////////////////////////////////////////////////////////////

void mbSendExcResponse(mbec_exception_code_t exc_code, mb_adu_t* adu) {
  uint8_t resp[5] = {adu->addr,
                     adu->fc | 0x80,
                     exc_code };
  U16_LSB2Stream(crc16(resp, 3), resp + 3);
  m_device->tp_send(resp, 5);
}
////////////////////////////////////////////////////////////////////////////

mb_adu_t aduFromStream(uint8_t *data, uint16_t len) {
  mb_adu_t result;
  result.addr = *(uint8_t*)data;
  data += sizeof(result.addr);
  result.fc = *data;
  data += sizeof(result.fc);
  result.data = data;
  result.dataLen = len - (sizeof(mb_adu_t) -
                          sizeof(result.data) -
                          sizeof(result.dataLen));
  data += result.dataLen;
  result.crc = U16_LSBFromStream(data);
  return result;
}
////////////////////////////////////////////////////////////////////////////

uint8_t* aduSerialize(mb_adu_t *adu) {
  uint16_t i, crc;
  uint8_t *tmp;
  uint8_t *buffer = (uint8_t*) hm_malloc(aduBufferLen(adu));
  if (!buffer) return NULL;

  tmp = buffer;
  *tmp = adu->addr;
  tmp += sizeof(adu->addr);
  *tmp = adu->fc;
  tmp += sizeof(adu->fc);

  for (i = 0; i < adu->dataLen; ++i, ++tmp) {
    *tmp = adu->data[i];
  }

  crc = crc16(buffer, aduBufferLen(adu) - sizeof(crc));
  U16_LSB2Stream(crc, tmp);
  return buffer;
}
//////////////////////////////////////////////////////////////////////////

static inline uint8_t validRegisterAddr(mb_dev_registers_mapping_t *mapping,
                                          uint16_t addr) {
  return (addr >= mapping->start_addr && addr < mapping->end_addr);
}

static inline uint8_t validBitAddr(mb_dev_bit_mapping_t *mapping,
                                     uint16_t bit_addr) {
  uint16_t n8 = nearestMultipleOf8(bit_addr) / 8;
  return n8 >= mapping->startAddr && n8 < mapping->endAddr;
}

uint16_t checkDiscreteInputAddress(mb_adu_t *adu) {
  uint16_t addr = U16_MSBFromStream(adu->data);
  return m_device && validBitAddr(&m_device->inputDiscreteMap, addr);
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkCoilsAddress(mb_adu_t *adu) {
  uint16_t addr = U16_MSBFromStream(adu->data);
  return m_device && validBitAddr(&m_device->coilsMap, addr);
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkInputRegistersAddress(mb_adu_t *adu) {
  uint16_t addr = U16_MSBFromStream(adu->data);
  return m_device && validRegisterAddr(&m_device->inputRegistersMap, addr);
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkHoldingRegistersAddress(mb_adu_t *adu) {
  uint16_t addr = U16_MSBFromStream(adu->data);
  return m_device && validRegisterAddr(&m_device->holdingRegistersMap, addr);
}
//////////////////////////////////////////////////////////////////////////

uint16_t checkAddressAndReturnOk() {
  return 1u;
}
//////////////////////////////////////////////////////////////////////////
