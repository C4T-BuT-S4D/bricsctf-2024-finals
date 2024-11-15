package web

import (
	"errors"
	"fmt"
	"numb/internal/models"
	"numb/internal/util"
	"strings"

	"github.com/gofiber/fiber/v2"
	"gorm.io/gorm"
)

type LoginRequest struct {
	Login    string `json:"login" validate:"required"`
	Password string `json:"password" validate:"required"`
}

type LoginResponse struct {
	Token string `json:"token"`
}

func (app *App) Login(c *fiber.Ctx) error {
	var body LoginRequest

	if err := util.ParseBodyAndValidate(c, &body); err != nil {
		return util.Fail(c, fiber.StatusBadRequest, err.Error())
	}

	var user models.User
	q := app.db.Take(&user, "login = ?", body.Login)
	if q.Error != nil && !errors.Is(q.Error, gorm.ErrRecordNotFound) {
		return q.Error
	}
	if q.Error != nil && errors.Is(q.Error, gorm.ErrRecordNotFound) || user.Password != body.Password {
		return util.Fail(c, fiber.StatusNotFound, "login or password incorrect")
	}

	token, err := app.generateJWT(user.ID)
	if err != nil {
		return err
	}
	return util.Success(c, fiber.StatusOK, &LoginResponse{
		Token: token,
	})
}

type RegisterRequest struct {
	Login    string `json:"login" validate:"required"`
	Password string `json:"password" validate:"required"`
}

type RegisterResponse struct {
	Token string `json:"token"`
}

func (app *App) Register(c *fiber.Ctx) error {
	var body RegisterRequest

	if err := util.ParseBodyAndValidate(c, &body); err != nil {
		return util.Fail(c, fiber.StatusBadRequest, err.Error())
	}

	company := models.User{
		Login:    body.Login,
		Password: body.Password,
	}

	q := app.db.Create(&company)

	if q.Error != nil {
		if strings.Contains(q.Error.Error(), "duplicate key value violates unique constraint") {
			return util.Fail(c, fiber.StatusBadRequest, "user with that login already exists")
		} else {
			return fmt.Errorf("creating user: %w", q.Error)
		}
	}

	token, err := app.generateJWT(company.ID)
	if err != nil {
		return err
	}
	return util.Success(c, fiber.StatusOK, &RegisterResponse{
		Token: token,
	})
}
