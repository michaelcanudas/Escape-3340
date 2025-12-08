cd #!/bin/bash

cd /home/student334/CPSC-3340-Final-Project/receipt_client

# Path to your Python virtual environment
VENV_PATH="receipt_env"

# Activate the virtual environment
source "$VENV_PATH/bin/activate"

# Check if activation was successful
if [ $? -eq 0 ]; then
    echo "Virtual environment activated."

    # Run your Python script or command here
    python3 main.py
else
    echo "Failed to activate virtual environment."
fi

# Deactivate the virtual environment after the script finishes
deactivate
