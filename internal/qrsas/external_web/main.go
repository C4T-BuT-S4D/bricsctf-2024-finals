package main

import (
	"fmt"
	"net/http"
	"os"
	"sync"
)

var (
	// A simple hashmap to store key-value pairs
	dataStore = make(map[string]string)
	mu        sync.RWMutex // Mutex for safe concurrent access to dataStore
)

func putHandler(w http.ResponseWriter, r *http.Request) {
	// Extract the token and key-value pair from the query string
	query := r.URL.Query()
	token := query.Get("token")
	key := query.Get("key")
	value := query.Get("value")

	// Check token from environment variable
	envToken := os.Getenv("TOKEN")
	if envToken == "" {
		http.Error(w, "Server configuration error: TOKEN is not set", http.StatusInternalServerError)
		return
	}

	// Compare tokens
	if token != envToken {
		http.Error(w, "Unauthorized: token mismatch", http.StatusUnauthorized)
		return
	}

	// Store the key-value pair in the hashmap
	mu.Lock()
	dataStore[key] = value
	mu.Unlock()

	fmt.Fprintf(w, "Stored key: %s with value: %s\n", key, value)
}

func valueHandler(w http.ResponseWriter, r *http.Request) {
	// Extract the key from the URL path
	key := r.URL.Path[1:] // Remove the leading "/"

	// Check if the key exists in the hashmap
	mu.RLock()
	value, exists := dataStore[key]
	mu.RUnlock()

	if exists {
		// Return the value as plain text
		w.Header().Set("Content-Type", "text/plain")
		fmt.Fprint(w, value)
	} else {
		http.NotFound(w, r)
	}
}

func main() {
	// Register handlers
	http.HandleFunc("/put", putHandler)
	http.HandleFunc("/", valueHandler)

	// Start the server on port 5555
	fmt.Println("Server is listening on port 5555...")
	if err := http.ListenAndServe(":5555", nil); err != nil {
		fmt.Println("Error starting server:", err)
	}
}
