import { Elm } from "../elm/Main";
import "../style.css";

const app = Elm.Main.init({ node: document.getElementById("main") });

app.ports.startGame.subscribe((message) => {
  let scriptEle = document.createElement("script");
  scriptEle.setAttribute("src", "srb2kart.js");

  scriptEle.setAttribute("type", "text/javascript");
  scriptEle.setAttribute("async", true);

  document.body.appendChild(scriptEle);
});

const parseEmsText = function (text) {
  if (arguments.length > 1) {
    return Array.prototype.slice.call(arguments).join(" ");
  } else {
    return text;
  }
};

window.Module = {
  preRun: [],
  postRun: [],
  print: (function () {
    return function (t) {
      const text = parseEmsText(t);
      app.ports.gameOutput.send(text);
    };
  })(),
  printErr: function (t) {
    const text = parseEmsText(t);
    app.ports.gameOutput.send(text);
  },
  canvas: (function () {
    var canvas = document.getElementById("canvas");

    // As a default initial behavior, pop up an alert when webgl context is lost. To make your
    // application robust, you may want to override this behavior before shipping!
    // See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
    canvas.addEventListener(
      "webglcontextlost",
      function (e) {
        alert("WebGL context lost. You will need to reload the page.");
        e.preventDefault();
      },
      false
    );

    return canvas;
  })(),
  setStatus: function (text) {
    app.ports.statusMessage.send(text);
  },
  totalDependencies: 1,
  monitorRunDependencies: function (left) {
    // console.log("LEFT", left);
    // this.totalDependencies = Math.max(this.totalDependencies, left);
    // Module.setStatus(
    //   left
    //     ? "Preparing... (" +
    //         (this.totalDependencies - left) +
    //         "/" +
    //         this.totalDependencies +
    //         ")"
    //     : "All downloads complete."
    // );
  },
};
