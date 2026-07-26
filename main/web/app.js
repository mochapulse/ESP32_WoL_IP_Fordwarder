async function refreshStatus() {
  try {
    const res = await fetch("/api/status");
    if (!res.ok) throw new Error("Not available");
    const data = await res.json();
    document.getElementById("wifi-status").textContent = data.wifi ? "Connected" : "Disconnected";
    document.getElementById("wifi-status").className = "value " + (data.wifi ? "connected" : "disconnected");
    document.getElementById("lan-ip").textContent = data.ip || "--";
  } catch {
    document.getElementById("wifi-status").textContent = "Unknown";
    document.getElementById("wifi-status").className = "value";
    document.getElementById("lan-ip").textContent = "--";
  }
}

document.getElementById("refresh-btn").addEventListener("click", refreshStatus);
refreshStatus();
