package glacier.ctf.icyslide

import android.content.Intent
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.wrapContentWidth
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.outlined.Info
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import glacier.ctf.icyslide.externalaccess.SlideProvider
import glacier.ctf.icyslide.network.decrypt
import glacier.ctf.icyslide.ui.theme.IcySlideTheme
import glacier.ctf.icyslide.utils.IcySlideUtils
import kotlinx.coroutines.runBlocking
import java.io.File

class ViewFileActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val utils = IcySlideUtils()

        if (utils.rooted(this@ViewFileActivity) || android.os.Debug.isDebuggerConnected() || android.os.Debug.waitingForDebugger()) {
            throw RuntimeException("Nonono")
        }

        if (utils.init1() || !utils.init2()) {
            throw RuntimeException("Very much also no")
        }

        val files = filesDir.listFiles()?.toList() ?: emptyList()
        setContent {
            IcySlideTheme {
                FileListScreen(files) { finish() }
            }
        }
    }
}


@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun FileListScreen(files: List<File>, onBackClick: () -> Unit) {
    val context = LocalContext.current
    var selectedFile by remember { mutableStateOf<File?>(null) }
    var showOverlay by remember { mutableStateOf(false) }
    var revealedContent by remember { mutableStateOf<String?>(null) }
    var showDeleteAllDialog by remember { mutableStateOf(false) }
    var fileList by remember { mutableStateOf(files) }

    Scaffold(
        topBar = {
            TopAppBar(
                title = {
                    Text(
                        text = "View Files",
                        color = MaterialTheme.colorScheme.onPrimary,
                        fontSize = 20.sp
                    )
                },
                navigationIcon = {
                    IconButton(onClick = onBackClick) {
                        Icon(
                            imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                            contentDescription = "Back"
                        )
                    }
                },
                actions = {
                    IconButton(onClick = { showDeleteAllDialog = true }) {
                        Icon(
                            imageVector = Icons.Outlined.Info,
                            contentDescription = "Help",
                            tint = MaterialTheme.colorScheme.secondary,
                            modifier = Modifier.scale(1.1f)
                        )
                    }
                },
                colors = TopAppBarDefaults.centerAlignedTopAppBarColors(
                    containerColor = MaterialTheme.colorScheme.primary
                )
            )
        }
    ) { paddingValues ->
        LazyVerticalGrid(
            columns = GridCells.Fixed(3),
            modifier = Modifier
                .fillMaxSize()
                .padding(paddingValues)
                .background(MaterialTheme.colorScheme.background)
        ) {
            items(fileList) { file ->
                FileItem(file) {
                    selectedFile = file
                    showOverlay = true
                    revealedContent = null
                }
            }
        }
    }
    if (showOverlay && selectedFile != null) {
        OverlayContent(
            file = selectedFile!!,
            onClose = {
                showOverlay = false
                selectedFile = null
                revealedContent = null
            },
            onRevealSecret = { revealedContent = runBlocking { decrypt(selectedFile!!, context) } },
            onDeleteFile = {
                if (selectedFile!!.exists()) {
                    selectedFile!!.delete()
                }
                fileList = context.filesDir.listFiles()?.toList() ?: emptyList()
                showOverlay = false
                selectedFile = null
                revealedContent = null
            },
            revealedContent = revealedContent
        )
    }

    if (showDeleteAllDialog) {
        DeleteAllDialog(onDismiss = { showDeleteAllDialog = false }, onConfirm = {
            context.filesDir.listFiles()?.forEach { it.delete() } // Delete all files
            showDeleteAllDialog = false // Close the dialog after deletion
        })
    }
}

@Composable
fun DeleteAllDialog(onDismiss: () -> Unit, onConfirm: () -> Unit) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { DeleteAllNotice() },
        containerColor = Color(0xFFFFF3CD),
        confirmButton = {
            TextButton(onClick = onConfirm) {
                Text("Got it, delete all files")
            }
        },
        dismissButton = {
            TextButton(
                onClick = { onDismiss() }
            ) {
                Text("No")
            }
        }
    )
}

@Composable
fun DeleteAllNotice() {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp)
            .background(Color(0xFFFFF3CD)) // Light yellow background
            .padding(16.dp), // Padding inside the box
        verticalAlignment = Alignment.CenterVertically
    ) {
        Icon(
            imageVector = Icons.Filled.Delete,
            contentDescription = "Warning",
            tint = MaterialTheme.colorScheme.scrim, // Warning color
            modifier = Modifier.size(24.dp) // Set size of the icon
        )
        Spacer(modifier = Modifier.width(8.dp))
        SelectionContainer {
            Text(
                text = "This will delete all stored files!",
                color = Color(0xFF856404), // Dark brown color
                fontWeight = FontWeight.Bold, // Bold text for emphasis
                modifier = Modifier.weight(1f) // Take remaining space
            )
        }
    }
}

