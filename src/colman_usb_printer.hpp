// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Raw access to USB printer-class devices (Linux and Windows)
/**
   The direct label-printing path (raw TSPL) talks to the printer without a
   print spooler.  This module hides the operating-system interface:

   - Linux: the device nodes @c /dev/usb/lpN of the usblp kernel driver.
   - Windows: the device interfaces of the usbprint.sys driver
     (@c GUID_DEVINTERFACE_USBPRINT), the driver Windows loads for every
     USB printer-class device, with or without a print queue.

   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_USB_PRINTER_HPP
#  define COLMAN_USB_PRINTER_HPP

#  ifndef SYSTEM_QBYTEARRAY
#    include <QByteArray>
#    define SYSTEM_QBYTEARRAY
#  endif
#  ifndef SYSTEM_QLIST
#    include <QList>
#    define SYSTEM_QLIST
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

namespace colman
{
  /// Open raw connection to one USB printer-class device.
  /**
     The device is opened by the constructor and closed by the destructor.
     All operations fail gracefully (return false or -1) when the device
     could not be opened.
  */
  class UsbPrinterDevice
  {
  public:
    /// Opens the device @p path (from list_usb_printer_devices()).
    explicit UsbPrinterDevice(QString const& path);
    /// Closes the device.
    ~UsbPrinterDevice();
    UsbPrinterDevice(UsbPrinterDevice const&) = delete;
    UsbPrinterDevice& operator=(UsbPrinterDevice const&) = delete;

    /// Returns true when the device could be opened.
    bool is_open() const;

    /// Writes @p data and waits until the printer accepted all of it.
    /** Only after this has returned true is it safe to close the device
        without losing the end of the data.
        @param data        bytes to send
        @param timeout_ms  maximum time without progress
        @return true when the printer accepted every byte */
    bool write_all(QByteArray const& data, int timeout_ms);

    /// Writes a short request without waiting for a busy printer.
    /** Fails right away (Linux) or after a short time (Windows) when the
        printer is still busy accepting earlier data.
        @return true when all of @p data was written */
    bool write_request(QByteArray const& data);

    /// Discards input from the printer that has not been read yet.
    /** Not possible on Windows without losing later answers; there it
        does nothing. */
    void discard_input();

    /// Waits for input and returns the last byte received.
    /** @param timeout_ms  maximum time to wait for the first byte
        @return the byte (0–255), or -1 when nothing arrived */
    int read_last_byte(int timeout_ms);

    /// Reads the USB printer-class port status byte (IEEE 1284 bits).
    /** A passive class request that does not enter the data stream.
        @return the status byte, or -1 when the printer does not answer */
    int port_status();

  private:
#  ifdef _WIN32
    void* handle_ = nullptr; ///< device handle (HANDLE), null when closed
#  else
    int fd_ = -1; ///< file descriptor, -1 when closed
#  endif
  };

  /// Lists all connected USB printer-class devices.
  /** @return device paths for UsbPrinterDevice, independent of the
      printer's vendor and model */
  QList<QString> list_usb_printer_devices();

  /// Returns the USB ID of the device @p path as "vvvv:pppp".
  /** Vendor and product ID in lower-case hexadecimal, e.g. "5958:0486".
      @return the ID, or an empty string when it cannot be determined */
  QString usb_printer_id(QString const& path);
} // namespace colman

#endif /* #ifndef COLMAN_USB_PRINTER_HPP */

/* EOF */
