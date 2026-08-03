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

function fmtRssi(dBm) {
  if (dBm == null) return "--";
  return dBm + " dBm";
}
