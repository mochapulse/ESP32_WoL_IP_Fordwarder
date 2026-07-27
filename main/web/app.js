function fmtBytes(n) {
  if (n == null) return "--";
  if (n >= 1048576) return (n / 1048576).toFixed(1) + " MB";
  if (n >= 1024) return (n / 1024).toFixed(1) + " kB";
  return n + " B";
}

function fmtUptime(secs) {
  if (secs == null) return "--";
  var h = Math.floor(secs / 3600);
  var m = Math.floor((secs % 3600) / 60);
  var s = secs % 60;
  if (h > 0) return h + "h " + m + "m " + s + "s";
  if (m > 0) return m + "m " + s + "s";
  return s + "s";
}

function fmtFreq(hz) {
  if (hz == null) return "--";
  if (hz >= 1000000) return (hz / 1000000).toFixed(0) + " MHz";
  if (hz >= 1000) return (hz / 1000).toFixed(0) + " kHz";
  return hz + " Hz";
}

function setVal(id, text, cls) {
  var el = document.getElementById(id);
  if (!el) return;
  el.textContent = text;
  if (cls !== undefined) el.className = "value " + cls;
  else el.className = "value";
}

function showError(show) {
  var el = document.getElementById("status-error");
  if (el) el.className = show ? "error-banner" : "error-banner hidden";
}

async function refreshStatus() {
  try {
    var res = await fetch("/api/status");
    if (!res.ok) throw new Error("Not available");
    var data = await res.json();

    showError(false);

    setVal("wifi-status", data.wifi ? "Connected" : "Disconnected",
           data.wifi ? "connected" : "disconnected");
    setVal("lan-ip", data.ip || "--");
    setVal("mac", data.mac || "--");

    setVal("heap-free", fmtBytes(data.heap_free));
    setVal("heap-total", fmtBytes(data.heap_total));
    setVal("heap-min-free", fmtBytes(data.heap_min_free));
    setVal("free-stack", fmtBytes(data.free_stack));
    setVal("task-count", data.task_count != null ? data.task_count : "--");
    setVal("uptime", fmtUptime(data.uptime));

    setVal("chip-model", data.chip_model || "--");
    setVal("chip-cores", data.chip_cores != null ? data.chip_cores : "--");
    setVal("chip-revision", data.chip_revision != null ? "Rev " + data.chip_revision : "--");
    setVal("cpu-freq", fmtFreq(data.cpu_freq));
    setVal("flash-size", fmtBytes(data.flash_size));
    setVal("chip-features", (data.chip_features && data.chip_features.length)
           ? data.chip_features.join(", ") : "--");

    setVal("app-name", data.app_name || "--");
    setVal("app-version", data.app_version || "--");
    setVal("app-build", (data.app_date && data.app_time)
           ? data.app_date + " " + data.app_time : "--");

    var ipEl = document.getElementById("sidebar-ip");
    if (ipEl && data.ip) ipEl.textContent = data.ip;
  } catch {
    showError(true);
  }
}

function switchTab(tabName) {
  document.querySelectorAll(".tab").forEach(function(t) {
    t.classList.remove("active");
  });
  document.querySelectorAll(".menu-item, .bottombar-item").forEach(function(el) {
    if (el.dataset.tab === tabName) el.classList.add("active");
    else el.classList.remove("active");
  });

  var tab = document.getElementById("tab-" + tabName);
  if (tab) tab.classList.add("active");
}

document.addEventListener("DOMContentLoaded", function() {
  document.querySelectorAll(".menu-item, .bottombar-item").forEach(function(el) {
    el.addEventListener("click", function() {
      switchTab(this.dataset.tab);
    });
  });

  refreshStatus();
  setInterval(refreshStatus, 5000);
});
