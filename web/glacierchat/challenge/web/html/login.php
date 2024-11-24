<?php
require_once __DIR__ . "/../utils/index.php";
$sessionID = getSessionID();
if($sessionID) {
    header("Location: /");
    exit(0);
}
if(isset($_POST["form_name"]) && $_POST["form_name"] === "login"
    && isset($_POST["username"]) && strlen($_POST["username"]) > 0
    && isset($_POST["password"]) && strlen($_POST["password"]) > 0
) {
    acquireDatabase();
    $username = $_POST["username"];
    $password = $_POST["password"];
    $stmt = $db->prepare("SELECT * FROM $users_table WHERE username = :username");
    $stmt->bindValue(":username", $username, SQLITE3_TEXT);
    $res = $stmt->execute();

    $data = $res->fetchArray();
    if($data) {
        $password_hash = $data["password_hash"];
        if(password_verify($password, $password_hash)) {
            setSessionID($data["id"], "totp_check");
            header("Location: /totp.php");
            exit(0);
        }
    }
    $msg = "Wrong credentials";
    releaseDatabase();
}

require_once __DIR__ . "/../pages/header.php";
?>
<h2>Login</h2>
<form method="POST">
    <?= isset($msg) ? '<div class="alert alert-danger">'.$msg.'</div>' : '' ?>
    <input type="hidden" name="form_name" value="login" />
    <input type="text" name="username" placeholder="Username" />
    <input type="password" name="password" placeholder="Password" />
    <input type="submit" value="Login" />
</form>
<a href="/reset.php">Reset password</a>
<?php require_once __DIR__ . "/../pages/footer.php"; ?>
