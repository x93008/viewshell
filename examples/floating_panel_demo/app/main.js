(function () {
  var role = window.__demoRole || 'main';

  if (role === 'main') {
    document.body.className = 'main';
    document.body.innerHTML = '<div class="main-shell"><button id="toggleFloating">Toggle Floating Window</button> <button id="quitBtn" class="quit">Quit</button></div>';
    document.getElementById('toggleFloating').addEventListener('click', function () {
      if (window.__viewshell) {
        window.__viewshell.invoke('demo.toggleFloating', {});
      }
    });
    document.getElementById('quitBtn').addEventListener('click', function () {
      if (window.__viewshell) {
        window.__viewshell.invoke('demo.quit', {});
      }
    });
    return;
  }

  document.body.className = 'floating';
  document.body.innerHTML = '' +
    '<div class="floating-shell compact" id="floatingShell">' +
      '<div class="breathing"></div>' +
      '<div class="floating-label">FLOAT</div>' +
      '<button class="toggle-btn" id="colorToggle">Toggle Color</button>' +
    '</div>';

  var shell = document.getElementById('floatingShell');
  var colorToggle = document.getElementById('colorToggle');
  var currentState = 'compact';

  function transition(state) {
    if (state === currentState) return;
    if (!window.__viewshell) return;
    currentState = state;
    window.__viewshell.invoke('demo.floatingTransition', { state: state });
    shell.classList.remove('compact', 'hover', 'expanded');
    shell.classList.add(state);
  }

  document.addEventListener('mouseenter', function () {
    if (currentState === 'compact') {
      transition('hover');
    }
  });

  document.addEventListener('mouseleave', function () {
    transition('compact');
  });

  shell.addEventListener('mousedown', function (event) {
    if (event.target === colorToggle) return;
    if (currentState !== 'expanded') {
      transition('expanded');
      return;
    }
    if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.viewshellDrag) {
      window.webkit.messageHandlers.viewshellDrag.postMessage('drag');
    }
  });

  colorToggle.addEventListener('click', function (event) {
    event.stopPropagation();
    colorToggle.classList.toggle('blue');
  });
})();
