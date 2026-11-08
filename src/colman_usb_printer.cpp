// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Raw access to USB printer-class devices (Linux and Windows)
/**
   @file
   @author Christoph Pesch
*/

#include "colman_usb_printer.hpp"
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef SYSTEM_WINDOWS_H
#    include <windows.h>
#    define SYSTEM_WINDOWS_H
#  endif
#  ifndef SYSTEM_WINIOCTL_H
#    include <winioctl.h>
#    define SYSTEM_WINIOCTL_H
#  endif
#  ifndef SYSTEM_SETUPAPI_H
#    include <setupapi.h>
#    define SYSTEM_SETUPAPI_H
#  endif
#  ifndef SYSTEM_QREGULAREXPRESSION
#    include <QRegularExpression>
#    define SYSTEM_QREGULAREXPRESSION
#  endif
#  ifndef SYSTEM_VECTOR
#    include <vector>
#    define SYSTEM_VECTOR
#  endif
#else
#  ifndef SYSTEM_QFILE
#    include <QFile>
#    define SYSTEM_QFILE
#  endif
#  ifndef SYSTEM_CERRNO
#    include <cerrno>
#    define SYSTEM_CERRNO
#  endif
#  ifndef SYSTEM_FCNTL
#    include <fcntl.h>
#    define SYSTEM_FCNTL
#  endif
#  ifndef SYSTEM_LINUX_LP
#    include <linux/lp.h>
#    define SYSTEM_LINUX_LP
#  endif
#  ifndef SYSTEM_POLL
#    include <poll.h>
#    define SYSTEM_POLL
#  endif
#  ifndef SYSTEM_SYS_IOCTL
#    include <sys/ioctl.h>
#    define SYSTEM_SYS_IOCTL
#  endif
#  ifndef SYSTEM_UNISTD
#    include <unistd.h>
#    define SYSTEM_UNISTD
#  endif
#endif

#ifdef _WIN32

// ===========================================================================
// Windows: usbprint.sys device interfaces
// ===========================================================================

namespace
{
  /// Device interface class of usbprint.sys (GUID_DEVINTERFACE_USBPRINT).
  GUID const UsbPrintInterfaceGuid = {
      0x28d78fad, 0x5a12, 0x11d1, {0xae, 0x5b, 0x00, 0x00, 0xf8, 0x03, 0xa8, 0xc2}};

  /// usbprint.sys request for the IEEE 1284 port status byte.
  /** IOCTL_USBPRINT_GET_LPT_STATUS from the WDK header usbprint.h. */
  DWORD constexpr IoctlUsbprintGetLptStatus =
      CTL_CODE(FILE_DEVICE_UNKNOWN, 12, METHOD_BUFFERED, FILE_ANY_ACCESS);

  /// Bytes per write request; each request must complete within the
  /// timeout, which mirrors the per-chunk progress check on Linux.
  DWORD constexpr WriteChunkBytes = 4096;

  /// Size of one read request (a full-speed bulk packet).
  DWORD constexpr ReadChunkBytes = 64;

  /// Milliseconds a short request may take before the printer counts as
  /// busy (the Linux version fails immediately in that case).
  int constexpr RequestTimeoutMs = 200;

  /// Milliseconds to wait for the port status answer.
  int constexpr PortStatusTimeoutMs = 500;

  /// Returns the Windows handle stored in @p handle.
  HANDLE to_handle(void* handle)
  {
    return static_cast<HANDLE>(handle);
  }

  /// One overlapped I/O request with its completion event.
  class IoRequest
  {
  public:
    IoRequest()
    {
      overlapped_.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    }

    ~IoRequest()
    {
      if(overlapped_.hEvent) {
        CloseHandle(overlapped_.hEvent);
      }
    }

    IoRequest(IoRequest const&) = delete;
    IoRequest& operator=(IoRequest const&) = delete;

    /// Returns the OVERLAPPED structure to pass to the I/O call.
    OVERLAPPED* overlapped()
    {
      return &overlapped_;
    }

    /// Returns true when the completion event could be created.
    bool valid() const
    {
      return overlapped_.hEvent != nullptr;
    }

    /// Waits for the request started on @p handle.
    /**
     * A request that is still pending after @p timeout_ms is cancelled;
     * the function waits until the cancellation is complete, so that the
     * buffer is no longer in use when it returns.
     * @param handle      device handle
     * @param started     return value of the I/O call
     * @param timeout_ms  maximum waiting time
     * @param transferred receives the number of bytes transferred
     * @return true when the request completed successfully in time
     */
    bool finish(HANDLE handle, BOOL started, int timeout_ms,
                DWORD& transferred)
    {
      transferred = 0;
      if(!started && GetLastError() != ERROR_IO_PENDING) {
        return false;
      }
      if(WaitForSingleObject(overlapped_.hEvent,
                             static_cast<DWORD>(timeout_ms)) != WAIT_OBJECT_0)
      {
        CancelIoEx(handle, &overlapped_);
        GetOverlappedResult(handle, &overlapped_, &transferred, TRUE);
        return false;
      }
      return GetOverlappedResult(handle, &overlapped_, &transferred, FALSE) !=
             0;
    }

