<?php
require_once __DIR__ . "/../utils/index.php";
$sessionID = getSessionID();
if($sessionID) {
    header("Location: /");
    exit(0);
}
if(isset($_POST["form_name"]) && $_POST["form_name"] === "reset"
    && isset($_POST["username"]) && strlen($_POST["username"]) > 0
) {
    acquireDatabase();
    $username = $_POST["username"];
    $stmt = $db->prepare("SELECT * FROM $users_table WHERE username = :username");
    $stmt->bindValue(":username", $username, SQLITE3_TEXT);
    $res = $stmt->execute();

    $data = $res->fetchArray();
    if($data) {
        $tenant = isset($_POST["is_tenant"]) && $_POST["is_tenant"] === "1" ? getTenantID() : "";
        $reset_code = getResetCode($tenant);
        $stmt = $db->prepare("UPDATE $users_table SET reset_code = :reset_code WHERE id = :id");
        $stmt->bindValue(":id", $data["id"], SQLITE3_INTEGER);
        $stmt->bindValue(":reset_code", $reset_code, SQLITE3_TEXT);
        $res = $stmt->execute();
    }
    $msg = "You will receive a mail with reset instructions, if you have an account on our service.";
    releaseDatabase();
}

require_once __DIR__ . "/../pages/header.php";
?>
<h2>Reset Password</h2>
<form method="POST">  
    <?= isset($msg) ? '<div class="alert alert-secondary">'.$msg.'</div>' : '' ?>
    <input type="hidden" name="form_name" value="reset" />
    <input type="text" name="username" placeholder="Username" />
    <input type="submit" value="Reset" />
</form>
<a href="/">Back</a>
<?php require_once __DIR__ . "/../pages/footer.php"; ?>
