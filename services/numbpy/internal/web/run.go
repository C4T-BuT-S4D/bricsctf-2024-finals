package web

import (
	"context"
	"fmt"
	"log"
	"numb/internal/models"
	"os"
	"os/exec"
	"strconv"

	"github.com/gofiber/contrib/websocket"
	"github.com/minio/minio-go/v7"
	"gorm.io/gorm"
)

func (app *App) RunRepl(c *websocket.Conn) {
	runReplInner := func(c *websocket.Conn) error {
		user, ok := c.Locals("user").(*models.User)
		if !ok {
			return ErrUserNotInlocals
		}

		id, err := strconv.Atoi(c.Params("id"))
		if err != nil {
			if err := c.WriteMessage(websocket.BinaryMessage, []byte("id must be int")); err != nil {
				return err
			}
			return nil
		}

		var repl models.Repl

		q := app.db.Take(&repl, "id = ?", id)

		if q.Error != nil {
			if q.Error == gorm.ErrRecordNotFound {
				if err := c.WriteMessage(websocket.BinaryMessage, []byte("repl not found")); err != nil {
					return err
				}
			}
			return fmt.Errorf("taking repl: %v", q.Error)
		}

		if user.ID != repl.UserId {
			if err := c.WriteMessage(websocket.BinaryMessage, []byte("this is not your repl")); err != nil {
				return err
			}
			return nil
		}

		ctx := context.Background()
		reader, err := app.fs.GetObject(ctx, models.ReplBucket, repl.Filename.String(), minio.GetObjectOptions{})
		if err != nil {
			return fmt.Errorf("getting minio reader: %v", err)
		}

		tmpFile, err := os.CreateTemp("", "repl-*.py")
		if err != nil {
			return fmt.Errorf("creating temporary file: %v", err)
		}
		defer os.Remove(tmpFile.Name())

		_, err = tmpFile.ReadFrom(reader)
		if err != nil {
			return fmt.Errorf("copying from reader: %v", err)
		}
		if err := tmpFile.Sync(); err != nil {
			return err
		}

		cmd := exec.Command("timeout", "10", "python", "/app/run_python.py", tmpFile.Name())
		stdin, err := cmd.StdinPipe()
		if err != nil {
			return fmt.Errorf("piping stdin: %v", err)
		}
		stdout, err := cmd.StdoutPipe()
		if err != nil {
			return fmt.Errorf("piping stdin: %v", err)
		}

		writerFunc := func() {
			buf := make([]byte, 1024)

			for {
				n, err := stdout.Read(buf)
				fmt.Println(string(buf))
				if err != nil {
					return
				}
				c.WriteMessage(websocket.BinaryMessage, buf[:n])
			}
		}
		readerFunc := func() {
			_, message, err := c.ReadMessage()
			if err != nil {
				return
			}
			stdin.Write(message)
		}
		cmd.Start()
		go writerFunc()
		go readerFunc()
		if err := cmd.Wait(); err != nil {
			return err
		}

		return nil
	}
	if err := runReplInner(c); err != nil {
		log.Println("got error handling websocket: %w", err)
	}
}
