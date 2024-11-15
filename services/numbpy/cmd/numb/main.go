package main

import (
	"context"
	"log"
	"numb/internal/models"
	"numb/internal/util"
	"numb/internal/web"

	"github.com/gofiber/contrib/websocket"
	"github.com/gofiber/fiber/v2"
	"github.com/gofiber/fiber/v2/middleware/cors"
	"github.com/minio/minio-go/v7"
	"github.com/minio/minio-go/v7/pkg/credentials"
	"github.com/sirupsen/logrus"
	"gorm.io/driver/postgres"
	"gorm.io/gorm"
	"gorm.io/gorm/logger"
)

func main() {
	app := fiber.New(fiber.Config{
		ErrorHandler: util.ErrorHandler,
	})

	ctx := context.Background()

	webApp := web.NewApp(setupMinio(ctx), setupDB())
	app.Use(cors.New())

	api := app.Group("api")

	api.Use("/run", func(c *fiber.Ctx) error {
		// IsWebSocketUpgrade returns true if the client
		// requested upgrade to the WebSocket protocol.
		if websocket.IsWebSocketUpgrade(c) {
			c.Locals("allowed", true)
			return c.Next()
		}
		return fiber.ErrUpgradeRequired
	})

	api.Post("/register", webApp.Register)
	api.Post("/login", webApp.Login)

	api.Get("/repls", webApp.AuthMiddleware, webApp.ListRepls)
	api.Get("/repl/:id", webApp.AuthMiddleware, webApp.GetReplCode)
	api.Patch("/repl/:id", webApp.AuthMiddleware, webApp.EditReplCode)
	api.Put("/repl", webApp.AuthMiddleware, webApp.PutRepl)

	api.Get("/run/:id", webApp.AuthMiddleware, websocket.New(webApp.RunRepl))

	api.Use(func(c *fiber.Ctx) error {
		return util.Fail(c, fiber.StatusNotFound, "not found")
	})

	log.Fatal(app.Listen(":7117"))
}

func setupMinio(ctx context.Context) *minio.Client {
	fs, err := minio.New("minio:9000", &minio.Options{
		Creds: credentials.NewStaticV4("numb", "6whMQ9fhHsaezmVr", ""),
	},
	)
	if err != nil {
		logrus.Fatalf("could not connect to minio with err %v", err)
	}

	makeBucketIfNotExists := func(bucketName string) {
		if bucketExists, err := fs.BucketExists(ctx, bucketName); err != nil {
			logrus.Fatalf("could not check if bucket exists with %v", err)
		} else if !bucketExists {
			if err := fs.MakeBucket(ctx, bucketName, minio.MakeBucketOptions{}); err != nil {
				logrus.Fatalf("could not create minio bucket with %v", err)
			}
		}
	}

	makeBucketIfNotExists(models.ReplBucket)

	return fs
}

func setupDB() *gorm.DB {
	dsn := "host=postgres user=numb password=numb dbname=numb port=5432 sslmode=disable"

	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{
		Logger: logger.Default.LogMode(logger.Info),
	})
	if err != nil {
		log.Fatal("Failed to connect to database. \n", err)
	}

	log.Println("connected to database")
	db.Logger = logger.Default.LogMode(logger.Info)

	log.Println("running migrations")
	err = db.AutoMigrate(&models.User{}, &models.Repl{})
	if err != nil {
		log.Fatal(err)
	}

	return db
}
