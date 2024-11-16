package web

import (
	"errors"

	"github.com/minio/minio-go/v7"
	"gorm.io/gorm"
)

type App struct {
	fs        *minio.Client
	db        *gorm.DB
	secretKey []byte
}

var ErrUserNotInlocals = errors.New("expected user in route under widdleware")

func NewApp(fs *minio.Client, db *gorm.DB) *App {
	app := &App{
		fs: fs,
		db: db,
	}
	return app
}
