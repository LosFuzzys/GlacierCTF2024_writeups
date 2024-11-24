<?php
require_once __DIR__ . "/../utils/index.php";
$sessionID = getSessionID();
if($sessionID) {
    header("Location: /");
    exit(0);
}
if(isset($_POST["form_name"]) && $_POST["form_name"] === "set_new_password"
    && isset($_POST["code"]) && strlen($_POST["code"]) > 0
    && isset($_POST["username"]) && strlen($_POST["username"]) > 0
    && isset($_POST["password"]) && strlen($_POST["password"]) > 0
    && isset($_POST["password_confirm"]) && strlen($_POST["password_confirm"]) > 0
) {
    acquireDatabase();
    $code = $_POST["code"];
    $username = $_POST["username"];
    $password = $_POST["password"];
    $password_confirm = $_POST["password_confirm"];
    if($password == $password_confirm && str_contains(strtolower($username), "blob") === false) { // NOPE
        $stmt = $db->prepare("SELECT id FROM $users_table WHERE reset_code = :code");
        $stmt->bindValue(":code", $code, SQLITE3_TEXT);
        $res = $stmt->execute();
        $data = $res->fetchArray();
        if($data) {
            $id = $data["id"];
            $stmt = $db->prepare("SELECT password_cost FROM $users_table WHERE reset_code = :code AND username = '$username'");
            $stmt->bindValue(":code", $code, SQLITE3_TEXT);
            $stmt->bindValue(":username", $username, SQLITE3_TEXT);
            $res = $stmt->execute();
            $data = $res->fetchArray();
            if($data) {
                $password_cost = $data["password_cost"];
                $password_cost = $password_cost > 12 ? 12 : $password_cost;
                $password_cost = $password_cost < 5 ? 5: $password_cost;
                $password_hash = password_hash($password, PASSWORD_BCRYPT, array(
                    "cost" => $password_cost     
                ));
                $stmt = $db->prepare("UPDATE $users_table SET password_hash = :password_hash WHERE id = :id");
                $stmt->bindValue(":password_hash", $password_hash, SQLITE3_TEXT);
                $stmt->bindValue(":id", $id, SQLITE3_INTEGER);
                $stmt->execute();
            } 
            $stmt = $db->prepare("UPDATE $users_table SET reset_code = NULL WHERE id = :id");
            $stmt->bindValue(":id", $id, SQLITE3_INTEGER);
            $res = $stmt->execute();

            $msg_succ = "Successfully reset password";
        } else {
            $msg = "Code invalid!";
        }
    } else {
        $msg = "Passwords do not match!";
    }
    releaseDatabase();
}

require_once __DIR__ . "/../pages/header.php";
?>
<h2>Reset Password</h2>
<?= isset($msg) ? '<div class="alert alert-danger">'.$msg.'</div>' : '' ?>
<?= isset($msg_succ) ? '<div class="alert alert-success">'.$msg_succ.'</div>' : '' ?>
<form method="POST">
    <input type="hidden" name="form_name" value="set_new_password" />
    <input type="text" name="code" placeholder="Reset Code" />
    <input type="username" name="username" placeholder="Username" />
    <input type="password" name="password" placeholder="New Password" />
    <input type="password" name="password_confirm" placeholder="Confirm new Password" />
    <input type="submit" value="Set new password" />
</form>
<?php require_once __DIR__ . "/../pages/footer.php"; ?>
