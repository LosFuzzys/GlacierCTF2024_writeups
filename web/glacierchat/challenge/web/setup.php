<?php
define("SKIP_INIT", true);
require_once __DIR__ . "/utils/index.php";

setNewTenantID();
setGlobalVariables();
setupDatabase();

?>