  private:
    OVERLAPPED overlapped_ = {}; ///< request state
  };

  /// Writes @p size bytes in one request, waiting at most @p timeout_ms.
  /** @return true when all bytes were accepted in time */
  bool write_once(HANDLE handle, char const* data, DWORD size, int timeout_ms)
  {
    IoRequest request;
    if(!request.valid()) {
      return false;
    }
    BOOL const started =
        WriteFile(handle, data, size, nullptr, request.overlapped());
    DWORD written = 0;
    return request.finish(handle, started, timeout_ms, written) &&
           written == size;
  }

  /// Reads up to ReadChunkBytes into @p buffer, waiting at most
  /// @p timeout_ms.
  /** @return the number of bytes read (0 when nothing arrived) */
  DWORD read_once(HANDLE handle, char* buffer, int timeout_ms)
  {
    IoRequest request;
    if(!request.valid()) {
      return 0;
    }
    BOOL const started =
        ReadFile(handle, buffer, ReadChunkBytes, nullptr, request.overlapped());
    DWORD read = 0;
    if(!request.finish(handle, started, timeout_ms, read)) {
      return 0;
    }
    return read;
  }

} // anonymous namespace

/**
 * Opens the usbprint.sys device interface for reading and writing in
 * overlapped mode, so that every transfer can be given a timeout.
 * Opening fails while another program (e.g. the print spooler with a
 * queue on this port) uses the device.
 * @param path  device interface path from list_usb_printer_devices()
 */
