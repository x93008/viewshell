(function () {
  document.getElementById("closeBtn").addEventListener("click", function (e) {
    e.stopPropagation();
    if (window.__viewshellWinClose) {
      window.__viewshellWinClose();
      return;
    }
    window.close();
  });

  var circle = document.querySelector(".circle");
  circle.addEventListener("mousedown", function (e) {
    if (e.target.id === "closeBtn" || e.target.closest("#closeBtn")) return;
    if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.viewshellDrag) {
      window.webkit.messageHandlers.viewshellDrag.postMessage("drag");
    }
  });
})();
