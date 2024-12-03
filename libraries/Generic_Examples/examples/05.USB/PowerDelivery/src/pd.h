// clang-format off
#include <stdint.h>
#include "include/ch5xx.h"
// clang-format on

// Check power capability with ADC value from CC1/CC2
#if defined(CH549)
#define DefaultPowerMin (342 - 137)
#define DefaultPowerMax (342 + 158)
#define Power1_5AMin (771 - 198)
#define Power1_5AMax (771 + 180)
#define Power3_0AMin (1383 - 310)
#define Power3_0AMax (1383 + 288)
#elif defined(CH552)
#define DefaultPowerMin ((342 - 137) >> 4)
#define DefaultPowerMax ((342 + 158) >> 4)
#define Power1_5AMin ((771 - 198) >> 4)
#define Power1_5AMax ((771 + 180) >> 4)
#define Power3_0AMin ((1383 - 310) >> 4)
#define Power3_0AMax ((1383 + 288) >> 4)
#endif

// size of RcvDataBuf
#define RcvBufMaxLen 73
// size of SndDataBuf
#define SndBufMaxLen 73

// Header bit define
#define DataRoleUFP 0
#define DataRoleDFP 1
#define PwrRoleSink 0
#define PwrRoleSource 1

// Message Type : Data Message
#define SourceSendCap 0x01   // 5B:00001
#define GoodCRC 0x01         // 5B:00001
#define SinkSendRequest 0x02 // 5B:00010
#define Accept 0x03          // 5B:00011
#define REJECT 0x04          // 5B:00100
#define SinkCap 0x04         // 5B:00100
#define PS_RDY 0x06          // 5B:00110
#define GetSourceCap 0x07    // 5B:00111
#define GetSinkCap 0x08      // 5B:01000
#define PRSwap 0x0A          // 5B:01010
#define SoftRst 0x0D         // 5B:01101
#define VDef 0x0F

#define VDef 0x0F

// Vendor Define Message Command
#define DiscIdent 0x01
#define DiscSVID 0x02
#define DiscMode 0x03
#define EnterMode 0x04
#define ExitMode 0x05
#define Attention 0x06 // SINK request notification
#define DPStatUpdate 0x10
#define DPConfig 0x11

#define REQ 0x00
#define ACK 0x01

// other non-data 4B5B code
#define Sync1 0x18 // 5B:11000
#define Sync2 0x11 // 5B:10001
#define RST1 0x07  // 5B:00111
#define RST2 0x19  // 5B:11001
#define EOP 0x0D   // 5B:01101
#define Sync3 0x06 // 5B:00110

// Start of Packet Sequences
#define SOP 0    // Start of Packet Sequence(SOP)
#define SOPP 1   // Start of Packet Sequence Prime(SOP')
#define SOPDP 2  // Start of Packet Sequence Double Prime(SOP'')
#define SOPPD 3  // Start of Packet Sequence Prime Double(SOP'_Debug)
#define SOPDPD 4 // Start of Packet Sequence Double Prime Double(SOP''_Debug)

// receive data status
#define REVSUCCESS 0x00 // Get data
#define NODATA 0x01     // No data
#define ILLEGAL 0x02    // Illegal data, might be SOP'

// connection status
#define DFP_PD_CONNECT 0x00
#define DFP_PD_DISCONNECT 0x01

// Header
typedef struct {
  //	Extended	  1		0
  //	NDO			  3
  //	MsgID		  3
  //	PortPwrRole	  1		0:Sink  1:Source
  //	SpecRev		  2		01 Rev2.0
  //	PortDataRole  1		0:UFP  1:DFP
  //	MessageType	  5

  uint8_t MsgType : 5;
  uint8_t PortDataRole : 1;
  uint8_t SpecRev : 2;

  uint8_t PortPwrRole : 1;
  uint8_t MsgID : 3;
  uint8_t NDO : 3;
  uint8_t Extended : 1;

} _Msg_Header_Struct;

// header
typedef union {
  _Msg_Header_Struct HeaderStruct;
  uint8_t HeaderData[2];
} _Union_Header;

