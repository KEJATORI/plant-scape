import express from "express";
import http from "http";
import { WebSocketServer } from "ws";

const app = express();
const server = http.createServer(app);
const wss = new WebSocketServer({ server });

app.set('view engine', 'ejs');
app.use(express.static('public'));

app.get("/", (req, res) => {
  res.render("index");
})

wss.on("connection", (ws) => {
  console.log("connected");

  try {
    ws.on("message", (data) => {

      const objectData = JSON.parse(data);
      console.log(objectData);

      wss.clients.forEach((client) => {
        if (client !== ws) {
          client.send(JSON.stringify(objectData));
        }
      });

    })
  } catch (error) {
    console.log(error);
  }

  ws.on("close", () => {
    console.log("Client Disconnected");
  })

})

server.listen(process.env.PORT || 80);