# Specify the single input file and output directory
$inputFile = ".\dist\index.html"
$outputDir = "..\ZaparooEsp32"

# Resolve paths to absolute paths
$inputFile = Resolve-Path -Path $inputFile
$outputDir = Resolve-Path -Path $outputDir

# Ensure output directory exists
if (!(Test-Path -Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir
}

# Check if 7-Zip is in the PATH
$sevenZipCommand = Get-Command "7z" -ErrorAction SilentlyContinue
if ($null -eq $sevenZipCommand) {
    Write-Error "7-Zip is not found in the PATH. Please install 7-Zip and add it to your PATH environment variable."
    exit 1
}
$sevenZipPath = $sevenZipCommand.Source

# Function to create a C-style array and define its length
function Convert-ToCArray {
    param (
        [string]$FilePath,
        [string]$ArrayName
    )
    $bytes = [System.IO.File]::ReadAllBytes($FilePath)
    $hexArray = $bytes | ForEach-Object { "0x{0:X2}" -f $_ }
    $cArray = $hexArray -join ", "
    $arrayLength = $bytes.Length
    $headerContent = @"
#define ${ArrayName}_len $arrayLength
const uint8_t $ArrayName[] PROGMEM = {
$cArray
};
"@
    return $headerContent
}

# Process the single file
$fileName = [System.IO.Path]::GetFileNameWithoutExtension($inputFile)
$fileExtension = [System.IO.Path]::GetExtension($inputFile).TrimStart('.')
$variableName = ($fileName + "_" + $fileExtension).Replace(".", "_") -replace "[^a-zA-Z0-9_]", "_"

$gzFile = Join-Path -Path $outputDir -ChildPath "$fileName.gz"
$hFile = Join-Path -Path $outputDir -ChildPath "$fileName.h"

# Compress the file using 7-Zip with highest compression (-mx=9)
Write-Host "Compressing: $inputFile to $gzFile using 7-Zip with highest compression"
$process = Start-Process -FilePath "7z" -ArgumentList "a -tgzip -mx=9 -y `"$gzFile`" `"$inputFile`"" -NoNewWindow -Wait -PassThru
if ($process.ExitCode -ne 0) {
    Write-Error "7-Zip compression failed. Exit code: $($process.ExitCode)"
    exit 1
}

# Generate the .h file with array and length definition
Write-Host "Generating .h file for: $gzFile"
$cArray = Convert-ToCArray -FilePath $gzFile -ArrayName $variableName
Set-Content -Path $hFile -Value $cArray

# Delete the .gz file after the .h file is generated
if (Test-Path -Path $gzFile) {
    Remove-Item -Path $gzFile -Force
    Write-Host "Deleted intermediate .gz file: $gzFile"
} else {
    Write-Host "The .gz file does not exist or was already deleted: $gzFile"
}

Write-Host "Processing complete. Output saved to $outputDir"
