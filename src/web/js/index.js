import { Elm } from "../elm/Main";
import { Kart } from "./kart";
import "../style.css";

const app = Elm.Main.init({ node: document.getElementById("main") });
const kart = new Kart(app);

app.ports.startGame.subscribe(kart.init);
app.ports.listWads.subscribe(() => kart.Command_ListWADS_f());
app.ports.requestFullScreen.subscribe(() => kart.requestFullscreen());
app.ports.addFile.subscribe(([filename, base64]) =>
  kart.addFile(filename, base64)
);
