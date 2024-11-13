package models

import (
	"github.com/google/uuid"
	"gorm.io/gorm"
)

type User struct {
	gorm.Model
	Login    string `gorm:"uniqueIndex"`
	Password string
	Repls    []Repl
}

type Repl struct {
	gorm.Model
	UserId   uint
	Filename uuid.UUID
}

const ReplBucket = "repls"
