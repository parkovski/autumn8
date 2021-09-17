[CmdletBinding(SupportsShouldProcess)]
param(
  [Parameter(Position = 0, Mandatory = $true)]
  [string]
  $Name,

  [Parameter(Position = 1, Mandatory = $false)]
  [Alias('f')]
  [string[]]
  $Files,

  [Parameter(Mandatory = $true)]
  [ValidateSet('G', 'Gen', 'Generator', 'E', 'Eff', 'Effect')]
  [Alias('k')]
  [string]
  $Kind,

  [Alias('a')]
  [string]
  $Author = $null
)

# TODO: FL Versions

if ($null -eq $Files -or $Files.Count -eq 0) {
  $ff = [System.Collections.Generic.List[string]]::new()

  if (Test-Path ".\$Name.dll" -PathType Leaf) {
    $ff.Add(".\$Name.dll")
  } elseif (Test-Path ".\bin\$Name.dll" -PathType Leaf) {
    $ff.Add(".\bin\$Name.dll")
  }

  if (Test-Path ".\${Name}_x64.dll" -PathType Leaf) {
    $ff.Add(".\${Name}_x64.dll")
  } elseif (Test-Path ".\bin\${Name}_x64.dll" -PathType Leaf) {
    $ff.Add(".\bin\${Name}_x64.dll")
  }

  $Files = $ff.ToArray()
}

$fl_path = Get-ItemPropertyValue HKLM:\SOFTWARE\Image-Line\Shared\Paths 'FL Studio'
$fl_path = Split-Path -Parent $fl_path

$plugins = "$fl_path\Plugins\Fruity\"

if ($Kind[0] -eq 'G') {
  $plugins += "Generators"
} else {
  $plugins += "Effects"
}

$dir = "$plugins\$Name\"
if (-not ($PSCmdlet.ShouldProcess($Name))) {
  if (-not (Test-Path $dir)) {
    Write-Output "Will create and install plugin in '$dir'."
  } else {
    Write-Output "Will install plugin in '$dir'."
  }
  Write-Output "Will copy $Files."
  if (-not ([string]::IsNullOrEmpty($Author))) {
    Write-Output "Will create '${dir}Plugin.nfo'."
  }
  exit
}

if (-not (Test-Path $dir)) {
  mkdir $dir
}

Copy-Item $Files $dir

if (-not ([string]::IsNullOrEmpty($Author))) {
  Write-Output -NoNewLine "ps_vendorname=$Author" > ($dir + "Plugin.nfo")
  Write-Output "Created Plugin.nfo for $Author"
}

Write-Output "Installed in '$dir'."