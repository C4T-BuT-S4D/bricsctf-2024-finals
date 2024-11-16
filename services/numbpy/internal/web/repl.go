package web

import (
	"fmt"
	"io"
	"numb/internal/models"
	"numb/internal/util"
	"strconv"
	"strings"

	"github.com/gofiber/fiber/v2"
	"github.com/google/uuid"
	"github.com/minio/minio-go/v7"
	"gorm.io/gorm"
)

type ListReplsResponse struct {
	ReplIDS []uint `json:"repl_ids"`
}

func (app *App) ListRepls(c *fiber.Ctx) error {
	user, ok := c.Locals("user").(*models.User)
	if !ok {
		return ErrUserNotInlocals
	}

	replIDS := make([]uint, len(user.Repls))
	for _, repl := range user.Repls {
		replIDS = append(replIDS, repl.ID)
	}

	return util.Success(c, fiber.StatusOK, &ListReplsResponse{
		ReplIDS: replIDS,
	})
}

type GetReplCodeResponse struct {
	Code string `json:"code"`
}

func (app *App) GetReplCode(c *fiber.Ctx) error {
	user, ok := c.Locals("user").(*models.User)
	if !ok {
		return ErrUserNotInlocals
	}

	id, err := strconv.Atoi(c.Params("id"))
	if err != nil {
		return util.Fail(c, fiber.StatusBadRequest, "id must be int")
	}

	var repl models.Repl

	q := app.db.Take(&repl, "id = ?", id)

	if q.Error != nil {
		if q.Error == gorm.ErrRecordNotFound {
			return util.Fail(c, fiber.StatusNotFound, "repl not found")
		}
		return fmt.Errorf("taking repl: %w", q.Error)
	}

	if user.ID != repl.UserId {
		return util.Fail(c, fiber.StatusForbidden, "not your repl")
	}

	reader, err := app.fs.GetObject(c.Context(), models.ReplBucket, repl.Filename.String(), minio.GetObjectOptions{})
	if err != nil {
		return fmt.Errorf("getting minio reader: %w", err)
	}

	code, err := io.ReadAll(reader)
	if err != nil {
		return fmt.Errorf("reading from minio: %w", err)
	}

	//nolint:wrapcheck
	return util.Success(c, fiber.StatusOK, &GetReplCodeResponse{
		Code: string(code),
	})
}

type PutReplRequest struct {
	Code string `json:"code"`
}

type PutReplResponse struct {
	ID uint `json:"id"`
}

func (app *App) PutRepl(c *fiber.Ctx) error {
	user, ok := c.Locals("user").(*models.User)
	if !ok {
		return ErrUserNotInlocals
	}

	var body PutReplRequest

	if err := util.ParseBodyAndValidate(c, &body); err != nil {
		return util.Fail(c, fiber.StatusBadRequest, err.Error())
	}

	filename := uuid.New()

	_, err := app.fs.PutObject(
		c.Context(),
		models.ReplBucket,
		filename.String(),
		strings.NewReader(body.Code),
		int64(len(body.Code)),
		minio.PutObjectOptions{},
	)
	if err != nil {
		return fmt.Errorf("uploading file: %w", err)
	}
	repl := models.Repl{
		UserId:   user.ID,
		Filename: filename,
	}

	q := app.db.Save(&repl)

	if q.Error != nil {
		return fmt.Errorf("saving repl to db: %w", q.Error)
	}

	return util.Success(c, fiber.StatusOK, &PutReplResponse{
		ID: repl.ID,
	})
}

type EditReplCodeRequest struct {
	Code string `json:"code"`
}

func (app *App) EditReplCode(c *fiber.Ctx) error {
	user, ok := c.Locals("user").(*models.User)
	if !ok {
		return ErrUserNotInlocals
	}

	id, err := strconv.Atoi(c.Params("id"))
	if err != nil {
		return util.Fail(c, fiber.StatusBadRequest, "id must be int")
	}

	var repl models.Repl

	q := app.db.Take(&repl, "id = ?", id)

	if q.Error != nil {
		if q.Error == gorm.ErrRecordNotFound {
			return util.Fail(c, fiber.StatusNotFound, "repl not found")
		}
		return fmt.Errorf("taking repl: %w", q.Error)
	}

	if user.ID != repl.UserId {
		return util.Fail(c, fiber.StatusForbidden, "not your repl")
	}

	var body EditReplCodeRequest

	if err := util.ParseBodyAndValidate(c, &body); err != nil {
		return util.Fail(c, fiber.StatusBadRequest, err.Error())
	}

	_, err = app.fs.PutObject(c.Context(), models.ReplBucket, repl.Filename.String(), strings.NewReader(body.Code), int64(len(body.Code)), minio.PutObjectOptions{})
	if err != nil {
		return fmt.Errorf("uploading file: %w", err)
	}

	//nolint:wrapcheck
	return util.Success(c, fiber.StatusOK, "ok")
}
