<?php include 'header.php'; ?>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
    <main>
        <section>
            <h2>Why We Provide Malware Databases</h2>
            <p>At FuzzyBytes, transparency and trust are paramount. We believe in providing access to our malware databases to demonstrate our commitment to protecting your digital assets. By sharing our malware databases, we aim to achieve the following:</p>
            <ul class="beautiful-list">
                <li>Transparency: We want our users to understand the sources and types of malware our scanner detects.</li>
                <li>Educational Resource: Researchers and security enthusiasts can benefit from our databases for analysis and research.</li>
                <li>Community Collaboration: We encourage the cybersecurity community to contribute and improve our malware detection capabilities.</li>
            </ul>
        </section>
        <section>
            <h2>Malware Databases</h2>
            <ul>
                <?php
                $databasesDir = "databases";
                $databaseFiles = scandir($databasesDir);
                
                foreach ($databaseFiles as $file) {
                    if ($file != "." && $file != "..") {
                        echo "<li class='database-card'>
                                <i class='database-icon fa fa-database'></i>
                                <a class='database-link' href='$databasesDir/$file' target='_blank'>$file</a>
                              </li>";
                    }
                }
                ?>
            </ul>
        </section>
    </main>
<?php include 'footer.php'; ?>
