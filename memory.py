# memory.py

# This module handles the bot's persistent memory.
# Memory could include user settings, conversation history, or other stateful data.

class Memory:
    def __init__(self):
        self.store = {}

    def get(self, key):
        return self.store.get(key, None)

    def set(self, key, value):
        self.store[key] = value

# Extend this class for database or file-based memory persistence.