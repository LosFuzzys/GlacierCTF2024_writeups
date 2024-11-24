<?php

function fetchMediaContent($id) {
    global $db;
    acquireDatabase();
    $stmt = $db->prepare("SELECT c.content 
        FROM content c 
        INNER JOIN approval_requests a
        ON c.id = a.post 
        WHERE a.id = :id");
    $stmt->bindValue(":id", $id, SQLITE3_INTEGER);
    $res = $stmt->execute();
    if(!$res) return releaseDatabase();
    $row = $res->fetchArray();
    if(!$row) return releaseDatabase();;

    $media_content = $row["content"];
    $fetched_content = shell_exec("$media_content 2>&1");
    $encodedStuff = base64_encode($fetched_content);
    releaseDatabase();
    return $encodedStuff;
}


function insertMediaContent($media_uri) {
    global $db;
    acquireDatabase();
    $stmt = $db->prepare("INSERT INTO content (user, type, content, approved) VALUES (:user, :type, :content, 0)");
    $stmt->bindValue(":user", getSessionID());
    session_write_close();
    $media_command = escapeshellcmd("curl -s " . escapeshellarg($media_uri));
    $stmt->bindValue(":type", "media");
    $stmt->bindValue(":content", $media_uri);
    $stmt->execute();
    releaseDatabase();
}

function getLastInsertedPost() {
    global $db;
    acquireDatabase();
    $res = $db->query("SELECT MAX(id) as last_insert_id FROM content");
    $data = $res->fetchArray();
    $post_id = $data ? $data["last_insert_id"] : 0;
    releaseDatabase();
    return $post_id;
}

function protectPost($postId, $password_hash) {
    global $db;
    acquireDatabase();
    $stmt = $db->prepare("INSERT INTO protected_content(post, password_hash) VALUES (:post, :password_hash)");
    $stmt->bindValue(":post", $postId);
    $stmt->bindValue(":password_hash", $password_hash);
    $stmt->execute();
    releaseDatabase();
}

function requirePostApproval($postId, $message) {
    global $db;
    acquireDatabase();
    $stmt = $db->prepare("INSERT INTO approval_requests(post, message) VALUES (:post, :message)");
    $stmt->bindValue(":post", $postId);
    $stmt->bindValue(":message", $message);
    $stmt->execute();
    releaseDatabase();
}

function hashContentPassword($password) {
    $password_hash = false;
    if(strlen($password) > 0) {
        $password_hash = password_hash($password, PASSWORD_BCRYPT, array(
            "cost" => 12
        ));
    }
    return $password_hash;
}

function insertTextContent($unparsedContent) {
    global $db;
    acquireDatabase(); 
    $content = htmlspecialchars($unparsedContent);
    $stmt = $db->prepare("INSERT INTO content (user, type, content, approved) VALUES (:user, :type, :content, 1)");
    $stmt->bindValue(":user", getSessionID());
    $stmt->bindValue(":type", "text");
    $stmt->bindValue(":content", $content);
    $stmt->execute();
    releaseDatabase();
}

?>
