import boto3
import json

from taskBoard import *

def check_str(input):
    return '[' not in input and ',' not in input and ']' not in input

def lambda_handler(event, context):
    print(event)
    query = event.get('queryStringParameters', {})
    try:
        token = query.get('token', '')
        # print(token)
        # token = "2dac5121447fb1ff7b5040689c361f2499f0c84a"
        rooms_str = query.get('rooms', '')
        rooms = [room for room in rooms_str.split("'") if check_str(room)]
        # rooms = ['Kitchen', 'Bedroom', 'Office', 'Outside', 'Toiletries']
        current_room = query.get('room_id', 0)
        # current_room = 0
        # completed = event['complete']
        completed = int(query.get('completed', -1))
        api = TodoistAPI(token)
        print(token, rooms, current_room, completed)
        return get_update(api, rooms, current_room, completed)
    except KeyError:
        return json.dumps({"Error": "Invalid input(s)"})