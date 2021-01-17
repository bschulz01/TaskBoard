# TaskBoard
IDEA Hacks 2021 submission by Bradley Schulz and Tyler Price

## What it does
TaskBoard is an extremely versatile task management system that visualizes all of your most pressing tasks. Its **highly customizable user interface** and **clean integration with external services** lends it to a broad range of applications, from doing chores around the house to planning errands around a city.

TaskBoard syncs with the online reminder service **Todoist**, printing out your most urgent tasks and illuminating a map of your house according to what tasks need to be done. The color of each room is determined by the urgency of the task(s) to be done in that room, allowing you to **easily visualize your todo list**. The TaskBoard also allows you to **easily check these tasks off of your Todoist lists with the push of a button.**

Since a **majority of the processing occurs in the cloud,** multiple TaskBoards can be connected to the same account—such as if a user wishes to have one in each room—and they will all synchronize with no modifications necessary. Additionally, changing the color codes or text formatting can be done remotely through the AWS console, minimizing the need for firmware updates.

To save energy, **the display and LEDs automatically turns off after no activity has been sensed.** To reactivate the display and LEDs, **the user simply has to wave their hand in front of the board or press one of the two buttons.**

## How we built it
Using **Lambda on Amazon Web Services**, we built a **custom API wrapper** for the Todoist API from which an ESP32 microcontroller, connected to a local network, can retrieve all your most urgent tasks. That ESP32 microcontroller then prints out those tasks on an SSD1306 OLED display so the user can see their most pressing tasks and check them off one by one using two pushbuttons. **RGB LEDs** controlled via a shift register illuminate the map, and an **infrared distance sensor wakes up the map** when a user is nearby. To make it **completely independent of any external wires**, it is powered by an onboard USB power bank.

## How to Use it
### Firmware Modifications
To customize the experience to your account and house, the user needs to input their unique Todoist API token, which can be found in settings --> integrations from the Todoist web interface. That token needs to be loaded in the formware, on line 16 of TaskBoard.ino.

To change the rooms in the house to match the rooms listed on a user's Todoist account, the values of room_array on line 18 of Todoist.ino can be updated. Note that the order of the rooms does matter, as the main room (the room whose tasks are displayed at the top of the list) that is specified by the variable main_room is used to index into this list.
  
### AWS Interface
The two scripts used to implement the custom AWS wrapper:
-  lambda_function.py is the landing function that parses the request and calls analysis functions
-  taskBoard.py communicates with the Todoist API to pull the most users tasks, group them by project, organize the tasks by priority, and generates the colors for each room
Note that the todoist and requests packagaes must be uplaoded to AWS in order for this script to work.

The custom API wrapper outputs the number of tasks to be completed; an array of strings that show the names of the tasks to display on the OLED display; the color codes of the 5 rooms in the form of a dictionary of arrays of 3 integers; and a copy of the ordered array of room labels to verify that the indices used by AWS and the local machine match.

## Challenges we ran into
Having the ESP32 microcontroller communicate directly with the Todoist API would be stretching the capabilities of the microcontroller, so we implemented a custom python script running on AWS Lambda that extract, analyzes, and organizes the relevant tasks from Todoist. Adding this intermediary was difficult to implement, as we had never used AWS before, but the results greatly increased the organization and performance of the TaskBoard.

Additionally, controlling 5 RGB LEDs along with two pushbuttons and the OLED display requires more GPIO pins than are available on the ESP32 microcontroller. So, we had to implement 74HC595 shift registers with a custom control function, to allow us to control all 5 RGB LEDs.

Finding an effective method of mounting the map to a wall also presented a challenge. We explored several ideas—including velcro, mounting brackets, and more—but we eventually settled on what is known as the French cleat mounting system. This mounting system beautifully and securely mounts the map to the wall, leading to a very clean and safe mount.

## What's next for TaskBoard
Improving the customization of the TaskBoard is the next step, with more options for maps and colors for the LEDs. **The paper map can be easily slid in and out of the case, allowing for easy customization through the design of new maps** One potential use for this is seasonal decoration; for example, the user's house could be decorated with flowers once Winter turns to Spring.

A more advanced LED driver and larger display would allow for more unique and clear displays. More colors allows for a more precise depiction of the urgency of each task, and a larger display would both allow for larger fonts and more tasks to be displayed at once.
Larger display

There exist a large number of integrations with Todoist, and we would like to make use of those. For example, **integrations with Google Calendar and the IOS notes app** already exist. Implementing those integrations, for example through [ifttt.com](https://ifttt.com/home), would greatly simplify the users' experience.





