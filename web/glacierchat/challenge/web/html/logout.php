<?php
require_once __DIR__ . "/../utils/index.php";
destroySession();
header("Location: /login.php");
