<?php
require_once __DIR__ . "/../utils/index.php";
$sessionID = getSessionID();
if(!$sessionID) {
    header("Location: /login.php");
    exit(0);
}
require_once __DIR__ . "/../pages/header.php";
acquireDatabase();
$stmt = $db->prepare("SELECT * FROM $users_table WHERE id = :id");
$stmt->bindValue(":id", $_SESSION["id"], SQLITE3_INTEGER);
$res = $stmt->execute();
$data = $res->fetchArray();
releaseDatabase();
?>

<?php
if(isset($_POST["form_name"]) && $_POST["form_name"] === "preview_content"
    && isset($_POST["id"]) && strlen($_POST["id"]) > 0
) {
    $id = $_POST["id"];
    $encodedContent = fetchMediaContent($id);
    if($encodedContent) {
        echo "<img width=400 src=\"data:image;base64,$encodedContent\" />";
    }
}
?>

<?php
// TODO:Summer intern will implement permission system at some day 
if(isset($_POST["form_name"]) && $_POST["form_name"] === "approve_content"
    && isset($_POST["id"]) && strlen($_POST["id"]) > 0
) {
    $id = $_POST["id"];
    acquireDatabase();
    $stmt = $db->prepare("SELECT post 
        FROM approval_requests 
        WHERE id = :id");
    $stmt->bindValue(":id", $id, SQLITE3_INTEGER);
    $res = $stmt->execute();
    $entry = $res->fetchArray();
    releaseDatabase();
    if($entry) {
        $mediaContent = fetchMediaContent($id);
        acquireDatabase();
        $embeddedContent = "<img width=400 src=\"data:image;base64,$mediaContent\" />";

        $stmt = $db->prepare("UPDATE content  
            SET content = :content, approved = 1
            WHERE id = :id");
        $stmt->bindValue(":content", $embeddedContent, SQLITE3_TEXT);
        $stmt->bindValue(":id", $entry["post"], SQLITE3_INTEGER);
        $res = $stmt->execute();

        $stmt = $db->prepare("DELETE FROM approval_requests 
            WHERE id = :id");
        $stmt->bindValue(":id", $id, SQLITE3_INTEGER);
        $res = $stmt->execute();
        releaseDatabase();
    } else {
        echo "Could not find post for approval request";
    }
}
?>
<div class="row flex-middle">
<div class="col-4 paper">
<table>
    <thead> 
        <th>Type</th>
        <th>Content</th> 
        <th>Message</th> 
        <th>User</th>
        <th>Actions</th>
    </thead>
    <tbody>
<?php
acquireDatabase();
$res = $db->query("SELECT a.id, c.type, c.content, u.username, a.message
    FROM approval_requests a 
    INNER JOIN content c 
    INNER JOIN $users_table u 
    ON a.post = c.id AND u.id = c.user
    WHERE c.approved = 0");
    while($row = $res->fetchArray()) { ?>
        <tr>
            <td><?= $row["type"]; ?></td>
            <td><?= $row["content"]; ?></td>
            <td><?= $row["message"]; ?></td>
            <td><?= $row["username"]; ?></td>
            <td>
                <form method="POST">
                    <input type="hidden" name="form_name" value="preview_content" />
                    <input type="hidden" name="id" value="<?=$row["id"]?>" />
                    <button>Preview Content</button>
                </form>
                <form method="POST">
                    <input type="hidden" name="form_name" value="approve_content" />
                    <input type="hidden" name="id" value="<?=$row["id"]?>" />
                    <button>Approve Content</button>
                </form>
            </td>
        </tr>
        <?php } ?>
    </tbody>
</table>
</div>
<?php releaseDatabase(); ?>

<?php
if(isset($_POST["form_name"]) && $_POST["form_name"] === "createPost"
    && isset($_POST["content_type"]) && strlen($_POST["content_type"]) > 0
    && isset($_POST["media_uri"]) 
    && isset($_POST["message"]) 
    && isset($_POST["password_protection"]) 
    && isset($_POST["content"]) 
) {
    $content_type = $_POST["content_type"];
    if($content_type === "media" && strlen($_POST["media_uri"]) > 0) {
        $password_protection = $_POST["password_protection"];
        $media_uri = $_POST["media_uri"];
        if(filter_var($media_uri, FILTER_VALIDATE_URL) !== FALSE) {
            $message = $_POST["message"];
            insertMediaContent($media_uri);
            $password_hash = hashContentPassword($password_protection);
            $last_insert_id = getLastInsertedPost();
            if($password_hash) protectPost($last_insert_id, $password_hash);
            requirePostApproval($last_insert_id, $message);
        } else {
            echo "Media must be an URI!";
        }
    }
    if($content_type === "text" && strlen($_POST["content"]) > 0) {
        insertTextContent($_POST["content"]);
    }
}
?>

<div class="col-1"></div>
<div class="col-4 paper">
<form method="POST">
  <input type="hidden" name="form_name" value="createPost" />
  <select name="content_type">
    <option value="media">Media</option> 
    <option value="text">Text</option>
  </select>
  <input type="text" name="media_uri" placeholder="Media URI" />
  <textarea name="content" placeholder="Here goes your content..."></textarea>
  <textarea name="message" placeholder="Message to the admin for the approval request"></textarea>
  <input type="password" name="password_protection" placeholder="Password protect post"/>
  <input type="submit" value="Create" />
</form>
</div>

<div class="col-1"></div>
<div class="col-2 paper">
<table>
  <tr>
    <td><strong>UserID</strong></td>
    <td><?= $data["id"]; ?></td>
  </tr>
  <tr>
    <td><strong>Username</strong></td>
    <td><?= $data["username"]; ?></td>
  </tr>
</table>
</div>
</div>

<table>
    <thead> 
        <th>User</th> 
        <th>Content</th> 
    </thead>
    <tbody>
<?php
// TODO:Summer intern will implement password protection at some day 
acquireDatabase();
$res = $db->query("SELECT * FROM content c INNER JOIN $users_table u ON c.user = u.id WHERE c.approved = 1");
        while($row = $res->fetchArray()) { ?>
        <tr>
            <td><?= $row["username"]; ?></td>
            <td><?= $row["content"]; ?></td>
        </tr>
        <?php } ?>
<?php releaseDatabase(); ?>
    </tbody>
</table>

<a href="/logout.php">Logout</a>
<?php require_once __DIR__ . "/../pages/footer.php"; ?>
