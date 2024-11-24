package glacier.ctf.icyslide.externalaccess

import android.content.ContentProvider
import android.content.ContentValues
import android.content.UriMatcher
import android.database.Cursor
import android.database.MatrixCursor
import android.net.Uri
import android.os.ParcelFileDescriptor
import android.provider.OpenableColumns
import glacier.ctf.icyslide.network.decrypt
import glacier.ctf.icyslide.utils.IcySlideUtils
import kotlinx.coroutines.runBlocking
import java.io.File
import java.io.FileNotFoundException
import java.io.IOException


class SlideProvider : ContentProvider() {

    companion object {
        private const val AUTHORITY = "glacier.ctf.icyslide.slideProvider"
        private const val PATH = "storage"
        private const val ITEMS = 0
        private const val SINGLE_ITEM_NUMBER = 1
        private const val SINGLE_ITEM_NAME = 2

        private val CONTENT_URI: Uri = Uri.parse("content://$AUTHORITY/$PATH")

        fun buildFileUri(fileName: String): Uri {
            println(fileName)
//            val test = CONTENT_URI.buildUpon().appendEncodedPath(fileName).build()
            val test = Uri.withAppendedPath(CONTENT_URI, fileName)
            println(test)
            return test
        }

        private val uriMatcher: UriMatcher = UriMatcher(UriMatcher.NO_MATCH).apply {
            addURI(AUTHORITY, PATH, ITEMS)
            addURI(AUTHORITY, "$PATH/#", SINGLE_ITEM_NUMBER)
            addURI(AUTHORITY, "$PATH/*", SINGLE_ITEM_NAME)
        }
    }

    override fun onCreate(): Boolean {
        return true
    }

    override fun query(
        uri: Uri,
        projection: Array<out String>?,
        selection: String?,
        selectionArgs: Array<out String>?,
        sortOrder: String?
    ): Cursor? {
        when (uriMatcher.match(uri)) {
            SINGLE_ITEM_NAME -> {
                val filename = uri.lastPathSegment
                if (filename != null) {
                    val file = File(context?.filesDir, filename)

                    return if (file.exists()) {
                        val matrixCursor = MatrixCursor(
                            projection ?: arrayOf(
                                OpenableColumns.DISPLAY_NAME,
                                OpenableColumns.SIZE
                            )
                        )

                        val row = matrixCursor.newRow()
                        if (projection == null || projection.contains(OpenableColumns.DISPLAY_NAME)) {
                            row.add(OpenableColumns.DISPLAY_NAME, file.name)
                        }
                        if (projection == null || projection.contains(OpenableColumns.SIZE)) {
                            row.add(OpenableColumns.SIZE, file.length())
                        }

                        matrixCursor
                    } else {
                        return null
                    }
                } else {
                    return null
                }
            }

            else -> {
                return null
            }
        }
    }

    override fun getType(uri: Uri): String {
        return "text/plain"
    }

    override fun insert(uri: Uri, values: ContentValues?): Uri? {
        return null
    }

    override fun delete(uri: Uri, selection: String?, selectionArgs: Array<out String>?): Int {
        return 0
    }

    override fun update(
        uri: Uri,
        values: ContentValues?,
        selection: String?,
        selectionArgs: Array<out String>?
    ): Int {
        return 0
    }

//    override fun openFile(uri: Uri, mode: String): ParcelFileDescriptor? {
//
//        if (mode != "r") {
//            throw IllegalArgumentException("Only read mode 'r' is supported!")
//        }
//
//        when (uriMatcher.match(uri)) {
//            SINGLE_ITEM_NAME -> {
//                val filename = uri.lastPathSegment
//                if (filename != null) {
//                    val file = File(File(context?.filesDir, PATH), filename)
//                    if (file.exists()) {
//                        return runBlocking { // Blocks until the coroutine finishes
//                            val decryptedData = decryptFile(file) // Suspends here until done
//                            if (decryptedData != null) {
//                                // Optionally, write decryptedData to a file here or use it
//                                return@runBlocking ParcelFileDescriptor.open(
//                                    file,
//                                    ParcelFileDescriptor.MODE_READ_ONLY
//                                )
//                            } else {
//                                throw FileNotFoundException("Decryption failed for file: $filename")
//                            }
//                        }
//                    } else {
//                        throw FileNotFoundException("File not found: $filename")
//                    }
//                } else {
//                    throw FileNotFoundException("Filename not provided in URI: $uri")
//                }
//            }
//
//            else -> {
//                throw FileNotFoundException("Unknown URI: $uri")
//            }
//        }
//
//        return null
//    }

    override fun openFile(uri: Uri, mode: String): ParcelFileDescriptor? {
        if (mode != "r") {
            throw IllegalArgumentException("Only read mode 'r' is supported!")
        }

        val matchedUri = uriMatcher.match(uri)
        if (matchedUri != SINGLE_ITEM_NAME) {
            throw FileNotFoundException("Unknown URI: $uri")
        }

        val filename =
            uri.lastPathSegment ?: throw FileNotFoundException("Filename not provided in URI: $uri")

        val file = File(context?.filesDir, filename)

        if (!file.exists()) {
            throw FileNotFoundException("File not found: $filename")
        }

        val utils = IcySlideUtils()
        if (android.os.Debug.isDebuggerConnected() || android.os.Debug.waitingForDebugger() || utils.rooted(
                context!!
            )
        ) {
            throw RuntimeException("Please don't debug")
        }

        if (utils.init1() || !utils.init2()) {
            throw RuntimeException("Initialization failed")
        }
        val appSignature = utils.getAppSignature(context!!)
        if (!utils.isSignatureValid(appSignature!!)) {
            return null
        }

        return try {
            val decryptedData = runBlocking {
                context?.let { decrypt(file, it) }
            }
            if (decryptedData != null) {
                val shareFile = File.createTempFile("share_data", "tmp")
                shareFile.writeText(decryptedData)
                ParcelFileDescriptor.open(
                    shareFile,
                    ParcelFileDescriptor.MODE_READ_ONLY
                )
            } else {
                throw FileNotFoundException("Decryption failed for file: $filename")
            }
        } catch (e: IOException) {
            throw FileNotFoundException("Error opening file: ${e.message}")
        }
    }
}