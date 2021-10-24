<#
.SYNOPSIS
  Dev env loader
.DESCRIPTION
  Usage: uenv <command> [args]...
  Commands:
    init [-a|--activate] <env-file> <toolset> [toolset-args]...
    load <env-file>
    activate <toolset> [toolset-args]...
    revert
    help [toolset]

  env-file will be named *.env, backups are named *.env.old.
#>
param()

function script:LoadEnv {
  $lines = $args[0]
  $old = $args[1]
  if ($old) {
    Set-Content -Path $old -Value "" -Encoding utf8 -NoNewline
    Write-Verbose "Current environment stored in $old"
  }
  foreach ($line in $lines) {
    $eq = $line.IndexOf('=')
    if ($eq -eq -1) {
      continue
    }
    $k = $line.Substring(0, $eq)
    $v = $line.Substring($eq + 1)
    if ($old) {
      $ov = Get-Content -Path "Env:\$k"
      if ($ov -ne $v) {
        "$k=$ov" | Out-File -FilePath $old -Append -Encoding utf8
      }
    }
    Set-Content "Env:\$k" $v
  }

  if ($old) {
    # LOL powershell
    [System.Environment]::CurrentDirectory = $PWD
    $env:UENV_OLD_ENV = [System.IO.Path]::GetFullPath($old)
  }
}

if ($args[0] -eq "init") {
  $fname = $args[1]
  $rest = $args[2..9999]
  $activate = $false
  if ($fname -eq "-a" -or $fname -eq "--activate") {
    $fname = $args[2]
    $rest = $args[3..9999]
    $activate = $true
  }
  if (!$fname -or $rest.Count -eq 0) {
    Write-Error "Missing arguments for init <file> <toolset> [args]..."
    exit 1
  }
  if (!$fname.EndsWith(".env") -and !$fname.EndsWith(".env.old")) {
    $fname += ".env"
  }
  Write-Verbose "Writing environment to $fname"
  node "$PSScriptRoot\mkenv.js" @rest | Out-File -FilePath $fname -Encoding utf8
  if ($activate) {
    $old = $null
    if (!$fname.EndsWith(".old")) {
      $old = "$fname.old"
    }
    Write-Verbose "Loading environment"
    LoadEnv $e $old
  }
} elseif ($args[0] -eq "activate") {
  $rest = $args[1..9999]
  if ($rest.Count -eq 0) {
    Write-Error "Missing arguments for activate <toolset> [args]..."
    exit 1
  }
  Write-Verbose "Calling $PSScriptRoot\mkenv.js"
  $e = node "$PSScriptRoot\mkenv.js" @rest
  Write-Verbose "Loading environment"
  LoadEnv $e $null
} elseif ($args[0] -eq "load") {
  if ($args.Count -ne 2) {
    Write-Error "Wrong number of arguments. Usage: uenv load <file>"
    exit 1
  }
  $fname = $args[1]
  if (!$fname.EndsWith(".env") -and !$fname.EndsWith(".env.old")) {
    $fname += ".env"
  }
  if ($fname.EndsWith(".old")) {
    $old = $null
  } else {
    $old = $fname + ".old"
  }
  Write-Verbose ("Loading environment from " + $fname)
  $lines = Get-Content -Path $fname -Encoding utf8
  LoadEnv $lines $old
} elseif ($args[0] -eq "revert") {
  if ($args.Count -gt 1) {
    Write-Error "Too many arguments"
    exit 1
  }
  $old = $env:UENV_OLD_ENV
  if (!$old) {
    Write-Error "UENV_OLD_ENV not set"
    exit 1
  }
  Write-Verbose "Loading environment from $old"
  $lines = Get-Content -Path $old -Encoding utf8
  LoadEnv $lines $null
  Remove-Item $old
  Set-Content "Env:\UENV_OLD_ENV" ""
} elseif ($args[0] -eq "help") {
  if ($args[1]) {
    node "$PSScriptRoot\mkenv.js" help $args[1]
  } else {
    Write-Output (Get-Help $PSCommandPath).description[0].Text
  }
} else {
  Write-Output (Get-Help $PSCommandPath).description[0].Text
  exit 1
}