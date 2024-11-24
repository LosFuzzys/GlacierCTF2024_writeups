<?php
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $uploadDir = "/tmp/";
    $targetFile = $uploadDir . basename($_FILES["file"]["name"]);
    $fileExtension = pathinfo($_FILES["file"]["name"], PATHINFO_EXTENSION);
    $allowedExtensions = array('gz');

    echo '<div id="log"></div>';

    if (!in_array($fileExtension, $allowedExtensions)) {
        echo "Sorry, only .tar.gz files are allowed.";
        exit();
    }

    ob_start();

    
    function addLog($message) {
        echo "<script>document.getElementById('log').innerHTML += '$message<br>';</script>";
        ob_flush();
        flush();
    }

    addLog("Unpacking file...");
    
    if (move_uploaded_file($_FILES["file"]["tmp_name"], $targetFile)) {
        addLog("The file " . htmlspecialchars(basename($_FILES["file"]["name"])) . " has been uploaded.");

        if ($fileExtension === 'gz') {
            addLog("Scanning file...");

            // Execute python malware checker
            exec("python3 /usr/check_for_malicious_code.py " . escapeshellarg($targetFile), $output, $returnCode);

            if ($returnCode === 0) {
                addLog("Python script executed successfully.");
            } else {
                addLog("Error executing Python script.");
            }

            unlink($targetFile);
            addLog("Cleaning up...");

            addLog("Done.");

        }
    } else {
        addLog("Sorry, there was an error uploading your file.");
    }

    ob_end_flush();
}
?>



<link rel="stylesheet" type="text/css" href="styles.css">
<a href="index.php" class="back-button">Back to Homepage</a>