colman::UsbPrinterDevice::UsbPrinterDevice(QString const& path)
{
  HANDLE const handle = CreateFileW(
      reinterpret_cast<wchar_t const*>(path.utf16()),
      GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
      nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
  if(handle != INVALID_HANDLE_VALUE) {
    handle_ = handle;
  }
}

colman::UsbPrinterDevice::~UsbPrinterDevice()
{
  if(handle_) {
    CloseHandle(to_handle(handle_));
  }
}

bool colman::UsbPrinterDevice::is_open() const
{
  return handle_ != nullptr;
}

/**
 * Writes @p data in chunks of WriteChunkBytes.  usbprint.sys completes a
 * write request only when the printer has accepted its bytes, so after the
 * last chunk the device can be closed without losing data.
 * @param data        bytes to send
 * @param timeout_ms  maximum time per chunk
 * @return true when the printer accepted every byte
 */
bool colman::UsbPrinterDevice::write_all(QByteArray const& data,
                                         int timeout_ms)
{
  if(!handle_) {
    return false;
  }
  char const* next = data.constData();
  qsizetype remaining = data.size();
  while(remaining > 0) {
    auto const chunk = static_cast<DWORD>(
        qMin<qsizetype>(remaining, static_cast<qsizetype>(WriteChunkBytes)));
    if(!write_once(to_handle(handle_), next, chunk, timeout_ms)) {
      return false;
    }
    next += chunk;
    remaining -= chunk;
  }
  return true;
}

/**
 * Writes @p data in one request.  When the printer does not accept it
 * within RequestTimeoutMs (e.g. because it is still busy with a job), the
 * request is cancelled and false is returned.
 * @param data  the request bytes
 * @return true when all of @p data was written
 */
bool colman::UsbPrinterDevice::write_request(QByteArray const& data)
{
  if(!handle_) {
    return false;
  }
  return write_once(to_handle(handle_), data.constData(),
                    static_cast<DWORD>(data.size()), RequestTimeoutMs);
}

/**
 * Does nothing on Windows.  Discarding would need read requests that are
 * cancelled when nothing arrives, and a cancelled read request makes the
 * next answer of the printer get lost (measured on the Vevor Y486 behind
 * usbprint.sys: after a cancelled read, every second status answer went
 * missing; without cancelled reads, all arrived).
 */
void colman::UsbPrinterDevice::discard_input()
{
}

/**
 * Issues one read request of ReadChunkBytes and waits up to
 * @p timeout_ms; a printer answer arrives as one packet.  No further read
 * is attempted afterwards, because a read request that has to be cancelled
 * makes the next answer get lost (see discard_input()).
 * @param timeout_ms  maximum time to wait for the answer
 * @return the last byte received, or -1 when nothing arrived
 */
int colman::UsbPrinterDevice::read_last_byte(int timeout_ms)
{
  if(!handle_) {
    return -1;
  }
  char buffer[ReadChunkBytes] = {};
  DWORD const read = read_once(to_handle(handle_), buffer, timeout_ms);
  if(read == 0) {
    return -1;
  }
  return static_cast<unsigned char>(buffer[read - 1]);
}

/**
 * Sends IOCTL_USBPRINT_GET_LPT_STATUS, the class request GET_PORT_STATUS.
 * @return the status byte, or -1 when the printer does not answer
 */
int colman::UsbPrinterDevice::port_status()
{
  if(!handle_) {
    return -1;
  }
  IoRequest request;
  if(!request.valid()) {
    return -1;
  }
  unsigned char status = 0;
  BOOL const started =
      DeviceIoControl(to_handle(handle_), IoctlUsbprintGetLptStatus, nullptr,
                      0, &status, sizeof(status), nullptr,
                      request.overlapped());
  DWORD returned = 0;
  if(!request.finish(to_handle(handle_), started, PortStatusTimeoutMs,
                     returned) ||
     returned != sizeof(status))
  {
    return -1;
  }
  return status;
}

/**
 * Enumerates the present device interfaces of usbprint.sys.  Windows
 * creates one for every connected USB printer-class device, whether or
 * not a print queue has been set up for it.
 * @return the device interface paths
 */
QList<QString> colman::list_usb_printer_devices()
{
  QList<QString> result;
  HDEVINFO const devices =
      SetupDiGetClassDevsW(&UsbPrintInterfaceGuid, nullptr, nullptr,
                           DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if(devices == INVALID_HANDLE_VALUE) {
    return result;
  }
  SP_DEVICE_INTERFACE_DATA interface_data = {};
  interface_data.cbSize = sizeof(interface_data);
  for(DWORD i = 0; SetupDiEnumDeviceInterfaces(
          devices, nullptr, &UsbPrintInterfaceGuid, i, &interface_data);
      ++i)
  {
    DWORD size = 0;
    SetupDiGetDeviceInterfaceDetailW(devices, &interface_data, nullptr, 0,
                                     &size, nullptr);
    if(size < sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W)) {
      continue;
    }
    std::vector<char> buffer(size);
    auto* detail =
        reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(buffer.data());
    detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
    if(SetupDiGetDeviceInterfaceDetailW(devices, &interface_data, detail,
                                        size, nullptr, nullptr))
    {
      result.append(QString::fromWCharArray(detail->DevicePath));
    }
  }
  SetupDiDestroyDeviceInfoList(devices);
  return result;
}

/**
 * The device interface path of a USB device contains its IDs, e.g.
 * @c \\?\usb#vid_5958&pid_0486#...#{28d78fad-...}.
 * @param path  device interface path
 * @return "vvvv:pppp", or an empty string
 */
QString colman::usb_printer_id(QString const& path)
{
  static QRegularExpression const pattern(
      QStringLiteral("vid_([0-9a-f]{4})&pid_([0-9a-f]{4})"));
  auto const match = pattern.match(path.toLower());
  if(!match.hasMatch()) {
    return {};
  }
  return match.captured(1) + ':' + match.captured(2);
}

#else

// ===========================================================================
// Linux: usblp device nodes
// ===========================================================================

namespace
{
  /// Highest number of device nodes the usblp kernel driver registers.
  int constexpr MaxUsblpNodes = 16;

  /// Reads one sysfs attribute of the USB device behind @p device_path.
  /**
     The usblp node @c /dev/usb/lpN is mirrored at
     @c /sys/class/usbmisc/lpN; its @c device link points to the USB
     printer interface, whose parent directory holds the device
     descriptor attributes (@c idVendor, @c idProduct, @c manufacturer,
     @c product, …).

     @param device_path device node, e.g. @c /dev/usb/lp0
     @param name        attribute file name, e.g. @c "idVendor"
     @return the trimmed attribute value, or an empty string
  */
  QString usb_device_attribute(QString const& device_path, char const* name)
  {
    QString const node = device_path.section('/', -1);
    QFile file(QString("/sys/class/usbmisc/%1/device/../%2")
                   .arg(node)
                   .arg(QLatin1String(name)));
    if(!file.open(QIODevice::ReadOnly)) {
      return {};
    }
    return QString::fromUtf8(file.readAll()).trimmed();
  }

  /// Waits until @p fd is ready for @p events.
  /** @return true when ready within @p timeout_ms */
  bool wait_for(int fd, short events, int timeout_ms)
  {
    struct pollfd pfd = {};
    pfd.fd = fd;
    pfd.events = events;
    return ::poll(&pfd, 1, timeout_ms) > 0 && (pfd.revents & events) != 0;
  }

} // anonymous namespace

/**
 * Opens the usblp device node non-blocking.  Opening fails without
 * permission (group @c lp) and while the CUPS usb backend has detached
 * usblp for a running job.
 * @param path  device node, e.g. @c /dev/usb/lp0
 */
colman::UsbPrinterDevice::UsbPrinterDevice(QString const& path)
    : fd_(::open(QFile::encodeName(path).constData(),
                 O_RDWR | O_NONBLOCK | O_CLOEXEC))
{
}

colman::UsbPrinterDevice::~UsbPrinterDevice()
{
  if(fd_ >= 0) {
    ::close(fd_);
  }
}

bool colman::UsbPrinterDevice::is_open() const
{
  return fd_ >= 0;
}

/**
 * The usblp kernel driver transfers writes through a single in-flight URB
 * and aborts it when the device is closed (usblp_release() unlinks the
 * URB) — anything the printer has not yet accepted is then lost.
 * Returning right after the final ::write() therefore truncates the tail
 * of any job larger than the driver's 8 kB buffer, observed as "only the
 * first label of a batch is printed".
 *
 * usblp reports POLLOUT exactly when no write URB is in flight, i.e. when
 * the printer has accepted all data so far.  This function hence polls for
 * POLLOUT both between chunks (buffer full) and once more after the final
 * chunk, so that a subsequent close cannot abort anything.
 *
 * @param data        bytes to send
 * @param timeout_ms  maximum time without progress
 * @return true when the printer accepted every byte
 */
bool colman::UsbPrinterDevice::write_all(QByteArray const& data,
                                         int timeout_ms)
{
  if(fd_ < 0) {
    return false;
  }
  char const* next = data.constData();
  qsizetype remaining = data.size();
  while(remaining > 0) {
    ssize_t const written =
        ::write(fd_, next, static_cast<size_t>(remaining));
    if(written > 0) {
      next += written;
      remaining -= written;
      continue;
    }
    if(written < 0 && (errno == EAGAIN || errno == EINTR) &&
       wait_for(fd_, POLLOUT, timeout_ms))
    {
      continue;
    }
    return false;
  }
  // Drain: wait for the final write URB to complete before the caller
  // closes the device.
  return wait_for(fd_, POLLOUT, timeout_ms);
}

/**
 * Writes @p data with one non-blocking ::write().  While a write URB of an
 * earlier job is still in flight, usblp refuses with EAGAIN and the
 * request fails right away.
 * @param data  the request bytes
 * @return true when all of @p data was written
 */
bool colman::UsbPrinterDevice::write_request(QByteArray const& data)
{
  if(fd_ < 0) {
    return false;
  }
  return ::write(fd_, data.constData(), static_cast<size_t>(data.size())) ==
         static_cast<ssize_t>(data.size());
}

void colman::UsbPrinterDevice::discard_input()
{
  if(fd_ < 0) {
    return;
  }
  unsigned char stale = 0;
  while(::read(fd_, &stale, 1) == 1) {
  }
}

/**
 * Waits up to @p timeout_ms for input, then reads everything available.
 * @param timeout_ms  maximum time to wait for the first byte
 * @return the last byte read, or -1 when nothing arrived
 */
int colman::UsbPrinterDevice::read_last_byte(int timeout_ms)
{
  if(fd_ < 0 || !wait_for(fd_, POLLIN, timeout_ms)) {
    return -1;
  }
  int last = -1;
  unsigned char byte = 0;
  while(::read(fd_, &byte, 1) == 1) {
    last = byte;
  }
  return last;
}

/**
 * Uses the @c LPGETSTATUS ioctl of the usblp kernel driver.
 * @return the status byte, or -1 when the printer does not answer
 */
int colman::UsbPrinterDevice::port_status()
{
  int status = 0;
  if(fd_ < 0 || ::ioctl(fd_, LPGETSTATUS, &status) != 0) {
    return -1;
  }
  return status;
}

/**
 * Every existing @c /dev/usb/lp* node is returned, independent of the
 * printer's vendor and model.  The usblp kernel driver creates one node per
 * connected USB printer-class device.  The nodes disappear temporarily
 * while a CUPS job is being printed, because the CUPS usb backend detaches
 * usblp for the duration of the job.
 * @return the existing device nodes
 */
QList<QString> colman::list_usb_printer_devices()
{
  QList<QString> result;
  for(int i = 0; i < MaxUsblpNodes; ++i) {
    QString const path = QString("/dev/usb/lp%1").arg(i);
    if(QFile::exists(path)) {
      result.append(path);
    }
  }
  return result;
}

/**
 * Reads @c idVendor and @c idProduct from sysfs.
 * @param path  device node, e.g. @c /dev/usb/lp0
 * @return "vvvv:pppp", or an empty string
 */
QString colman::usb_printer_id(QString const& path)
{
  QString const vendor = usb_device_attribute(path, "idVendor");
  QString const product = usb_device_attribute(path, "idProduct");
  if(vendor.isEmpty() || product.isEmpty()) {
    return {};
  }
  return vendor.toLower() + ':' + product.toLower();
}

#endif

/* EOF */
