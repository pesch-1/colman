package de.colman.colman_client

import android.content.Context
import android.net.wifi.WifiManager
import io.flutter.embedding.android.FlutterActivity
import io.flutter.embedding.engine.FlutterEngine
import io.flutter.plugin.common.MethodChannel

class MainActivity : FlutterActivity() {
    private var multicastLock: WifiManager.MulticastLock? = null

    override fun configureFlutterEngine(flutterEngine: FlutterEngine) {
        super.configureFlutterEngine(flutterEngine)
        MethodChannel(
            flutterEngine.dartExecutor.binaryMessenger,
            UsbPrinterChannel.CHANNEL
        ).setMethodCallHandler(UsbPrinterChannel(this))
        MethodChannel(
            flutterEngine.dartExecutor.binaryMessenger,
            "de.colman.colman_client/multicast_lock"
        ).setMethodCallHandler { call, result ->
            when (call.method) {
                "acquire" -> {
                    if (multicastLock == null) {
                        val wifi = applicationContext
                            .getSystemService(Context.WIFI_SERVICE) as WifiManager
                        multicastLock = wifi.createMulticastLock("colman-mdns").apply {
                            setReferenceCounted(false)
                            acquire()
                        }
                    }
                    result.success(null)
                }
                "release" -> {
                    multicastLock?.let {
                        if (it.isHeld) {
                            it.release()
                        }
                    }
                    multicastLock = null
                    result.success(null)
                }
                else -> result.notImplemented()
            }
        }
    }

    override fun onDestroy() {
        multicastLock?.let {
            if (it.isHeld) {
                it.release()
            }
        }
        multicastLock = null
        super.onDestroy()
    }
}
