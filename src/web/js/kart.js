export class Kart {
  constructor(app) {
    this.canvas = document.getElementById("canvas");
    this.app = app;

    this.Module = {
      preRun: [],
      postRun: [],
      print: (function () {
        return function (t) {
          const text = Kart.parseEmsText(t);
          console.log(text);
          app.ports.gameOutput.send(text);
        };
      })(),
      printErr: (t) => {
        const text = Kart.parseEmsText(t);
        console.log(text);
        this.app.ports.gameOutput.send(text);
      },
      canvas: this.canvas,
      setStatus: (text) => {
        this.app.ports.statusMessage.send(text);
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

    window.Module = this.Module;
  }

  static parseEmsText(text) {
    if (arguments.length > 1) {
      return Array.prototype.slice.call(arguments).join(" ");
    } else {
      return text;
    }
  }

  setupMethods = () => {
    this.P_AddWadFile = this.Module.cwrap(
      "P_AddWadFile",
      "boolean",
      ["string"],
      ["string"]
    );

    this.Command_ListWADS_f = this.Module.cwrap("Command_ListWADS_f", "void");
  };

  addFile = (filename, base64String) => {
    const data = Uint8Array.from(atob(base64String), (c) => c.charCodeAt(0));
    FS.writeFile(filename, data);

    this.P_AddWadFile(filename);
  };

  requestFullscreen = () => {
    this.canvas.requestFullscreen();
  };

  init = () => {
    const kartScript = document.createElement("script");
    kartScript.setAttribute("src", "srb2kart.js");

    kartScript.setAttribute("type", "text/javascript");
    kartScript.setAttribute("async", true);
    kartScript.addEventListener("load", () => {
      this.setupMethods();
    });

    document.body.appendChild(kartScript);
  };
}