@Composable
fun OverlayContent(
    file: File,
    onClose: () -> Unit,
    onRevealSecret: () -> Unit,
    onDeleteFile: () -> Unit,
    revealedContent: String?
) {
    val context = LocalContext.current
    var showDeleteConfirmation by remember { mutableStateOf(false) }

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black.copy(alpha = 0.6f))
            .clickable { onClose() } // Close overlay on background click
    ) {
        Column(
            modifier = Modifier
                .align(Alignment.Center)
                .background(Color.White, shape = MaterialTheme.shapes.medium)
                .padding(16.dp)
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = file.name,
                    fontSize = 20.sp,
                    fontWeight = FontWeight.Bold,
                    modifier = Modifier
                        .padding(bottom = 8.dp, top = 8.dp),
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )

                IconButton(
                    onClick = { showDeleteConfirmation = true },
                ) {
                    Icon(
                        imageVector = Icons.Default.Delete,
                        contentDescription = "Delete file",
                        tint = MaterialTheme.colorScheme.onTertiaryContainer,
                        modifier = Modifier
                            .size(50.dp)
                    )
                }
            }

            if (showDeleteConfirmation) {
                AlertDialog(
                    onDismissRequest = { showDeleteConfirmation = false },
                    title = {
                        Text(text = "Delete File")
                    },
                    text = {
                        Text(text = "Are you sure you want to delete '${file.name}'?")
                    },
                    confirmButton = {
                        TextButton(
                            onClick = {
                                onDeleteFile() // Call delete function
                                showDeleteConfirmation = false
                                onClose() // Close overlay after deletion
                            }
                        ) {
                            Text("Yes")
                        }
                    },
                    dismissButton = {
                        TextButton(
                            onClick = { showDeleteConfirmation = false }
                        ) {
                            Text("No")
                        }
                    }
                )
            }

            revealedContent?.let {
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(200.dp)
                        .background(Color.Gray.copy(alpha = 0.3f)) // Placeholder for blurred effect
                        .verticalScroll(rememberScrollState())
                ) {

                    Text(
                        text = it,
                        modifier = Modifier.padding(16.dp),
                        color = Color.White
                    )
                }
            }

            Spacer(modifier = Modifier.height(24.dp))

            // Buttons for actions
            Row(
                horizontalArrangement = Arrangement.spacedBy(16.dp),
                modifier = Modifier.align(Alignment.CenterHorizontally)
            ) {
                Button(onClick = onRevealSecret) {
                    Text(text = "Reveal Secret")
                }
                Button(onClick = {
                    val uri = file.name.let { SlideProvider.buildFileUri(it) }
                    val sendIntent = Intent(Intent.ACTION_SEND).apply {
                        type = "text/plain"
                        putExtra(Intent.EXTRA_STREAM, uri)
                        addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
                    }
                    context.startActivity(
                        Intent.createChooser(
                            sendIntent,
                            "Share secret with other app"
                        )
                    )
                }) {
                    Text(text = "Send File")
                }
            }
        }
    }
}

@Composable
fun FileItem(file: File, onClick: () -> Unit) {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { onClick() }
            .padding(16.dp)
            .background(Color.White, shape = MaterialTheme.shapes.medium)
            .padding(8.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        Image(
            painter = painterResource(id = getFileIcon(file)),
            contentDescription = "File icon",
            modifier = Modifier.size(40.dp)
        )
        Text(
            text = file.name,
            fontSize = 16.sp,
            textAlign = TextAlign.Center,
            modifier = Modifier
                .padding(top = 4.dp)
                .wrapContentWidth(Alignment.CenterHorizontally)
        )
    }
}

fun getFileIcon(file: File): Int {
    return when (file.extension.lowercase()) {
        // Define icons based on file extension
        // "jpg", "jpeg", "png" -> R.drawable.penguins // Image icon
        // "txt" -> R.drawable.icon_text // Text file icon
        // "pdf" -> R.drawable.icon_pdf // PDF icon
        else -> R.drawable.package_icon // Default folder icon for other file types
    }
}

@Preview(showBackground = true)
@Composable
fun PreviewViewFile() {
    val sampleFiles = listOf(
        File("SampleFile1.txt"),
        File("SampleFile2.jpg"),
        File("SampleFile3.pdf"),
        File("SampleFile4.docx"),
        File("SampleFile5.xlsx"),
        File("SampleFile6.mp4")
    )
    IcySlideTheme { FileListScreen(files = sampleFiles, onBackClick = {}) }
}
