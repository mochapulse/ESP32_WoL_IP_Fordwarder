/* ── History ring buffer ──────────────────────────────────────── */

var histData = { ts: [] };
Object.keys(METRICS).forEach(function(k) { histData[k] = []; });

try { localStorage.removeItem("history.v2"); } catch(e) {}

function pushSample(data) {
  if (!histData || !histData.ts || !Array.isArray(histData.ts)) {
    console.warn("histData corrupt, resetting");
    histData = { ts: [] };
    Object.keys(METRICS).forEach(function(k) { histData[k] = []; });
  }
  var keys = Object.keys(METRICS);
  for (var ki = 0; ki < keys.length; ki++) {
    if (!Array.isArray(histData[keys[ki]])) {
      console.warn("histData." + keys[ki] + " missing, resetting");
      histData = { ts: [] };
      Object.keys(METRICS).forEach(function(k) { histData[k] = []; });
      break;
    }
  }
  histData.ts.push(Date.now() / 1000);
  keys.forEach(function(k) {
    var v = data[k];
    histData[k].push(v != null ? v : null);
  });
  if (histData.ts.length > MAX_POINTS) {
    histData.ts.shift();
    keys.forEach(function(k) { histData[k].shift(); });
  }
  scheduleSave();
}

var saveTimer = null;
function scheduleSave() {
  if (saveTimer) return;
  saveTimer = setTimeout(function() {
    saveHistory();
    saveTimer = null;
  }, 30000);
}

function saveHistory() {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(histData));
  } catch (e) { /* storage full/blocked */ }
}

function loadHistory() {
  var raw;
  try { raw = localStorage.getItem(STORAGE_KEY); } catch (e) { return; }
  if (!raw) return;

  var saved;
  try { saved = JSON.parse(raw); } catch (e) { return; }
  if (!saved || !Array.isArray(saved.ts)) return;

  var keys = Object.keys(METRICS);
  for (var ki = 0; ki < keys.length; ki++) {
    if (!Array.isArray(saved[keys[ki]])) return;
  }

  var cutoff = Date.now() / 1000 - MAX_AGE_S;
  for (var i = 0; i < saved.ts.length; i++) {
    var t = saved.ts[i];
    if (typeof t !== "number" || t < cutoff) continue;
    if (histData.ts.length && t - histData.ts[histData.ts.length - 1] > MAX_GAP_S) {
      histData.ts.push(t - 1);
      for (var ki2 = 0; ki2 < keys.length; ki2++) histData[keys[ki2]].push(null);
    }
    histData.ts.push(t);
    for (var ki3 = 0; ki3 < keys.length; ki3++) histData[keys[ki3]].push(saved[keys[ki3]][i]);
    if (histData.ts.length > MAX_POINTS) {
      histData.ts.shift();
      for (var ki4 = 0; ki4 < keys.length; ki4++) histData[keys[ki4]].shift();
    }
  }
}
