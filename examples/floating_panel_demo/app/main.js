(function () {
  var role = window.__demoRole || 'main';

  if (role === 'main') {
    document.body.className = 'main';
    document.body.innerHTML = '<div class="main-shell"><button id="toggleFloating">Toggle Floating Window</button></div>';
    document.getElementById('toggleFloating').addEventListener('click', function () {
      if (window.__viewshell) {
        window.__viewshell.invoke('demo.toggleFloating', {});
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
  var collapseTimer = null;

  function transition(state) {
    if (!window.__viewshell) return;
    window.__viewshell.invoke('demo.floatingTransition', { state: state });
    shell.classList.remove('compact', 'hover', 'expanded');
    shell.classList.add(state);
  }

  function scheduleCollapse() {
    clearTimeout(collapseTimer);
    collapseTimer = setTimeout(function () {
      transition('compact');
    }, 800);
  }

  shell.addEventListener('mouseenter', function () {
    clearTimeout(collapseTimer);
    if (!shell.classList.contains('expanded')) {
      transition('hover');
    }
  });

  shell.addEventListener('mouseleave', function () {
    scheduleCollapse();
  });

  document.body.addEventListener('mouseleave', function () {
    scheduleCollapse();
  });

  document.addEventListener('mouseout', function (event) {
    if (!event.relatedTarget) {
      scheduleCollapse();
    }
  });

  window.addEventListener('blur', function () {
    scheduleCollapse();
  });

  shell.addEventListener('mousedown', function (event) {
    if (event.target === colorToggle) return;
    if (!shell.classList.contains('expanded')) {
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