// voltage current parsing
typedef struct {
  //	Data	12bit
  //	Volt	10bit		Voltage *0.05V
  //	Curr	10bit		Current *0.01A

  uint16_t Current : 10;
  uint8_t VoltL6 : 6;
  uint8_t VoltH4 : 4;

  uint8_t PeakCurrent : 2;
  uint8_t Reserved : 2;
  uint8_t UnchunkedExtended : 1;
  uint8_t DualRole : 1;
  uint8_t USBCommunicationsCapable : 1;
  uint8_t UnconstrainedPower : 1;
  uint8_t USBSuspendSupported : 1;
  uint8_t DualRolePower : 1;
  uint8_t Fixedsupply : 2;

} _SRC_Cap_Fixed_Supply_Struct;

typedef struct {
  uint8_t MaxCurrent : 7;
  uint8_t Reserved : 1;
  uint8_t MinVoltage : 8;
  uint8_t Reserved2 : 1;
  uint8_t MaxVoltageL7 : 7;
  uint8_t MaxVoltageH1 : 1;
  uint8_t Reserved3 : 2;
  uint8_t PPSPowerLimited : 1;
  uint8_t PPS : 2;
  uint8_t AugmentedPowerDataObj : 2;
} _SRC_Cap_Augmented_Supply_Struct;

// voltage current request
typedef struct {

  uint16_t MaxCurrent : 10;
  uint8_t CurrentL6 : 6;
  uint8_t CurrentH4 : 4;

  uint8_t Reserved : 3;

  uint8_t UnchunkedExtended : 1;
  uint8_t NoUSBSuspend : 1;
  uint8_t USBCommunicationsCapable : 1;
  uint8_t CapabilityMismatch : 1;
  uint8_t GiveBackFlag : 1;

  uint8_t ObjectPosition : 3;

  uint8_t Reserved2 : 1;

} _Sink_Request_Data_Fixed_Struct;

typedef struct {
  uint8_t Current : 7;
  uint8_t Reserved : 1;
  // don't make Reserved go across byte boundary
  uint8_t Reserved1 : 1;
  uint8_t VoltageL7 : 7;
  uint8_t VoltageH4 : 4;
  uint8_t Reserved2 : 3;
  uint8_t UnchunkedExtended : 1;
  uint8_t NoUSBSuspend : 1;
  uint8_t USBCommunicationsCapable : 1;
  uint8_t CapabilityMismatch : 1;
  uint8_t Reserved3 : 1;
  uint8_t ObjectPosition : 3;
  uint8_t Reserved4 : 1;
} _Sink_Request_Data_Programmable_Struct;

typedef union { /* Src Cap */
  _SRC_Cap_Fixed_Supply_Struct SrcCapFixedSupplyStruct;
  _SRC_Cap_Augmented_Supply_Struct SrcCapAugmentedSupplyStruct;
  uint8_t SrcCapData[4];
} _Union_SrcCap;

// VDM Header
typedef struct {
  //	SVID			16		0xFF01: DisplayPort
  //
  //	StructuredVDM	1
  //	SVDMVer			2		00: Ver.1
  //	----			5
  //	CommandType		2		00: REQ		01: ACK
  //	----			1
  //	Command			5

  uint16_t SVID : 16;

  uint16_t Command : 5;
  uint16_t : 1;
  uint16_t CommandType : 2;
  uint16_t ModeIndex : 3;
  uint16_t : 2;
  uint16_t SVDMVer : 2;
  uint16_t StructuredVDM : 1;

} _VDM_Hdr_Struct;
typedef union {
  _VDM_Hdr_Struct VDMHdrStruct;
  uint8_t VDMHdrData[4];
} _Union_VDM_Hdr;

// external reference
// select cc pin: 1:cc1 2:cc2
extern __xdata uint8_t CCSel;
// current SOP type received
extern __xdata uint8_t RecvSop;
// pointer to header
extern __xdata _Union_Header *__data Union_Header;
// pointer to voltage current pair structure
extern __xdata _Union_SrcCap *__data Union_SrcCap;
// VDM header pointer
// extern _Union_VDM_Hdr xdata *Union_VDM_Hdr;

extern uint8_t RcvDataBuf[];
extern uint8_t RcvDataCount;

extern uint8_t SndDataBuf[];
extern uint8_t SndDataCount;

extern __code uint8_t Cvt5B4B[];
extern __code uint8_t Cvt4B5B[];
extern __code uint32_t CRC32_Table[];

// initialize sending header (PD2.0 version, MsgID
// initialized，PortDataRole,PortPwrRole customizable)
void ResetSndHeader();
// send content of SndDataBuf
uint8_t SendHandle();
// check receving，0x00:packet received；0x01:packet not received；0x02:packet
// not valid. Data stored in RcvDataBuf
uint8_t ReceiveHandle();
