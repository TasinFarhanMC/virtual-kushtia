#!/bin/bash

SESSION_NAME="Virtual-Kushtia"

if ! tmux has-session -t "$SESSION_NAME" 2>/dev/null; then
  tmux new-session -d -s "$SESSION_NAME"
  tmux send-keys -t "$SESSION_NAME" 'nvim' C-m

  tmux new-window -t "$SESSION_NAME":2

  tmux select-window -t "$SESSION_NAME":1

  tmux attach -t "$SESSION_NAME"

  # chromium --auto-open-devtools-for-tabs --window-size=375,812 "http://localhost:8000"
else
  tmux attach -t "$SESSION_NAME"
fi
