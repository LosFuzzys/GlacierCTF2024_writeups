<?php

function getSessionID($field = "id") {
    @session_start();
    return (isset($_SESSION[$field])) ? $_SESSION[$field] : false;
    session_write_close();
}

function setSessionID($id, $field = "id") {
    @session_start();
    $_SESSION[$field] = $id;
    session_write_close();
}

function destroySession() {
    @session_start();
    session_unset();
    session_destroy();
}

?>
