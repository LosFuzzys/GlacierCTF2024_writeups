import os
import sys
from time import sleep

from playwright.sync_api import sync_playwright


def visit(race, user, password, url):
    print("Checking Race", race, file=sys.stderr)

    with sync_playwright() as p:
        browser = p.chromium.launch(
            headless=True,
            args=[
                "--disable-dev-shm-usage",
                "--disable-extensions",
                "--disable-gpu",
                "--no-sandbox",
                "--headless"
            ])

        context = browser.new_context()
        page = context.new_page()
        page.goto(f"{url}/login")
        page.get_by_label("Username").fill(user)
        page.get_by_label("Password").fill(password)
        page.get_by_role("button", name="Login").click()
        page.goto(f"{url}{race}")

        sleep(4)

        context.close()
        browser.close()