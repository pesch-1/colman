package de.colman.colman_client

import android.app.Activity
import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.hardware.usb.UsbConstants
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbDeviceConnection
import android.hardware.usb.UsbEndpoint
import android.hardware.usb.UsbInterface
import android.hardware.usb.UsbManager
import android.os.Build
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel

/**
 * Platform channel that sends raw bytes (a TSPL command stream) to a
 * cable-connected USB thermal label printer via Android's [UsbManager].
 *
 * Kept dependency-free on purpose: USB-host printing is Android-only and the
 * needed surface (enumerate, request permission, bulk-transfer) is small.
 */
class UsbPrinterChannel(private val activity: Activity) :
    MethodChannel.MethodCallHandler {

    companion object {
        const val CHANNEL = "de.colman.colman_client/usb_printer"
        private const val ACTION_USB_PERMISSION =
            "de.colman.colman_client.USB_PERMISSION"
    }

    private val usbManager: UsbManager
        get() = activity.getSystemService(Context.USB_SERVICE) as UsbManager

    override fun onMethodCall(call: MethodCall, result: MethodChannel.Result) {
        when (call.method) {
            "listDevices" -> result.success(listDevices())
            "requestPermission" ->
                requestPermission(call.argument<String>("name"), result)
            "printBytes" -> printBytes(
                call.argument<String>("name"),
                call.argument<ByteArray>("bytes"),
                result
            )
            else -> result.notImplemented()
        }
    }

    private fun listDevices(): List<Map<String, Any?>> {
        return usbManager.deviceList.values.map { dev ->
            mapOf(
                "name" to dev.deviceName,
                "vendorId" to dev.vendorId,
                "productId" to dev.productId,
                "product" to (dev.productName ?: ""),
                "manufacturer" to (dev.manufacturerName ?: ""),
                "isPrinter" to isPrinter(dev),
                "hasPermission" to usbManager.hasPermission(dev)
            )
        }
    }

    private fun isPrinter(dev: UsbDevice): Boolean {
        for (i in 0 until dev.interfaceCount) {
            if (dev.getInterface(i).interfaceClass ==
                UsbConstants.USB_CLASS_PRINTER) {
                return true
            }
        }
        return false
    }

    private fun findDevice(name: String?): UsbDevice? =
        usbManager.deviceList.values.firstOrNull { it.deviceName == name }

    private fun requestPermission(name: String?, result: MethodChannel.Result) {
        val device = findDevice(name)
        if (device == null) {
            result.error("no_device", "USB device not found.", null)
            return
        }
        if (usbManager.hasPermission(device)) {
            result.success(true)
            return
        }
        val receiver = object : BroadcastReceiver() {
            override fun onReceive(context: Context, intent: Intent) {
                if (intent.action != ACTION_USB_PERMISSION) return
                activity.unregisterReceiver(this)
                val granted = intent.getBooleanExtra(
                    UsbManager.EXTRA_PERMISSION_GRANTED, false)
                result.success(granted)
            }
        }
        val filter = IntentFilter(ACTION_USB_PERMISSION)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            activity.registerReceiver(
                receiver, filter, Context.RECEIVER_NOT_EXPORTED)
        } else {
            @Suppress("UnspecifiedRegisterReceiverFlag")
            activity.registerReceiver(receiver, filter)
        }
        val flags = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S)
            PendingIntent.FLAG_MUTABLE else 0
        val intent = PendingIntent.getBroadcast(
            activity, 0,
            Intent(ACTION_USB_PERMISSION).setPackage(activity.packageName),
            flags)
        usbManager.requestPermission(device, intent)
    }

    private fun printBytes(
        name: String?,
        bytes: ByteArray?,
        result: MethodChannel.Result
    ) {
        val device = findDevice(name)
        if (device == null) {
            result.error("no_device", "USB device not found.", null)
            return
        }
        if (bytes == null || bytes.isEmpty()) {
            result.error("no_data", "No data to print.", null)
            return
        }
        if (!usbManager.hasPermission(device)) {
            result.error(
                "no_permission", "No USB permission for the printer.", null)
            return
        }

        var iface: UsbInterface? = null
        var endpoint: UsbEndpoint? = null
        loop@ for (i in 0 until device.interfaceCount) {
            val intf = device.getInterface(i)
            for (e in 0 until intf.endpointCount) {
                val ep = intf.getEndpoint(e)
                if (ep.direction == UsbConstants.USB_DIR_OUT &&
                    ep.type == UsbConstants.USB_ENDPOINT_XFER_BULK) {
                    iface = intf
                    endpoint = ep
                    // Prefer a printer-class interface when present.
                    if (intf.interfaceClass == UsbConstants.USB_CLASS_PRINTER) {
                        break@loop
                    }
                }
            }
        }
        if (iface == null || endpoint == null) {
            result.error(
                "no_endpoint",
                "No bulk OUT endpoint found on the printer.", null)
            return
        }

        val connection: UsbDeviceConnection? = usbManager.openDevice(device)
        if (connection == null) {
            result.error("open_failed", "Could not open the USB device.", null)
            return
        }
        try {
            if (!connection.claimInterface(iface, true)) {
                result.error(
                    "claim_failed",
                    "Could not claim the printer interface.", null)
                return
            }
            var offset = 0
            val chunk = 16384
            while (offset < bytes.size) {
                val len = minOf(chunk, bytes.size - offset)
                val part = bytes.copyOfRange(offset, offset + len)
                val sent = connection.bulkTransfer(endpoint, part, len, 5000)
                if (sent < 0) {
                    result.error(
                        "transfer_failed",
                        "USB bulk transfer failed at byte $offset.", null)
                    return
                }
                offset += sent
            }
            result.success(null)
        } finally {
            connection.releaseInterface(iface)
            connection.close()
        }
    }
}
