<?php include 'header.php'; ?>

<main>
    <section>
        <h2>Contact Us</h2>
        <p>If you have any questions, feedback, or need assistance, please feel free to contact us. We value your input and are here to help you.</p>

        <h3>Contact Form</h3>
        <p>Use the form below to get in touch with us:</p>

        <form action="contact.php" method="post" class="modern-contact-form">
            <div class="form-group">
                <label for="name">Your Name</label>
                <input type="text" id="name" name="name" required>
            </div>

            <div class="form-group">
                <label for="email">Your Email</label>
                <input type="email" id="email" name="email" required>
            </div>

            <div class="form-group">
                <label for="message">Your Message</label>
                <textarea id="message" name="message" rows="5" required></textarea>
            </div>

            <div class="form-group">
                <button type="submit">Submit</button>
            </div>
        </form>
    </section>
</main>

<?php include 'footer.php'; ?>
