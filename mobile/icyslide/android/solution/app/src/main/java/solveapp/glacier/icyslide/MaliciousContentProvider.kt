package solveapp.glacier.icyslide

import android.content.ContentProvider
import android.content.ContentValues
import android.content.Context
import android.database.Cursor
import android.database.MatrixCursor
import android.net.Uri
import android.os.Environment
import android.os.ParcelFileDescriptor
import android.provider.OpenableColumns
import androidx.core.content.FileProvider
import java.io.File

class FileContentProvider : ContentProvider() {


    override fun onCreate(): Boolean {
        return true
    }

    override fun query(
        uri: Uri,
        projection: Array<String>?,
        selection: String?,
        selectionArgs: Array<String>?,
        sortOrder: String?
    ): Cursor? {
        val cursor = MatrixCursor(arrayOf(OpenableColumns.DISPLAY_NAME))

//         TODO fix path
        val predefinedFileName = "../config.cfg"
//        val predefinedFileName = "this_is_my_final_test_today.txt"
        cursor.addRow(arrayOf(predefinedFileName))

        return cursor
    }

    override fun getType(uri: Uri): String? {
        // Return the MIME type based on the URI path
        return when (uri.pathSegments[0]) {
            "storage" -> "application/octet-stream"
            else -> null
        }
    }

    override fun insert(uri: Uri, values: ContentValues?): Uri? {
        throw UnsupportedOperationException("Not supported")
    }

    override fun delete(uri: Uri, selection: String?, selectionArgs: Array<String>?): Int {
        throw UnsupportedOperationException("Not supported")
    }

    override fun update(uri: Uri, values: ContentValues?, selection: String?, selectionArgs: Array<String>?): Int {
        throw UnsupportedOperationException("Not supported")
    }

    override fun openFile(uri: Uri, mode: String): ParcelFileDescriptor {
        val solutionFile = File(context?.filesDir, "new_solution.txt")

        // Ensure the file exists
        if (!solutionFile.exists()) {
            solutionFile.writeText("{\n" +
                    "  \"app\": {\n" +
                    "    \"name\": \"IcySlide\",\n" +
                    "    \"version\": \"1.0.0\",\n" +
                    "    \"developer\": \"LetsFailNoob\"\n" +
                    "  },\n" +
                    "  \"encryption\": {\n" +
                    "    \"algorithm\": \"Fernet\",\n" +
                    "    \"keySize\": \"default\",\n" +
                    "    \"encryptionKey\": \"REDACTED\"\n" +
                    "  },\n" +
                    "  \"server\": {\n" +
                    "    \"endpoints\": {\n" +
                    "      \"encrypt\": \"upload\",\n" +
                    "      \"decrypt\": \"getData\"\n" +
                    "    },\n" +
                    "    \"timeout\": \"None\",\n" +
                    "    \"apiKey\": \"5233e94b-e63d-4a63-b8c3-7093104cf6ae\",\n" +
                    "    \"canGetFlag\": true\n" +
                    "  },\n" +
                    "  \"storage\": {\n" +
                    "    \"internalDirectory\": \"default\",\n" +
                    "    \"allowedFileTypes\": [\n" +
                    "      \"text/plain\"\n" +
                    "    ]\n" +
                    "  },\n" +
                    "  \"logging\": {\n" +
                    "    \"logLevel\": \"NONE\",\n" +
                    "    \"logFilePath\": \"\"\n" +
                    "  }\n" +
                    "}") // Write predefined content
        }

        // Return a ParcelFileDescriptor for the file
        return ParcelFileDescriptor.open(solutionFile, ParcelFileDescriptor.MODE_READ_ONLY)
    }
}