(function () {
  document.getElementById("closeBtn").addEventListener("click", function (e) {
    e.stopPropagation();
    window.close();
  });

  var circle = document.querySelector(".circle");
  circle.addEventListener("mousedown", function (e) {
    if (e.target.id === "closeBtn" || e.target.closest("#closeBtn")) return;
    window.webkit.messageHandlers.viewshellDrag.postMessage("drag");
  });
})();
