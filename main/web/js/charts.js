/* ── uPlot chart setup ────────────────────────────────────────── */

var charts = [];

function makeChartOpts(container, chartDef) {
  var axisStyle = {
    stroke: "#64748b",
    grid:   { stroke: "#334155", width: 1 },
    ticks:  { stroke: "#334155", width: 1 }
  };

  var fmt = METRICS[chartDef.keys[0]].fmt;

  var series = [{}];
  chartDef.keys.forEach(function(k) {
    var m = METRICS[k];
    series.push({
      label:  m.label,
      stroke: m.color,
      width:  m.width,
      dash:   m.dash,
      points: { show: false }
    });
  });

  return {
    width:  container.clientWidth,
    height: chartDef.height,
    legend: { show: true, live: true },
    cursor: { show: true },
    scales: {
      x: {
        time: true,
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
          if (dataMin == null) return [0, 100];
          return uPlot.rangeNum(dataMin, dataMax, 0.1, true);
        }
      }
    },
    axes: [
      axisStyle,
      Object.assign({}, axisStyle, {
        values: function(self, splits) {
          return splits.map(function(v) { return fmt(v); });
        }
      })
    ],
    series: series
  };
}

function initCharts() {
  if (typeof uPlot === "undefined") return;

  CHARTS.forEach(function(chartDef) {
    var container = document.getElementById(chartDef.id);
    if (!container) return;

    var opts = makeChartOpts(container, chartDef);
    var ch = new uPlot(opts, null, container);
    charts.push({ inst: ch, id: chartDef.id, keys: chartDef.keys, height: chartDef.height });
  });

  window.addEventListener("resize", function() {
    charts.forEach(function(ch) {
      var el = document.getElementById(ch.id);
      if (el) ch.inst.setSize({ width: el.clientWidth, height: ch.height });
    });
  });
}

function updateCharts(data) {
  if (!charts.length) return;
  try {
    pushSample(data);
    charts.forEach(function(ch) {
      var arrs = [histData.ts];
      ch.keys.forEach(function(k) { arrs.push(histData[k]); });
      ch.inst.setData(arrs);
      var latest = {};
      ch.keys.forEach(function(k) { latest[k] = histData[k][histData[k].length - 1]; });
      console.log("chart " + ch.id + " updated:", JSON.stringify(latest));
    });
  } catch (e) {
    console.error("chart update failed:", e);
  }
}
