<?php
require_once __DIR__ . "/../utils/index.php";
$totp_check = getSessionID("totp_check");
if(!$totp_check){
    header("Location: /login.php");
    exit(0);
}
if(isset($_POST["form_name"]) && $_POST["form_name"] === "totp"
    && isset($_POST["totp"]) && strlen($_POST["totp"]) > 0
) {
    acquireDatabase();
    $totp = $_POST["totp"];
    $id = $_SESSION["totp_check"];
    $stmt = $db->prepare("SELECT * FROM $users_table WHERE id = :id");
    $stmt->bindValue(":id", $id, SQLITE3_INTEGER);
    $res = $stmt->execute();

    $data = $res->fetchArray();
    if($data) {
        $totp_secret = $data["totp_secret"];
        if(verifySecret($totp_secret, $totp)) {
            setSessionID($totp_check);
            header("Location: /");
            exit(0);
        }
    }
    $msg = "Wrong TOTP Token";
    releaseDatabase();
}

require_once __DIR__ . "/../pages/header.php";
?>
<h2>Two-Factor Authentication</h2>
<?= isset($msg) ? '<div class="alert alert-danger">'.$msg.'</div>' : '' ?>
<form method="POST">
    <input type="hidden" name="form_name" value="totp" />
    <input type="text" name="totp" placeholder="TOTP Code" />
    <input type="submit" value="Authenticate" />
</form>
<?php require_once __DIR__ . "/../pages/footer.php"; ?>
