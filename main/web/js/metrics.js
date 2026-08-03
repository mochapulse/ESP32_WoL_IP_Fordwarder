/* ── Chart configuration ──────────────────────────────────────── */

var MAX_POINTS = 360;
var STORAGE_KEY = "history.v2";
var MAX_AGE_S = 30 * 60;
var MAX_GAP_S = 15;

var METRICS = {
  heap_free:     { label: "Heap Free",     fmt: fmtBytes, color: "#38bdf8", width: 2, dash: null },
  heap_min_free: { label: "Heap Min Free", fmt: fmtBytes, color: "#34d399", width: 1, dash: [6, 4] },
  wifi_rssi:     { label: "Wi-Fi RSSI",    fmt: fmtRssi,  color: "#facc15", width: 2, dash: null },
  task_count:    { label: "Tasks",         fmt: function(v) { return v != null ? String(v) : "--"; }, color: "#c084fc", width: 1, dash: [4, 4] },
  free_stack:    { label: "Free Stack",    fmt: fmtBytes, color: "#fb923c", width: 1, dash: null }
};

var CHARTS = [
  { id: "chart-heap",   keys: ["heap_free", "heap_min_free"], height: 200 },
  { id: "chart-rssi",   keys: ["wifi_rssi"],                   height: 180 },
  { id: "chart-tasks",  keys: ["task_count"],                  height: 180 },
  { id: "chart-stack",  keys: ["free_stack"],                  height: 180 }
];
