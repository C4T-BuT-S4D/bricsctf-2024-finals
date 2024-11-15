package util

import (
	"fmt"
	"log"

	"github.com/go-playground/validator/v10"
	"github.com/gofiber/fiber/v2"
)

type Status struct {
	Code    int         `json:"code"`
	Status  string      `json:"status"`
	Message interface{} `json:"message"`
}

func Validate(payload interface{}) error {
	validate := validator.New()

	err := validate.RegisterValidation("password", func(fl validator.FieldLevel) bool {
		l := len(fl.Field().String())

		return l >= 8 && l < 60
	})
	if err != nil {
		log.Fatal(err)
	}

	err = validate.Struct(payload)
	if err != nil {
		return fmt.Errorf("validating struct: %w", err)
	}

	return nil
}

func ParseBodyAndValidate(c *fiber.Ctx, body interface{}) error {
	if err := c.BodyParser(body); err != nil {
		return fiber.ErrBadRequest
	}

	return Validate(body)
}

func Fail(c *fiber.Ctx, code int, message interface{}) error {
	return c.Status(code).JSON(&Status{
		Code:    code,
		Status:  "error",
		Message: message,
	})
}

func Success(c *fiber.Ctx, code int, message interface{}) error {
	return c.Status(code).JSON(&Status{
		Code:    code,
		Status:  "success",
		Message: message,
	})
}

func ErrorHandler(c *fiber.Ctx, err error) error {
	log.Printf("got error: %v\n", err)
	return Fail(c, fiber.StatusInternalServerError, err.Error())
}
