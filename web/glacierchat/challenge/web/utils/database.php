<?php

$fp = -1;
$tenant_id = null;
$users_table = null;
/**
    * @var SQLite3 $db
*/
$db = null;

function setGlobalVariables() {
   global $tenant_id;
   global $users_table; 
   global $fp;
   $tenant_id = getTenantID(); 
   $users_table = "tenant_" . $tenant_id . "_users";
   $fp = fopen("/tmp/db.lock", "c+");
}

if(!defined("SKIP_INIT")) setGlobalVariables(); 

function acquireDatabase() {
    global $fp;
    global $db;
    
    $lock = false;
    $timeout_seconds = 10;
    $useconds_sleep = 100;
    for($i = 0; $i <= 1000000 / $useconds_sleep * $timeout_seconds; $i++) {
        $lock = flock($fp, LOCK_EX);
        if($lock) break;
        usleep($useconds_sleep);
    } 
    if(!$lock) die("Database could not be accessed");
    $db_file = "/tmp/db.sql";
    $db = new SQLITE3($db_file);
    return $db;
}

function releaseDatabase() {
    global $fp;
    global $db;
    $db->close();
    fflush($fp);
    flock($fp, LOCK_UN);
}

function setupDatabase() {
    acquireDatabase();
    prepareTenant();
    prepareTables();

    createUser("admin", random_str(64), getNewSecret());
    releaseDatabase();
}

function prepareTenant() {
    global $users_table;
    global $db;
    $db->exec("DROP TABLE IF EXISTS $users_table");
    $db->exec("CREATE TABLE IF NOT EXISTS $users_table(id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT, password_hash TEXT, password_cost INT, reset_code TEXT, totp_secret TEXT)");
}

function prepareTables() {
    global $db;
    $db->exec("DROP TABLE IF EXISTS content");
    $db->exec("CREATE TABLE IF NOT EXISTS content(id INTEGER PRIMARY KEY AUTOINCREMENT, user INTEGER, type TEXT, content TEXT, approved INTEGER)");
    $db->exec("DROP TABLE IF EXISTS protected_content");
    $db->exec("CREATE TABLE IF NOT EXISTS protected_content(id INTEGER PRIMARY KEY AUTOINCREMENT, post INTEGER, password_hash TEXT)");
    $db->exec("DROP TABLE IF EXISTS approval_requests");
    $db->exec("CREATE TABLE IF NOT EXISTS approval_requests(id INTEGER PRIMARY KEY AUTOINCREMENT, post INTEGER, message TEXT)");
}

function createUser($username, $password, $totp_secret) {
    global $users_table;
    global $db;
    $stmt = $db->prepare("INSERT INTO $users_table (username, password_hash, password_cost, totp_secret) VALUES (:name, :hash, :cost, :totp_secret)");
    $stmt->bindValue(":name", $username, SQLITE3_TEXT);
    $stmt->bindValue(":hash", password_hash($password, PASSWORD_BCRYPT, array(
        "cost" => 12
    )), SQLITE3_TEXT);
    $stmt->bindValue(":cost", 12, SQLITE3_INTEGER);
    $stmt->bindValue(":totp_secret", $totp_secret, SQLITE3_TEXT);
    $stmt->execute();
}



?>
