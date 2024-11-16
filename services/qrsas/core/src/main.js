require("dotenv").config({ path: "app.env" });

const crypto = require("crypto");
const express = require("express");
const { Pool } = require("pg");
const session = require("express-session");
const MemoryStore = require("memorystore")(session);
const grpc = require("@grpc/grpc-js");
require("./proto/auth_pb");
var services = require("./proto/auth_grpc_pb");
const fileUpload = require("express-fileupload");
const Jimp = require("jimp");
var curl = require("curlrequest");
const QrCode = require("qrcode-reader");

const app = express();
const port = process.env.PORT || 3000;

// PostgreSQL connection pool setup
const pool = new Pool({
  user: process.env.PG_USER,
  host: process.env.PG_HOST,
  database: process.env.PG_DATABASE,
  password: process.env.PG_PASSWORD,
  port: process.env.PG_PORT,
});

let authTokensClient = new services.TokenServiceClient(
  process.env.TOKENS_HOST,
  grpc.credentials.createInsecure(),
);

app.use(
  session({
    cookie: { maxAge: 14400000 },
    store: new MemoryStore({
      checkPeriod: 14400000, // prune expired entries every 4h
    }),
    resave: false,
    saveUninitialized: false,
    secret: process.env.SESSION_SECRET,
  }),
);

// Middleware to parse JSON
app.use(express.json());

app.use(
  fileUpload({
    limits: { fileSize: 512 * 1024 },
  }),
);

app.post("/api/signup", async (req, res) => {
  try {
    const { username, password } = req.body;
    if (!username || !password) {
      return res.status(400).json({ error: "Invalid request" });
    }
    const alreadyExists = await pool.query(
      "SELECT * FROM users WHERE username = $1",
      [username],
    );
    if (alreadyExists.rows.length > 0) {
      return res.status(401).json({ error: "User already exists" });
    }
    const newId = await pool.query(
      "INSERT INTO users (username, password) VALUES ($1, $2) RETURNING id",
      [username, password],
    );
    res.status(201).json({ uid: newId.rows[0].id, username: username });
  } catch (error) {
    console.error(error.message);
  }
});

app.post("/api/signin", async (req, res) => {
  try {
    const { username, password } = req.body;
    if (!username || !password) {
      return res.status(400).json({ error: "Invalid request" });
    }
    const user = await pool.query(
      "SELECT * FROM users WHERE username = $1 AND password = $2",
      [username, password],
    );
    if (user.rows.length === 0) {
      return res.status(401).json({ error: "Invalid credentials" });
    }
    let userId = user.rows[0].id;
    req.session.userId = userId.toString();
    res.status(200).json({ uid: userId, username: username });
  } catch (error) {
    console.error(error.message);
  }
});

app.get("/api/signout", async (req, res) => {
  req.session.destroy();
  res.status(200).json({ message: "User signed out" });
});

app.get("/api/tokens", async (req, res) => {
  if (!req.session.userId) {
    return res.status(401).json({ error: "Unauthorized" });
  }

  let userId = req.session.userId;

  let listReq = new proto.ListTokensRequest();
  listReq.setUserId(userId);
  authTokensClient.listTokens(listReq, function (err, response) {
    if (err) {
      console.error(err);
      return res.status(500).json({ error: "Internal server error" });
    }
    let tokens = response.getTokensList().map((token) => {
      return {
        name: token.getName(),
        token: token.getToken(),
      };
    });
    res.status(200).json({ tokens: tokens });
  });
});

app.post("/api/tokens", async (req, res) => {
  if (!req.session.userId) {
    return res.status(401).json({ error: "Unauthorized" });
  }

  let name = req.body.name;
  if (!name) {
    return res.status(400).json({ error: "Invalid request" });
  }

  let userId = req.session.userId;

  let createReq = new proto.CreateTokenRequest();
  createReq.setUserId(userId);
  createReq.setToken(
    new proto.Token()
      .setName(name)
      .setToken(crypto.randomBytes(32).toString("hex")),
  );

  authTokensClient.createToken(createReq, function (err, response) {
    if (err) {
      console.error(err);
      return res.status(500).json({ error: "Internal server error" });
    }
    res.status(201).json({
      token: {
        name: response.getToken().getName(),
        token: response.getToken().getToken(),
      },
    });
  });
});

