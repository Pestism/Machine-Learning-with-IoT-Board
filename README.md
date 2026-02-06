# Machine-Learning-with-IoT-Board
Some ML algorithms implemented in an ESP32 Arduino IoT board. Intended for the IK1332 course.

## ML_Anomaly_Detection
This folder contains the firmware, MATLAB scripts, and training data used to detect sudden temperature anomalies. A Gaussian probabilistic model is trained on normal operating data and used as the cost function to identify deviations indicative of anomalous temperature behavior.

## ML_Classification

This folder contains the firmware, MATLAB scripts, and training datasets used for location classification based on RSSI measurements. A binary logistic regression model is trained using a sigmoid activation function to learn and distinguish between two predefined location. The system can then classify the current position based on received signal strength.
