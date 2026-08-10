<#
.SYNOPSIS
  Downloads GGUF roleplay-chat models from Hugging Face for use with LlamaStories' Llama.cpp tab.

.DESCRIPTION
  Ships with a short curated list of GGUF models known to work well for in-character roleplay
  and to fit comfortably on a machine with a 12GB GPU (see recommended_models.html for the
  reasoning behind the list and quant choice). You can also pass an arbitrary direct .gguf URL.

  This script only fetches model weights. It does not install llama.cpp itself - grab
  llama-server.exe / llama-cli.exe (CUDA build, for an NVIDIA GPU) from
  https://github.com/ggml-org/llama.cpp/releases and point the app's Llama.cpp tab at them.

.PARAMETER List
  Show the curated model list and exit.

.PARAMETER Key
  Which curated model to download (see -List).

.PARAMETER Quant
  Which quantisation of that model to fetch (see -List for what's available). Default Q4_K_M.

.PARAMETER Url
  A direct .gguf download URL to use instead of a curated -Key (e.g. any other Hugging Face
  "resolve/main/...gguf" link).

.PARAMETER OutDir
  Folder to save the model into. Defaults to this script's folder, which is also a sensible
  "Models folder" to point the app at.

.EXAMPLE
  .\Download-LlamaModel.ps1 -List

.EXAMPLE
  .\Download-LlamaModel.ps1 -Key nemo-rpmax-12b -Quant Q5_K_M

.EXAMPLE
  .\Download-LlamaModel.ps1 -Url https://huggingface.co/bartowski/L3-8B-Stheno-v3.2-GGUF/resolve/main/L3-8B-Stheno-v3.2-Q5_K_M.gguf
#>
param(
    [switch]$List,
    [string]$Key,
    [string]$Quant = "Q4_K_M",
    [string]$Url,
    [string]$OutDir = $PSScriptRoot
)

# Verified against each repo's file listing - see recommended_models.html for how these were picked.
$Models = [ordered]@{
    "stheno-8b" = @{
        Name        = "L3-8B-Stheno-v3.2 (Sao10K) - fast 8B Llama-3 roleplay finetune"
        Repo        = "bartowski/L3-8B-Stheno-v3.2-GGUF"
        FilePattern = "L3-8B-Stheno-v3.2-{0}.gguf"
        Quants      = [ordered]@{ Q4_K_M = "4.92 GB"; Q5_K_M = "5.73 GB"; Q6_K = "6.59 GB"; Q8_0 = "8.54 GB" }
    }
    "nemo-rpmax-12b" = @{
        Name        = "Mistral-Nemo-12B-ArliAI-RPMax v1.2 - balanced 12B, purpose-built for roleplay, 128k context"
        Repo        = "QuantFactory/Mistral-Nemo-12B-ArliAI-RPMax-v1.2-GGUF"
        FilePattern = "Mistral-Nemo-12B-ArliAI-RPMax-v1.2.{0}.gguf"
        Quants      = [ordered]@{ Q4_K_M = "7.48 GB"; Q5_K_M = "8.73 GB"; Q6_K = "10.1 GB"; Q8_0 = "13 GB" }
    }
    "mythomax-13b" = @{
        Name        = "MythoMax-L2-13B (Gryphe) - old but battle-tested roleplay favourite"
        Repo        = "TheBloke/MythoMax-L2-13B-GGUF"
        FilePattern = "mythomax-l2-13b.{0}.gguf"
        Quants      = [ordered]@{ Q4_K_M = "7.87 GB"; Q5_K_M = "9.23 GB"; Q6_K = "10.68 GB"; Q8_0 = "13.83 GB" }
    }
    "cydonia-24b" = @{
        Name        = "Cydonia-24B v4.3 (TheDrummer) - highest quality, needs partial CPU offload on a 12GB GPU"
        Repo        = "bartowski/TheDrummer_Cydonia-24B-v4.3-GGUF"
        FilePattern = "TheDrummer_Cydonia-24B-v4.3-{0}.gguf"
        Quants      = [ordered]@{ IQ3_M = "10.7 GB"; Q3_K_M = "11.5 GB"; IQ4_XS = "12.8 GB"; Q4_K_S = "13.5 GB"; Q4_K_M = "14.3 GB" }
    }
}

function Show-ModelList {
    Write-Host "Curated roleplay models:" -ForegroundColor Cyan
    foreach ($key in $Models.Keys) {
        $model = $Models[$key]
        Write-Host ""
        Write-Host "  $key" -ForegroundColor Yellow -NoNewline
        Write-Host " - $($model.Name)"
        Write-Host "    https://huggingface.co/$($model.Repo)"
        Write-Host "    Quants: $($model.Quants.Keys -join ', ')"
    }
    Write-Host ""
    Write-Host "Usage:"
    Write-Host "  .\Download-LlamaModel.ps1 -Key <key> [-Quant Q4_K_M] [-OutDir <folder>]"
    Write-Host "  .\Download-LlamaModel.ps1 -Url <direct .gguf URL> [-OutDir <folder>]"
}

function Get-FileWithResume {
    param([string]$DownloadUrl, [string]$Destination)

    Write-Host "Downloading:"
    Write-Host "  $DownloadUrl"
    Write-Host "  -> $Destination"

    # -C - resumes a partial download; -L follows the redirect to the actual CDN file.
    & curl.exe -L --fail --retry 3 -C - -o $Destination $DownloadUrl
    if ($LASTEXITCODE -ne 0) {
        throw "Download failed (curl exit code $LASTEXITCODE). Re-run the same command to resume."
    }
}

if ($List -or (-not $Key -and -not $Url)) {
    Show-ModelList
    exit 0
}

if (-not (Test-Path $OutDir)) {
    New-Item -ItemType Directory -Path $OutDir -Force | Out-Null
}

if ($Url) {
    $fileName = Split-Path -Leaf ([Uri]$Url).LocalPath
    Get-FileWithResume -DownloadUrl $Url -Destination (Join-Path $OutDir $fileName)
    Write-Host "Done. Point the app's Llama.cpp tab 'Models folder' at:" -ForegroundColor Green
    Write-Host "  $OutDir"
    exit 0
}

if (-not $Models.Contains($Key)) {
    Write-Error "Unknown model key '$Key'. Run with -List to see available models."
    exit 1
}

$model = $Models[$Key]
if (-not $model.Quants.Contains($Quant)) {
    Write-Error "Quant '$Quant' isn't offered for '$Key'. Choices: $($model.Quants.Keys -join ', ')"
    exit 1
}

$fileName = ($model.FilePattern -f $Quant)
$downloadUrl = "https://huggingface.co/$($model.Repo)/resolve/main/$($fileName)?download=true"
$destination = Join-Path $OutDir $fileName

Write-Host $model.Name
Write-Host "Approx size: $($model.Quants[$Quant])"
Get-FileWithResume -DownloadUrl $downloadUrl -Destination $destination

Write-Host "Done. Point the app's Llama.cpp tab 'Models folder' at:" -ForegroundColor Green
Write-Host "  $OutDir"
