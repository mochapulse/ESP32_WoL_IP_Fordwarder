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

/* ── Memory history chart (uPlot) ─────────────────────────────── */

var MAX_POINTS = 360; // 30 min at 5 s poll interval
var STORAGE_KEY = "heapHistory.v1";
var MAX_AGE_S = 30 * 60;   // drop restored points older than this
var MAX_GAP_S = 15;        // >3 missed polls = line break (null gap)

var chart = null;
var history = { ts: [], free: [], min: [] };
var API_TOKEN_KEY = "webApiToken.v1";
var apiToken = "";
var tokenInput = null;
var tokenToggleBtn = null;
var tokenChangeBtn = null;
var tokenForgetBtn = null;
var isTokenEditing = false;

function loadApiToken() {
  try {
    var saved = localStorage.getItem(API_TOKEN_KEY);
    apiToken = saved || "";
  } catch (e) {
    apiToken = "";
  }
}

function persistApiToken(value) {
  apiToken = value || "";
  try {
    if (apiToken) localStorage.setItem(API_TOKEN_KEY, apiToken);
    else localStorage.removeItem(API_TOKEN_KEY);
  } catch (e) { /* storage unavailable */ }
}

function authHeaders() {
  if (!apiToken) return {};
  return { "X-API-Key": apiToken };
}

function setTokenInputMode(editing) {
  if (!tokenInput || !tokenChangeBtn) return;
  isTokenEditing = !!editing;
  tokenInput.disabled = !isTokenEditing;
  tokenChangeBtn.textContent = isTokenEditing ? "Save token" : "Change token";
  if (isTokenEditing) {
    tokenInput.focus();
    tokenInput.select();
  }
}

function setTokenVisibility(show) {
  if (!tokenInput || !tokenToggleBtn) return;
  tokenInput.type = show ? "text" : "password";
  tokenToggleBtn.textContent = show ? "Hide key" : "Show key";
}

function pushSample(heapFree, heapMinFree) {
  history.ts.push(Date.now() / 1000);
  history.free.push(heapFree);
  history.min.push(heapMinFree);
  if (history.ts.length > MAX_POINTS) {
    history.ts.shift();
    history.free.shift();
    history.min.shift();
  }
}

function saveHistory() {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(history));
  } catch (e) { /* storage full/blocked — history just won't persist */ }
}

function loadHistory() {
  var raw;
  try {
    raw = localStorage.getItem(STORAGE_KEY);
  } catch (e) { return; }
  if (!raw) return;

  var saved;
  try {
    saved = JSON.parse(raw);
  } catch (e) { return; }
  if (!saved || !Array.isArray(saved.ts) || !Array.isArray(saved.free) ||
      !Array.isArray(saved.min)) return;

  var cutoff = Date.now() / 1000 - MAX_AGE_S;
  for (var i = 0; i < saved.ts.length; i++) {
    var t = saved.ts[i];
    if (typeof t !== "number" || t < cutoff) continue;
    // Break the line where the device was unreachable (gap in capture)
    if (history.ts.length && t - history.ts[history.ts.length - 1] > MAX_GAP_S) {
      history.ts.push(t - 1);
      history.free.push(null);
      history.min.push(null);
    }
    history.ts.push(t);
    history.free.push(saved.free[i]);
    history.min.push(saved.min[i]);
    if (history.ts.length > MAX_POINTS) {
      history.ts.shift();
      history.free.shift();
      history.min.shift();
    }
  }
}

function initChart() {
  if (typeof uPlot === "undefined") return; // lib failed to load — skip
  var container = document.getElementById("heap-chart");
  if (!container) return;

  var axisStyle = {
    stroke: "#64748b",
    grid:   { stroke: "#334155", width: 1 },
    ticks:  { stroke: "#334155", width: 1 }
  };

  var opts = {
    width:  container.clientWidth,
    height: 220,
    legend: { show: true, live: true },
    cursor: { show: true },
    scales: {
      x: {
        time: true,
        // no-data pattern from uPlot demos/no-data.html: supply default
        // ranges while history is empty (dataMin == null before setData)
        range: function(u, dataMin, dataMax) {
          if (dataMin == null) {
            var now = Date.now() / 1000;
            return [now - 300, now];
          }
          if (dataMin === dataMax) return [dataMin - 150, dataMax + 150];
          return [dataMin, dataMax];
        }
      },
      y: {
        range: function(u, dataMin, dataMax) {
          if (dataMin == null) return [0, 360 * 1024];
          return uPlot.rangeNum(dataMin, dataMax, 0.1, true);
        }
      }
    },
    axes: [
      axisStyle,
      Object.assign({}, axisStyle, {
        values: function(self, splits) {
          return splits.map(function(v) { return fmtBytes(v); });
        }
      })
    ],
    series: [
      {},
      {
        label: "Heap Free",
        stroke: "#38bdf8",
        width: 2,
        points: { show: false }
      },
      {
        label: "Heap Min Free",
        stroke: "#34d399",
        width: 1,
        dash: [6, 4],
        points: { show: false }
      }
    ]
  };

  // Init with null data — [[],[],[]] crashes uPlot's constructor.
  chart = new uPlot(opts, null, container);

  window.addEventListener("resize", function() {
    chart.setSize({ width: container.clientWidth, height: 220 });
  });
}

function updateChart(data) {
  if (!chart || data.heap_free == null) return;
  // Isolated from refreshStatus()'s try/catch — a chart error must never
  // surface as "Failed to reach device" (that banner means fetch failed).
  try {
    pushSample(data.heap_free, data.heap_min_free);
    chart.setData([history.ts, history.free, history.min]);
  } catch (e) {
    console.error("chart update failed:", e);
  }
}

async function refreshStatus() {
  try {
    var res = await fetch("/api/status", { headers: authHeaders() });
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

    updateChart(data);
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
  loadApiToken();

  tokenInput = document.getElementById("api-token");
  tokenToggleBtn = document.getElementById("toggle-api-token");
  tokenChangeBtn = document.getElementById("change-api-token");
  tokenForgetBtn = document.getElementById("forget-api-token");

  if (tokenInput) {
    tokenInput.value = apiToken;
    tokenInput.addEventListener("keydown", function(evt) {
      if (evt.key === "Enter" && isTokenEditing) {
        persistApiToken(this.value.trim());
        setTokenInputMode(false);
        refreshStatus();
      }
    });
    setTokenInputMode(!apiToken);
  }

  if (tokenToggleBtn) {
    tokenToggleBtn.addEventListener("click", function() {
      setTokenVisibility(tokenInput && tokenInput.type === "password");
    });
  }
  setTokenVisibility(false);

  if (tokenChangeBtn) {
    tokenChangeBtn.addEventListener("click", function() {
      if (!tokenInput) return;
      if (!isTokenEditing) {
        setTokenInputMode(true);
        return;
      }
      persistApiToken(tokenInput.value.trim());
      setTokenInputMode(false);
      refreshStatus();
    });
  }

  if (tokenForgetBtn) {
    tokenForgetBtn.addEventListener("click", function() {
      persistApiToken("");
      if (tokenInput) tokenInput.value = "";
      setTokenInputMode(true);
      setTokenVisibility(false);
      showError(true);
    });
  }

  document.querySelectorAll(".menu-item, .bottombar-item").forEach(function(el) {
    el.addEventListener("click", function() {
      switchTab(this.dataset.tab);
    });
  });

  initChart();
  refreshStatus();
  setInterval(refreshStatus, 5000);
});
