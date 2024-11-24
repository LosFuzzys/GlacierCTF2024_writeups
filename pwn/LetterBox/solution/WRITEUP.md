# Writeup

## SSTI Vulnerability in Web-App

Vulnerable SSTI Code:
```python
    return render_template_string(html)
```

Letters send from the mail program will directly be rendered on the webpage. Therefore, when sending a letter containing an SSTI payload, it will trigger, allowing to exploit the webapp and get the flag from the config.

## Use After Free Vulnerability in Mail Program

The mail program has two structs:

```c
struct __attribute__((packed)) User {
    char username[128];
    char password[128];
} typedef User;

struct __attribute__((packed)) Letter {
    char content[256];
    size_t id;
} typedef Letter;
```

When deleting a Letter, the block is freed, however the Letter id is not reset. Because Letter blocks are big enough for User blocks, when deleting a Letter and then creating a User, the block will be reused. Therefore, when deleting a Letter and then creating a new User, the Letter block will be overwritten by the User struct. When sending all Letters then, the content of the User struct will be send instead, allowing to bypass the `is_alphanumeric` check.

Steps:
1. Register User and Login
2. Write a Letter
3. Delete the Letter
4. Logout and create a user with the SSTI payload as the Username
5. Login as the other User again
6. Send all Letters
7. Login to Web-UI, triggering the SSTI and get flag