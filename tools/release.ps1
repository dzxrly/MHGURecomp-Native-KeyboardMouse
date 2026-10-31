param([string]$Archive = '', [string]$NotesFile = '', [switch]$Publish)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$version = (Get-Content -LiteralPath (Join-Path $repoRoot 'VERSION') -Raw).Trim()
$upstream = (Get-Content -LiteralPath (Join-Path $repoRoot 'UPSTREAM_VERSION') -Raw).Trim()
if ($version -notmatch '^\d+\.\d+\.\d+$' -or $upstream -notmatch '^\d+(\.\d+){1,3}$') { throw 'Invalid version metadata.' }
$title = "$version-$upstream"
if (!$Archive) { $Archive = Join-Path $repoRoot "dist/Windows-MHGURecomp.$version.zip" }
$Archive = [IO.Path]::GetFullPath($Archive)
if (!$Publish) {
    [pscustomobject]@{ Version = $version; UpstreamVersion = $upstream; Title = $title; Archive = $Archive }
    return
}
if (!(Test-Path -LiteralPath $Archive) -or !(Test-Path -LiteralPath "$Archive.sha256")) { throw 'Build the archive and its checksum first.' }
if (!$NotesFile -or !(Test-Path -LiteralPath $NotesFile)) { throw 'Pass -NotesFile with the release notes.' }
$NotesFile = [IO.Path]::GetFullPath($NotesFile)
Push-Location $repoRoot
try {
    if (git status --porcelain) { throw 'Commit the prepared release before publishing.' }
    $head = (git rev-parse HEAD).Trim()
    $tag = git rev-parse "${version}^{commit}"
    if ($LASTEXITCODE -ne 0 -or $tag.Trim() -ne $head) { throw 'The version tag must point to HEAD; push it before publishing.' }
    $origin = (git remote get-url origin).Trim()
    if ($origin -notmatch 'github\.com[:/](?<repo>[^/]+/[^/]+?)(\.git)?$') { throw 'The origin is not a GitHub repository.' }
    $repository = $Matches.repo
    $expectedHash = (Get-Content -LiteralPath "$Archive.sha256" -Raw).Split(' ')[0].ToLowerInvariant()
    if ($expectedHash -notmatch '^[0-9a-f]{64}$') { throw 'Invalid archive checksum.' }
    & gh release create $version $Archive "$Archive.sha256" --repo $repository --verify-tag --title $title --notes-file $NotesFile --draft
    if ($LASTEXITCODE -ne 0) { throw 'Draft release/upload failed.' }
    $release = & gh api "repos/$repository/releases/tags/$version" | ConvertFrom-Json
    if ($LASTEXITCODE -ne 0) { throw 'Cannot verify the uploaded draft.' }
    $asset = @($release.assets | Where-Object { $_.name -eq [IO.Path]::GetFileName($Archive) })
    if ($asset.Count -ne 1 -or $asset[0].size -ne (Get-Item -LiteralPath $Archive).Length -or $asset[0].digest -ne "sha256:$expectedHash") {
        throw 'Uploaded archive does not match the prepared payload; release remains a draft.'
    }
    & gh release edit $version --repo $repository --draft=false --latest --title $title
    if ($LASTEXITCODE -ne 0) { throw 'Publishing failed; inspect the draft.' }
    & gh release view $version --repo $repository --json url,name,tagName,isDraft,isPrerelease
} finally { Pop-Location }
