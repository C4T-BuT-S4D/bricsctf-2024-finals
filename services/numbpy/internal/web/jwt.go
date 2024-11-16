package web

import (
	"errors"
	"fmt"
	"numb/internal/models"
	"numb/internal/util"
	"strconv"
	"strings"
	"time"

	"github.com/gofiber/fiber/v2"
	"github.com/golang-jwt/jwt/v5"
	"gorm.io/gorm"
)

var (
	ErrUserIDNotFound = errors.New("jwt company_id not found")
	ErrJwtCast        = errors.New("can not cast to JwtClaims")
)

func (app *App) AuthMiddleware(c *fiber.Ctx) error {
	h := c.Get("Authorization")

	if h == "" {
		return util.Fail(c, fiber.StatusUnauthorized, "no bearer token provided")
	}

	// Authorization must be of the form "Bearer <jwt>"
	chunks := strings.Split(h, " ")

	if len(chunks) < 2 || chunks[0] != "Bearer" {
		return util.Fail(c, fiber.StatusUnauthorized, "invalid token form")
	}

	userID, err := app.verifyJWT(chunks[1])
	if err != nil {
		return util.Fail(c, fiber.StatusUnauthorized, err.Error())
	}

	var user models.User

	q := app.db.Preload("Repls").Take(&user, "id = ?", userID)
	if q.Error != nil && !errors.Is(q.Error, gorm.ErrRecordNotFound) {
		return q.Error
	}
	if errors.Is(q.Error, gorm.ErrRecordNotFound) {
		return ErrUserIDNotFound
	}

	c.Locals("user", &user)

	//nolint:wrapcheck
	return c.Next()
}

func (app *App) generateJWT(id uint) (string, error) {
	t := jwt.NewWithClaims(jwt.SigningMethodHS256, jwt.MapClaims{
		"exp": time.Now().Add(time.Minute * 10).Unix(),
		"id":  strconv.FormatUint(uint64(id), 10),
	})

	token, err := t.SignedString(app.secretKey)
	if err != nil {
		return "", fmt.Errorf("signing jwt: %w", err)
	}

	return token, nil
}

func (app *App) verifyJWT(token string) (uint, error) {
	parsed, err := jwt.Parse(token, func(t *jwt.Token) (interface{}, error) {
		if _, ok := t.Method.(*jwt.SigningMethodHMAC); !ok {
			return nil, fmt.Errorf("unexpected signing method: %v", t.Header["alg"])
		}

		return app.secretKey, nil
	})
	if err != nil {
		return 0, fmt.Errorf("verifying jwt: %w", err)
	}

	claims, ok := parsed.Claims.(jwt.MapClaims)
	if !ok {
		return 0, ErrJwtCast
	}

	idStr, ok := claims["id"].(string)
	if !ok {
		return 0, ErrUserIDNotFound
	}

	signedID, err := strconv.Atoi(idStr)
	if err != nil {
		return 0, fmt.Errorf("converting id from jwt to int: %w", err)
	}

	id := uint(signedID)

	return id, nil
}
