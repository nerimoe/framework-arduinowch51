// clang-format off
#include <stdint.h>
#include <stdbool.h>
#include "include/ch5xx.h"
#include "include/ch5xx_usb.h"
#include "USBconstant.h"
#include "USBhandler.h"
// clang-format off

// clang-format off
extern __xdata __at (EP0_ADDR) uint8_t Ep0Buffer[];
extern __xdata __at (EP3_ADDR) uint8_t Ep3Buffer[];
// clang-format on

#define LINE_CODEING_SIZE 7
__xdata uint8_t LineCoding[LINE_CODEING_SIZE] = {
    0x00, 0xe1, 0x00, 0x00,
    0x00, 0x00, 0x08}; // Initialize for baudrate 57600, 1 stopbit, No parity,
                       // eight data bits

volatile __xdata uint8_t USBByteCountEP3 =
    0; // Bytes of received data on USB endpoint
volatile __xdata uint8_t USBBufOutPointEP3 = 0; // Data pointer for fetching

volatile __xdata uint8_t UpPoint3_Busy =
    0; // Flag of whether upload pointer is busy
volatile __xdata uint8_t controlLineState = 0;

__xdata uint8_t usbWritePointer = 0;

typedef void (*pTaskFn)(void);

void delayMicroseconds(__data uint16_t us);

void resetCDCParameters() {

  USBByteCountEP3 = 0; // Bytes of received data on USB endpoint
  UpPoint3_Busy = 0;
}

void setLineCodingHandler() {
  for (__data uint8_t i = 0;
       i < ((LINE_CODEING_SIZE <= USB_RX_LEN) ? LINE_CODEING_SIZE : USB_RX_LEN);
       i++) {
    LineCoding[i] = Ep0Buffer[i];
  }
}

uint16_t getLineCodingHandler() {
  __data uint16_t returnLen;

  returnLen = LINE_CODEING_SIZE;
  for (__data uint8_t i = 0; i < returnLen; i++) {
    Ep0Buffer[i] = LineCoding[i];
  }

  return returnLen;
}

void setControlLineStateHandler() {
  controlLineState = Ep0Buffer[2];

  // We check DTR state to determine if host port is open (bit 0 of lineState).
  if (((controlLineState & 0x01) == 0) &&
      (*((__xdata uint32_t *)LineCoding) ==
       1200)) { // both linecoding and sdcc are little-endian
    /*pTaskFn tasksArr[1];
    USB_CTRL = 0;
    EA = 0; //Disabling all interrupts is required. TMOD = 0; tasksArr[0] =
    (pTaskFn)0x3800; delayMicroseconds(50000); delayMicroseconds(50000);
    (tasksArr[0])( ); //Jump to bootloader code while(1);*/
  }
}

bool USBSerial() {
  __data bool result = false;
  if (controlLineState > 0)
    result = true;
  // delay(10); not doing it for now
  return result;
}

void USBSerial_flush(void) {
  if (!UpPoint3_Busy && usbWritePointer > 0) {
    UEP3_T_LEN = usbWritePointer;
    UEP3_CTRL = UEP3_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_ACK; // Respond ACK
    UpPoint3_Busy = 1;
    usbWritePointer = 0;
  }
}

uint8_t USBSerial_write(__data char c) { // 3 bytes generic pointer
  __data uint16_t waitWriteCount;
  if (controlLineState > 0) {
    while (true) {
      waitWriteCount = 0;
      while (UpPoint3_Busy) { // wait for 250ms or give up, on my mac it takes
                              // about 256us
        waitWriteCount++;
        delayMicroseconds(5);
        if (waitWriteCount >= 50000)
          return 0;
      }
      if (usbWritePointer < MAX_PACKET_SIZE) {
        Ep3Buffer[MAX_PACKET_SIZE + usbWritePointer] = c;
        usbWritePointer++;
        return 1;
      } else {
        USBSerial_flush(); // go back to first while
      }
    }
  }
  return 0;
}

uint8_t
USBSerial_print_n(uint8_t *__xdata buf,
                  __xdata int len) { // 3 bytes generic pointer, not using
                                     // USBSerial_write for a bit efficiency
  __data uint16_t waitWriteCount;
  if (controlLineState > 0) {
    while (len > 0) {
      waitWriteCount = 0;
      while (UpPoint3_Busy) { // wait for 250ms or give up, on my mac it takes
                              // about 256us
        waitWriteCount++;
        delayMicroseconds(5);
        if (waitWriteCount >= 50000)
          return 0;
      }
      while (len > 0) {
        if (usbWritePointer < MAX_PACKET_SIZE) {
          Ep3Buffer[MAX_PACKET_SIZE + usbWritePointer] = *buf++;
          usbWritePointer++;
          len--;
        } else {
          USBSerial_flush(); // go back to first while
          break;
        }
      }
    }
  }
  return 0;
}

uint8_t USBSerial_available() { return USBByteCountEP3; }

char USBSerial_read() {
  if (USBByteCountEP3 == 0)
    return 0;
  __data char data = Ep3Buffer[USBBufOutPointEP3];
  USBBufOutPointEP3++;
  USBByteCountEP3--;
  if (USBByteCountEP3 == 0) {
    UEP3_CTRL = UEP3_CTRL & ~MASK_UEP_R_RES | UEP_R_RES_ACK;
  }
  return data;
}

void USB_EP3_IN() {
  UEP3_T_LEN = 0; // No data to send anymore
  UEP3_CTRL =
      UEP3_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_NAK; // Respond NAK by default
  UpPoint3_Busy = 0;                               // Clear busy flag
}

void USB_EP3_OUT() {
  if (U_TOG_OK) // Discard unsynchronized packets
  {
    USBByteCountEP3 = USB_RX_LEN;
    USBBufOutPointEP3 = 0; // Reset Data pointer for fetching
    if (USBByteCountEP3)
      UEP3_CTRL = UEP3_CTRL & ~MASK_UEP_R_RES |
                  UEP_R_RES_NAK; // Respond NAK after a packet. Let main code
                                 // change response after handling.
  }
}
