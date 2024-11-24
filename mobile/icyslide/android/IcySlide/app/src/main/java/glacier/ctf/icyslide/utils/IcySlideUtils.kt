package glacier.ctf.icyslide.utils

import android.content.Context
import android.content.pm.PackageInfo
import android.content.pm.PackageManager
import android.util.Base64
import com.scottyab.rootbeer.RootBeer
import java.security.MessageDigest

class IcySlideUtils {
    init {
        System.loadLibrary("integrity-app")
    }

    external fun init1(): Boolean
    external fun init2(): Boolean
    external fun isSignatureValid(hash: String): Boolean

    fun getAppSignature(context: Context): String? {
        return try {
            val packageInfo: PackageInfo = context.packageManager.getPackageInfo(
                context.packageName,
                PackageManager.GET_SIGNING_CERTIFICATES
            )
            val signatures = packageInfo.signingInfo?.apkContentsSigners // This gets the signatures

            if (!signatures.isNullOrEmpty()) {
                val md = MessageDigest.getInstance("SHA")
                md.update(signatures[0].toByteArray()) // Use the first signature
                val signature = md.digest()
                Base64.encodeToString(signature, Base64.NO_WRAP)
                    .trim() // Return base64 encoded signature
            } else {
                null
            }
        } catch (e: Exception) {
            e.printStackTrace()
            null
        }
    }

    fun rooted(context: Context): Boolean {
        var rootChecker = RootBeer(context)
        if (rootChecker.isRooted || android.os.Debug.isDebuggerConnected() || android.os.Debug.waitingForDebugger()) {
            throw RuntimeException("Please no")
            return true
        }
        return false
    }
}