app.delete("/api/tokens", async (req, res) => {
  if (!req.session.userId) {
    return res.status(401).json({ error: "Unauthorized" });
  }

  let token = req.body.token;

  let invalidateReq = new proto.InvalidateTokenRequest();
  invalidateReq.setToken(token);

  authTokensClient.invalidateToken(invalidateReq, function (err, response) {
    if (err) {
      console.error(err);
      return res.status(500).json({ error: "Internal server error" });
    }
    res.status(200).json({ message: "Token invalidated" });
  });
});

function getToken(token) {
  if (token.startsWith("Bearer ")) {
    return token.slice("Bearer ".length, token.length);
  }
  return "";
}

function validateTokenAsync(token) {
  return new Promise((resolve, reject) => {
    let validateReq = new proto.ValidateTokenRequest();
    validateReq.setToken(token);

    authTokensClient.validateToken(validateReq, function (err, response) {
      if (err) {
        console.error(err);
        reject(err);
        return;
      }

      if (!response.getIsValid()) {
        reject("Unauthorized");
        return;
      }

      resolve(response.getUserId());
    });
  });
}

function decodeQrAsync(bitmap) {
  return new Promise((resolve, reject) => {
    var qr = new QrCode();
    qr.callback = function (err, value) {
      if (err) {
        console.error(err);
        reject(err);
        return;
      }
      resolve(value.result);
    };
    qr.decode(bitmap);
  });
}

function fetchAsync(options) {
  return new Promise((resolve, reject) => {
    curl.request(options, function (err, data) {
      if (err) {
        reject(err);
        return;
      }
      resolve(data);
    });
  });
}

app.post("/api/decode", async (req, res) => {
  let token = getToken(req.get("Authorization"));
  if (!token) {
    return res.status(401).json({ error: "Unauthorized" });
  }

  let userId;
  try {
    userId = await validateTokenAsync(token);
  } catch (error) {
    console.error(error);
    return res.status(401).json({ error: "Unauthorized" });
  }

  if (!req.files?.qr) {
    return res.status(400).json({ error: "No QR code provided" });
  }

  let buffer = req.files.qr.data;
  let content;
  try {
    let imgBuffer = await Jimp.Jimp.read(buffer);
    content = await decodeQrAsync(imgBuffer.bitmap);
  } catch (error) {
    console.error(error);
    return res.status(400).json({ error: "Invalid QR code" });
  }

  let parse = req.query.parse;

  if (URL.canParse(content) && parse) {
    let options = {
      url: content,
      method: "GET",
      timeout: 2,
    };
    try {
      let response = (await fetchAsync(options)) || "";
      content = response.substring(0, 5000);
    } catch (error) {
      console.error(error);
    }
  }

  $uuid = crypto.randomUUID();
  try {
    await pool.query(
      "INSERT INTO QRs (id, user_id, content) VALUES ($1, $2, $3)",
      [$uuid, userId, content],
    );
  } catch (error) {
    console.error(error);
    return res.status(500).json({ error: "Internal server error" });
  }

  res.status(200).json({ content: content, id: $uuid });
});

app.get("/api/results", async (req, res) => {
  let token = getToken(req.get("Authorization"));
  if (!token) {
    return res.status(401).json({ error: "Unauthorized" });
  }

  let userId;
  try {
    userId = await validateTokenAsync(token);
  } catch (error) {
    console.error(error);
    return res.status(401).json({ error: "Unauthorized" });
  }

  let queryRes = await pool.query("SELECT * FROM QRs WHERE user_id = $1", [
    userId,
  ]);

  let results = queryRes.rows.map((row) => {
    return {
      id: row.id,
      content: row.content,
      decoded_at: row.created_at,
    };
  });

  res.status(200).json({ results: results });
});

app.listen(port, () => {
  console.log(`Example app listening on port ${port}`);
});
