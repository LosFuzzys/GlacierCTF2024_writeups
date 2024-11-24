<?php

// https://stackoverflow.com/a/31107425
function random_str(
    int $length = 64,
    string $keyspace = '0123456789abcdefghijklmnpqrstuvwxyzABCDEFGHIJKLMNPQRSTUVWXYZ',
): string {
    if ($length < 1) {
        throw new \RangeException("Length must be a positive integer");
    }
    $pieces = [];
    $max = mb_strlen($keyspace, '8bit') - 1;
    for ($i = 0; $i < $length; ++$i) {
        $pieces []= $keyspace[random_int(0, $max)];
    }
    return implode('', $pieces);
}

function getTenantID() {
    return file_get_contents("/tmp/tenant");
}

function setNewTenantID() {
    $tenant = random_str(8);
    file_put_contents("/tmp/tenant", $tenant);
    return $tenant;
}

function getResetCode($prefix = "") {
    $reset_code = $prefix . random_str(16);
    if($prefix !== "") echo "Warning: Reset code " . $reset_code . " uses custom prefix. Using a prefix is deprecated in future releases.";
    return $reset_code;
}
