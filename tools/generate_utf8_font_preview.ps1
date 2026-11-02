Add-Type -AssemblyName System.Drawing

$characters = @(
    ' ', '!', '"', '#', '$', '%', '&', "'", '(', ')', '*', '+', ',', '-', '.', '/',
    '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?','@',
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    '[','\',']','^','_','`',
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
    '{','|','}','~',
    'À','Á','Â','Ã','Ä','Å','Æ','Ç','È','É','Ê','Ë','Ì','Í','Î','Ï','Ð','Ñ','Ò','Ó','Ô','Õ','Ö','Ø','Ù','Ú','Û','Ü','Ý','Þ','ß',
    'à','á','â','ã','ä','å','æ','ç','è','é','ê','ë','ì','í','î','ï','ð','ñ','ò','ó','ô','õ','ö','ø','ù','ú','û','ü','ý','þ','ÿ',
    'А','Б','В','Г','Д','Е','Ё','Ж','З','И','Й','К','Л','М','Н','О','П','Р','С','Т','У','Ф','Х','Ц','Ч','Ш','Щ','Ъ','Ы','Ь','Э','Ю','Я',
    'а','б','в','г','д','е','ё','ж','з','и','й','к','л','м','н','о','п','р','с','т','у','ф','х','ц','ч','ш','щ','ъ','ы','ь','э','ю','я'
)

function Write-Atlas($cellWidth, $cellHeight, $columns, $fontSize, $outputName) {
    $rows = [int][Math]::Ceiling($characters.Count / $columns)
    $bitmap = [System.Drawing.Bitmap]::new($columns * $cellWidth, $rows * $cellHeight, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::SingleBitPerPixelGridFit
    $graphics.Clear([System.Drawing.Color]::Transparent)
    $font = [System.Drawing.Font]::new('Consolas', $fontSize, [System.Drawing.FontStyle]::Regular, [System.Drawing.GraphicsUnit]::Pixel)
    $brush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::White)
    for ($index = 0; $index -lt $characters.Count; ++$index) {
        $x = ($index % $columns) * $cellWidth
        $y = [Math]::Floor($index / $columns) * $cellHeight
        $graphics.DrawString($characters[$index], $font, $brush, $x, $y)
    }
    $out = Join-Path $PSScriptRoot ("..\assets\Fonts\$outputName")
    $bitmap.Save($out, [System.Drawing.Imaging.ImageFormat]::Png)
    $brush.Dispose(); $font.Dispose(); $graphics.Dispose(); $bitmap.Dispose()
}

function Write-JeuDePixelAtlas {
    $cellWidth = 16; $cellHeight = 16; $columns = 64
    $rows = [int][Math]::Ceiling($characters.Count / $columns)
    $bitmap = [System.Drawing.Bitmap]::new($columns * $cellWidth, $rows * $cellHeight, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::SingleBitPerPixelGridFit
    $graphics.Clear([System.Drawing.Color]::Transparent)
    $collection = [System.Drawing.Text.PrivateFontCollection]::new()
    $collection.AddFontFile((Join-Path $PSScriptRoot '..\assets\Fonts\JeuDePixel-Regular.otf'))
    $font = [System.Drawing.Font]::new($collection.Families[0], 14, [System.Drawing.FontStyle]::Regular, [System.Drawing.GraphicsUnit]::Pixel)
    $brush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::White)
    for ($index = 0; $index -lt $characters.Count; ++$index) {
        $x = ($index % $columns) * $cellWidth
        $y = [Math]::Floor($index / $columns) * $cellHeight
        $graphics.DrawString($characters[$index], $font, $brush, $x, $y)
    }
    $bitmap.Save((Join-Path $PSScriptRoot '..\assets\Fonts\jeu_de_pixel_utf8_preview.png'), [System.Drawing.Imaging.ImageFormat]::Png)
    $brush.Dispose(); $font.Dispose(); $collection.Dispose(); $graphics.Dispose(); $bitmap.Dispose()
}

Write-Atlas 16 18 64 11 'pixel_dungeon_utf8_preview.png'
Write-Atlas 8 9 128 7 'font1x_utf8_preview.png'
Write-JeuDePixelAtlas

$source = [System.Drawing.Bitmap]::new((Join-Path $PSScriptRoot '..\assets\Fonts\font2x.png'))
$downscaled = [System.Drawing.Bitmap]::new([int]($source.Width / 2), [int]($source.Height / 2), [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
$downscaleGraphics = [System.Drawing.Graphics]::FromImage($downscaled)
$downscaleGraphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor
$downscaleGraphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::Half
$downscaleGraphics.DrawImage($source, 0, 0, $downscaled.Width, $downscaled.Height)
$downscaled.Save((Join-Path $PSScriptRoot '..\assets\Fonts\font1x_from_font2_preview.png'), [System.Drawing.Imaging.ImageFormat]::Png)
$downscaleGraphics.Dispose(); $downscaled.Dispose(); $source.Dispose()
