(function () {
  var statusEl = document.getElementById("status");
  var consoleEl = document.getElementById("console");
  var invokePingEl = document.getElementById("invokePing");
  var emitEventEl = document.getElementById("emitEvent");

  function writeLine(title, payload) {
    var text = title + "\n" + JSON.stringify(payload, null, 2);
    consoleEl.textContent = text + "\n\n" + consoleEl.textContent;
  }

  var bridge = window.__viewshell;
  if (!bridge) {
    statusEl.textContent = "Bridge bootstrap missing";
    consoleEl.textContent = "window.__viewshell is not available.";
    return;
  }

  statusEl.textContent = "Bridge ready";
  writeLine("bridge:ready", { invoke: true, emit: true });

  window.addEventListener("viewshell:message", function (event) {
    writeLine("native -> page", event.detail);
  });

  invokePingEl.addEventListener("click", function () {
    var payload = { value: 42, from: "hello_viewshell" };
    writeLine("page -> native invoke", { name: "app.ping", payload: payload });
    bridge.invoke("app.ping", payload).then(function (result) {
      writeLine("promise resolved", result);
    }).catch(function (error) {
      writeLine("promise rejected", error);
    });
  });

  emitEventEl.addEventListener("click", function () {
    var payload = { from: "hello_viewshell", at: new Date().toISOString() };
    writeLine("page -> native emit", { name: "native-ready", payload: payload });
    bridge.emit("native-ready", payload);
  });
})();
