<?php include 'header.php'; ?>
<main>
    <section>
        <p>
            Welcome to our Malware Scanner. This service allows you to upload a single <strong>.tar.gz</strong> file, which our server will then carefully examine for potential malware threats. We'll provide you with a percentage indicating how certain we are that malware is contained within the uploaded file.
        </p>
        <p>
            Why is checking for malware important? Malicious software, or malware, poses a significant threat to the security and integrity of your data and systems. By using our scanner, you can proactively detect and remove potential threats before they can cause harm. It's an essential step in safeguarding your digital assets.
        </p>
        <p>
            So, go ahead and use our Malware Scanner to ensure the safety of your files. Your data security is our top priority.
        </p>
    </section>
    <section>
        <h2>Scan for Malware</h2>
        <div class="upload-form">
            <form action="upload.php" method="post" enctype="multipart/form-data">
                <label for="file">Choose a file to upload:</label>
                <input type="file" name="file" id="file" accept=".tar.gz">
                <button type="submit" class="upload-button">Upload File</button>
            </form>
        </div>
    </section>
</main>
<?php include 'footer.php'; ?>
