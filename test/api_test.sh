#!/bin/bash

# API Testing Script for iSprinklr ESP WebServer
# This script tests various API endpoints and error handling

# Set default server address or use provided argument
SERVER_ADDRESS=${1:-"192.168.88.25"}
BASE_URL="http://${SERVER_ADDRESS}"

echo "==============================================="
echo "  iSprinklr ESP API Test"
echo "  Server: $BASE_URL"
echo "==============================================="

# Function to make a request and display the results
function make_request() {
  local description=$1
  local endpoint=$2
  local method=$3
  local data=$4
  
  echo -e "\n== Test: $description =="
  echo "Endpoint: $endpoint"
  echo "Method: $method"
  
  if [ -n "$data" ]; then
    echo "Data: $data"
    response=$(curl -s -X $method -H "Content-Type: application/json" -d "$data" "$BASE_URL$endpoint")
  else
    response=$(curl -s -X $method "$BASE_URL$endpoint")
  fi
  
  echo -e "Response: $response\n"
  
  # Optional: Add a small delay between requests
  sleep 0.5
}

# Test 0: Status check - Enhanced system information
make_request "Enhanced status check" "/api/status" "GET" ""

# Test 1: Valid start request
make_request "Valid start request" "/api/start" "POST" '{"zone":5,"minutes":10}'

# Test 2: Invalid JSON (syntax error)
make_request "Invalid JSON" "/api/start" "POST" '{"zone":5,"minutes":10'

# Test 3: Missing required parameter (zone)
make_request "Missing required parameter" "/api/start" "POST" '{"minutes":10}'

# Test 4: Missing required parameter (minutes)
make_request "Missing required parameter" "/api/start" "POST" '{"zone":5}'

# Test 5: Invalid parameter type (string instead of number)
make_request "Invalid parameter type" "/api/start" "POST" '{"zone":"five","minutes":10}'

# Test 6: Zone out of range (too low)
make_request "Zone out of range (too low)" "/api/start" "POST" '{"zone":0,"minutes":10}'

# Test 7: Zone out of range (too high)
make_request "Zone out of range (too high)" "/api/start" "POST" '{"zone":21,"minutes":10}'

# Test 8: Minutes out of range (too low)
make_request "Minutes out of range (too low)" "/api/start" "POST" '{"zone":5,"minutes":0}'

# Test 9: Minutes out of range (too high)
make_request "Minutes out of range (too high)" "/api/start" "POST" '{"zone":5,"minutes":121}'

# Test 10: Empty JSON object
make_request "Empty JSON object" "/api/start" "POST" '{}'

# Test 11: Empty array instead of object
make_request "Empty array instead of object" "/api/start" "POST" '[]'

# Test 12: Valid stop request
make_request "Valid stop request" "/api/stop" "POST" '{"zone":5}'

# Test 13: Invalid JSON in stop request
make_request "Invalid JSON in stop request" "/api/stop" "POST" '{"zone":5'

# Test 14: Missing zone parameter in stop request
make_request "Missing zone parameter in stop request" "/api/stop" "POST" '{}'

# Test 15: Invalid zone value in stop request
make_request "Invalid zone value in stop request" "/api/stop" "POST" '{"zone":0}'

# Test 16: Zone out of range in stop request
make_request "Zone out of range in stop request" "/api/stop" "POST" '{"zone":21}'

echo -e "\n==============================================="
echo "  API Testing Complete"
echo "==============================================="
