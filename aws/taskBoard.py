from todoist.api import TodoistAPI
import json

def organize_data(api, rooms):
  api.sync()
  projects = api.state['projects']
  item_dict = {}

  room_ids = {project['id']: project['name'] for project in projects if project['name'] in rooms}

  items = api.state['items']

  # print(items)
  for item in items:
    if not item['checked']:
      room_id = item['project_id']
      if room_id in room_ids:
        item_info = {'name': item['content'], 'priority': item['priority'], 'room': rooms.index(room_ids[room_id])}
        item_dict[item['id']] = item_info
  
  return item_dict

# Generate an a list of tasks in the order of their priority
def prioritize_items(item_dict, current_room):
  # separate tasks into current room and other rooms
  room_tiers = [[], [], [], []]
  other_tiers = [[], [], [], []]
  # iterate through all items
  for item, attributes in item_dict.items():
    if attributes['room'] == current_room:
      room_tiers[3-attributes['priority']].append(item)
    else:
      other_tiers[3-attributes['priority']].append(item)
  # Combine lists into one prioritized list
  ordered_tasks = []
  for tier in room_tiers:
    ordered_tasks += [item for item in tier]
  for tier in other_tiers:
    ordered_tasks += [item for item in tier]
  return ordered_tasks


# Convert the list of item ids to strings for printing
def create_display_strings(item_dict, current_room):
  order = prioritize_items(item_dict, current_room)
  # texts = [self.rooms[self.item_dict[item_id]['room']] + ': ' + self.item_dict[item_id]['name'] for item_id in order]
  # #truncate the strings so they fit on one line of the display
  # self.display_strings = [text[0:20] for text in texts]
  display_strings = [item_dict[item_id]['name'] for item_id in order]
  return display_strings

# Generate the colors for the rooms
def generate_colors(item_dict, rooms, current_room):
  # initialize each room to have priority 0
  room_vals = {room_num: 0 for room_num in range(len(rooms))}
  # iterate through all items and find the max priority task for each room
  for task, attributes in item_dict.items():
    room_vals[attributes['room']] = max(attributes['priority'], room_vals[attributes['room']])
  # convert priorities to color codes
  room_colors = {room: get_rgb(room_vals[room], room, current_room) for room in range(len(rooms))}
  return room_colors


# returns rgb color code based on priority of task
def get_rgb(priority, room, current_room):
  r = 0
  g = 0
  b = 0
  if priority == 0:
    b=1
  elif priority == 1:
    b=1
    g=1
  elif priority == 2:
    g=1
  elif priority == 3:
    g=1
    if room==current_room:
      r=1
    else:
      b=1
  else:
    r=1
  return [r, g, b]


def get_update(api, rooms, current_room, completed=-1):
  item_dict = organize_data(api, rooms)
  # check off an item if it was completed
  if completed >= 0:
    ordered_tasks = prioritize_items(item_dict, current_room)
    item = api.items.get_by_id(ordered_tasks[completed])
    item.complete()
    api.commit()
    item_dict = organize_data(api, rooms)
  display_strings = create_display_strings(item_dict, current_room)
  room_colors = generate_colors(item_dict, rooms, current_room)
  return_dict = {
    "tasks": len(display_strings),
    "display": display_strings,
    "colors": room_colors,
    "key": rooms
  }
  return json.dumps(return_dict)