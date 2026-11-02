param(
    [Parameter(Mandatory = $true)][string]$InputPath,
    [Parameter(Mandatory = $true)][string]$OutputPath,
    [int]$VramX = 640,
    [int]$VramY = 0
)

$ErrorActionPreference = 'Stop'

if (-not (Get-Command ffprobe -ErrorAction SilentlyContinue) -or
    -not (Get-Command ffmpeg -ErrorAction SilentlyContinue)) {
    throw 'ffmpeg and ffprobe are required to convert PS1 texture assets.'
}

$dimensions = (& ffprobe -v error -select_streams v:0 -show_entries stream=width,height `
    -of csv=p=0:s=x $InputPath).Trim().Split('x')
if ($dimensions.Count -ne 2) { throw "Unable to read image size: $InputPath" }

$width = [int]$dimensions[0]
$height = [int]$dimensions[1]
if (($width -le 0) -or ($height -le 0) -or (($width % 2) -ne 0)) {
    throw 'PS1 direct-color TIM textures must have a positive even width.'
}

$outputDirectory = Split-Path -Parent $OutputPath
New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
$baseName = [IO.Path]::GetFileNameWithoutExtension($OutputPath)
$colorPath = Join-Path $outputDirectory ($baseName + '.bgr555.tmp')
$alphaPath = Join-Path $outputDirectory ($baseName + '.rgba.tmp')

try {
    # Let ffmpeg perform the exact BGR555 conversion validated on hardware.
    & ffmpeg -y -v error -i $InputPath -pix_fmt bgr555le -f rawvideo $colorPath
    if ($LASTEXITCODE -ne 0) { throw "ffmpeg failed while converting $InputPath" }
    # A separate RGBA pass supplies alpha; PS1 treats a zero texture value as
    # transparent, while opaque colors retain ffmpeg's proven RGB conversion.
    & ffmpeg -y -v error -i $InputPath -pix_fmt rgba -f rawvideo $alphaPath
    if ($LASTEXITCODE -ne 0) { throw "ffmpeg failed while reading alpha from $InputPath" }

    $timPixels = [IO.File]::ReadAllBytes($colorPath)
    $alphaPixels = [IO.File]::ReadAllBytes($alphaPath)
    if ($timPixels.Length -ne ($width * $height * 2) -or
        $alphaPixels.Length -ne ($width * $height * 4)) { throw 'Unexpected converted texture size.' }

    for ($pixel = 0; $pixel -lt ($width * $height); ++$pixel) {
        if ($alphaPixels[$pixel * 4 + 3] -lt 128) {
            $timPixels[$pixel * 2] = 0
            $timPixels[$pixel * 2 + 1] = 0
        } elseif ($timPixels[$pixel * 2] -eq 0 -and $timPixels[$pixel * 2 + 1] -eq 0) {
            # Reserve zero for transparent texels; bit 15 keeps opaque black.
            $timPixels[$pixel * 2 + 1] = 0x80
        }
    }

    $stream = [IO.File]::Open($OutputPath, [IO.FileMode]::Create, [IO.FileAccess]::Write)
    try {
        $writer = [IO.BinaryWriter]::new($stream)
        try {
            $writer.Write([uint32]0x00000010)                 # TIM magic
            $writer.Write([uint32]0x00000002)                 # 16-bit direct color
            $writer.Write([uint32](12 + $timPixels.Length))
            $writer.Write([uint16]$VramX)
            $writer.Write([uint16]$VramY)
            $writer.Write([uint16]$width)                     # Width in 16-bit VRAM words
            $writer.Write([uint16]$height)
            $writer.Write($timPixels)
        } finally { $writer.Dispose() }
    } finally { $stream.Dispose() }
} finally {
    Remove-Item -LiteralPath $colorPath, $alphaPath -Force -ErrorAction SilentlyContinue
}
