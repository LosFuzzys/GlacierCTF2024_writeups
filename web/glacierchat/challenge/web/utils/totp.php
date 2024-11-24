<?php
use OTPHP\TOTP;


function getNewSecret() {
    return strtoupper(random_str(8, "ABCDEFGHIJKLMNPQRSTUVWXYZ234567"));
}

function verifySecret($secret, $code) {
    $otp = TOTP::createFromSecret($secret);
    return $otp->now() === $code;
}

?>
