<?php
require_once __DIR__ . "/utils/index.php";
function ilog($message) {
  echo "[+] " . $message . PHP_EOL;
}

setSessionID(1);

$jokes = explode("\n", file_get_contents("jokes.txt"));
$joke = $jokes[array_rand($jokes)];

ilog("Post joke.");
insertTextContent($joke);

?>
