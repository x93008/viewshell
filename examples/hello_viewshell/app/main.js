(function () {
  var statusEl = document.getElementById("status");
  var consoleEl = document.getElementById("console");
  var invokePingEl = document.getElementById("invokePing");
  var invokeFailEl = document.getElementById("invokeFail");
  var emitEventEl = document.getElementById("emitEvent");
  var subscribeEventEl = document.getElementById("subscribeEvent");
  var unsubscribeEventEl = document.getElementById("unsubscribeEvent");
  var reloadPageEl = document.getElementById("reloadPage");
  var unsubscribeNativeReady = null;

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
  writeLine("bridge:ready", { invoke: true, emit: true, on: true, off: true });
  writeLine("init-script", {
    value: window.__viewshellInitScriptRan || "missing"
  });

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

  invokeFailEl.addEventListener("click", function () {
    var payload = { from: "hello_viewshell" };
    writeLine("page -> native invoke", { name: "app.fail", payload: payload });
    bridge.invoke("app.fail", payload).then(function (result) {
      writeLine("unexpected resolve", result);
    }).catch(function (error) {
      writeLine("promise rejected", error);
    });
  });

  emitEventEl.addEventListener("click", function () {
    var payload = { from: "hello_viewshell", at: new Date().toISOString() };
    writeLine("page -> native emit", { name: "native-ready", payload: payload });
    bridge.emit("native-ready", payload);
  });

  subscribeEventEl.addEventListener("click", function () {
    if (unsubscribeNativeReady) {
      writeLine("subscription", { name: "native-ready", status: "already subscribed" });
      return;
    }
    unsubscribeNativeReady = bridge.on("native-ready", function (payload) {
      writeLine("native event listener", { name: "native-ready", payload: payload });
    });
    writeLine("subscription", { name: "native-ready", status: "subscribed" });
  });

  unsubscribeEventEl.addEventListener("click", function () {
    if (!unsubscribeNativeReady) {
      writeLine("subscription", { name: "native-ready", status: "already unsubscribed" });
      return;
    }
    unsubscribeNativeReady();
    unsubscribeNativeReady = null;
    writeLine("subscription", { name: "native-ready", status: "unsubscribed" });
  });

  reloadPageEl.addEventListener("click", function () {
    writeLine("page", { action: "reload" });
    window.location.reload();
  });
})();
