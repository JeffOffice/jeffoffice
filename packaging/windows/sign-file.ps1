# Signs one file with Azure Artifact Signing, the way the azure/artifact-signing-action steps in build.yml do (same module, same settings).
# Inno Setup runs it (ISCC's /S option, see build-installer.ps1 -SignCommand) for the installer and for the uninstaller it builds in.
# The signed file is also copied to $env:JO_SIGNED_COPIES (when set), so that the workflow can check what Inno signed.
param([Parameter(Mandatory)][string]$File)
$ErrorActionPreference = 'Stop'
Invoke-ArtifactSigning -Endpoint https://eus.codesigning.azure.net/ -CodeSigningAccountName jeffofficesigning `
  -CertificateProfileName jeffoffice-public -Files $File -FileDigest SHA256 -TimestampRfc3161 http://timestamp.acs.microsoft.com `
  -TimestampDigest SHA256 -Description JeffOffice -DescriptionUrl https://www.jeffoffice.com
if ($env:JO_SIGNED_COPIES) {
  New-Item -ItemType Directory -Force $env:JO_SIGNED_COPIES | Out-Null
  $n = @(Get-ChildItem $env:JO_SIGNED_COPIES -File).Count
  Copy-Item $File (Join-Path $env:JO_SIGNED_COPIES ("{0}-{1}" -f $n, (Split-Path $File -Leaf)))
}
