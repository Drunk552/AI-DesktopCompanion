# stream_h264.ps1 - FFmpeg H.264 RTP to WSL; SDP at /tmp/stream.sdp
# 使用 H.264 编码，兼容 OpenCV 4.5.4
$ErrorActionPreference = 'Stop'

# ===== Config =====
$Distro    = 'Ubuntu-22.04'
$Device    = 'Integrated Camera'
# $Device    = 'USB Camera'
# $Device    = 'Nantian Camera 8713'
$VideoSize = '640x480'
$Framerate = 30
$Port      = 5004
# ===================

# 获取WSL IP
$WSL_IP = $null
$ipLine = (wsl -d $Distro hostname -I) -as [string]
if ($ipLine) {
  $WSL_IP = ($ipLine -split '\s+') | Where-Object { $_ -match '^\d{1,3}(\.\d{1,3}){3}$' } | Select-Object -First 1
}

Write-Host "WSL IP: $WSL_IP"
Write-Host "开始 H.264 推流到 WSL..."

ffmpeg -f dshow -video_size $VideoSize -framerate $Framerate -pixel_format yuyv422 `
  -rtbufsize 8M -thread_queue_size 8 `
  -i video=$Device `
  -c:v libx264 -preset ultrafast -tune zerolatency -crf 18 `
  -g 30 -keyint_min 30 -sc_threshold 0 `
  -b:v 2M -maxrate 2M -bufsize 1M `
  -pix_fmt yuv420p `
  -fflags nobuffer -flags low_delay -max_delay 0 `
  -f rtp -payload_type 96 `
  -sdp_file \\wsl.localhost\$Distro\tmp\stream.sdp `
  "rtp://$($WSL_IP):${Port}?pkt_size=1200"
