package glacier.ctf.icyslide.network

import android.app.Activity
import android.content.Context
import android.util.Base64
import android.util.Log
import android.widget.Toast
import glacier.ctf.icyslide.network.NetworkModule.apiService
import glacier.ctf.icyslide.utils.IcySlideUtils
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.MultipartBody
import okhttp3.RequestBody.Companion.asRequestBody
import java.io.File
import java.io.IOException

fun Context.displayToast(message: String) {
    (this as? Activity)?.runOnUiThread {
        Toast.makeText(this, message, Toast.LENGTH_SHORT).show()
    } ?: Toast.makeText(this, message, Toast.LENGTH_SHORT)
        .show() // Fallback for non-activity contexts
}

fun prepareFilePart(file: File, partName: String = "file"): MultipartBody.Part {
    val requestFile = file.asRequestBody("application/octet-stream".toMediaType())
    return MultipartBody.Part.createFormData(
        partName,
        file.canonicalPath,
        requestFile
    )
}

fun prepareConfig(context: Context): MultipartBody.Part {
    return prepareFilePart(File(context.dataDir, "config.cfg"), "config")
}

suspend fun decrypt(file: File, context: Context): String? = withContext(Dispatchers.IO) {
    /*
    ...
    */
    
    try {
        val filePart = prepareFilePart(file)
        val config = prepareConfig(context)
        val response = apiService.decrypt(appSignature, filePart, config).execute()

        if (response.isSuccessful) {
            response.body()?.processedData?.let { fileUploadResponse ->
                return@withContext try {
                    String(
                        Base64.decode(fileUploadResponse, Base64.DEFAULT),
                        Charsets.UTF_8
                    )
                } catch (e: IllegalArgumentException) {
                    Log.e("Decoding Error", "Invalid Base64 input: ${e.message}")
                    context.displayToast("Data decoding error.")
                    null
                }
            } ?: run {
                Log.e("Upload Error", "Received empty response.")
                context.displayToast("Received empty response from server.")
                return@withContext null
            }
        } else {
            val errorMessage = response.errorBody()?.string() ?: "Unknown error"
            Log.e("Upload Error", "Error: $errorMessage")
            context.displayToast("Server error: $errorMessage")
            return@withContext null
        }
    } catch (e: IOException) {
        Log.e("Network Error", "Network issue: ${e.message}")
        context.displayToast("Network error occurred.")
    } catch (e: Exception) {
        Log.e("Upload Failure", "Unexpected error: ${e.message}", e)
        context.displayToast("An unexpected error occurred.")
    }
}

suspend fun encrypt(file: File, context: Context): ByteArray? = withContext(Dispatchers.IO) {
    /*
    ...
    */
    try {
        val filePart = prepareFilePart(file)
        val config = prepareConfig(context)
        val response = apiService.encrypt(appSignature, filePart, config).execute()

        if (response.isSuccessful) {
            response.body()?.processedData?.let { fileUploadResponse ->
                return@withContext try {
                    Base64.decode(fileUploadResponse, Base64.DEFAULT)
                } catch (e: IllegalArgumentException) {
                    Log.e("Decoding Error", "Invalid Base64 input: ${e.message}")
                    context.displayToast("Data decoding error.")
                    null
                }
            } ?: run {
                Log.e("Upload Error", "Received empty response.")
                context.displayToast("Received empty response from server.")
                return@withContext null
            }
        } else {
            val errorMessage = response.errorBody()?.string() ?: "Unknown error"
            Log.e("Upload Error", "Error: $errorMessage")
            context.displayToast("Server error: $errorMessage")
            return@withContext null
        }
    } catch (e: IOException) {
        Log.e("Network Error", "Network issue: ${e.message}")
        context.displayToast("Network error occurred.")
    } catch (e: Exception) {
        Log.e("Upload Failure", "Unexpected error: ${e.message}", e)
        context.displayToast("An unexpected error occurred.")
    }
}

suspend fun checkConfig(context: Context): String? = withContext(Dispatchers.IO) {
    try {
        val config = prepareConfig(context)
        val response = apiService.checkConfig(appSignature, config).execute()

        if (response.isSuccessful) {
            response.body()?.processedData?.let { fileUploadResponse ->
                return@withContext try {
                    String(
                        Base64.decode(fileUploadResponse, Base64.DEFAULT),
                        Charsets.UTF_8
                    )
                } catch (e: IllegalArgumentException) {
                    Log.e("Decoding Error", "Invalid Base64 input: ${e.message}")
                    context.displayToast("Data decoding error.")
                    null
                }
            } ?: run {
                Log.e("Upload Error", "Received empty response.")
                context.displayToast("Received empty response from server.")
                return@withContext null
            }
        } else {
            val errorMessage = response.errorBody()?.string() ?: "Unknown error"
            Log.e("Upload Error", "Error: $errorMessage")
            context.displayToast("Server error: $errorMessage")
            return@withContext null
        }
    } catch (e: IOException) {
        Log.e("Network Error", "Network issue: ${e.message}")
        context.displayToast("Network error occurred.")
    } catch (e: Exception) {
        Log.e("Upload Failure", "Unexpected error: ${e.message}", e)
        context.displayToast("An unexpected error occurred.")
    }
}
