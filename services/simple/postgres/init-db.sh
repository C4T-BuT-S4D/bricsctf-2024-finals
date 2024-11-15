#!/bin/sh

psql --username simple --dbname simple  <<-EOSQL
        CREATE TABLE IF NOT EXISTS users (
		id VARCHAR(64) PRIMARY KEY,
		username VARCHAR(64) UNIQUE NOT NULL,
		password VARCHAR(64) NOT NULL
        );

        CREATE TABLE IF NOT EXISTS notes (
		id VARCHAR(64) PRIMARY KEY,
		user_id VARCHAR(64) REFERENCES users(id),
                text VARCHAR(1024)
        );
EOSQL
