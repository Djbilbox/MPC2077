# MPC2077 — GitHub Release Setup
# Run this to initialize the GitHub repo and configure auto-builds

Write-Host "╔════════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║  MPC2077 — GitHub Actions Setup                              ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Step 1: Check if gh CLI is authenticated
Write-Host "🔑 Checking GitHub CLI authentication..." -ForegroundColor Yellow
gh auth status 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ GitHub CLI not authenticated!" -ForegroundColor Red
    Write-Host "Please run: gh auth login" -ForegroundColor Yellow
    Write-Host "Then re-run this script." -ForegroundColor Yellow
    exit 1
}
Write-Host "✓ GitHub CLI authenticated" -ForegroundColor Green
Write-Host ""

# Step 2: Create GitHub repo (if it doesn't exist)
Write-Host "📦 Creating GitHub repository..." -ForegroundColor Yellow
$repoUrl = gh repo create MPC2077 `
    --public `
    --source=. `
    --remote=origin `
    --push `
    2>&1

if ($LASTEXITCODE -eq 0) {
    Write-Host "✓ Repository created and pushed" -ForegroundColor Green
} else {
    Write-Host "⚠ Repository may already exist or error occurred" -ForegroundColor Yellow
    Write-Host "Checking existing remote..." -ForegroundColor Yellow

    # Try to add remote if it doesn't exist
    $remotes = git remote -v 2>&1
    if ($remotes -notlike "*origin*") {
        Write-Host "❌ No 'origin' remote configured" -ForegroundColor Red
        Write-Host "Please set up the remote manually:" -ForegroundColor Yellow
        Write-Host "  git remote add origin https://github.com/YOUR_USERNAME/MPC2077.git" -ForegroundColor Cyan
        exit 1
    }
}

Write-Host ""
Write-Host "📝 Committing MANUAL.md and workflows..." -ForegroundColor Yellow

# Step 3: Commit and push
cd $PSScriptRoot
git add MANUAL.md README.md .github/workflows/ SETUP_GITHUB.ps1
git commit -m "Add documentation, GitHub Actions workflows, and release setup" -q
git push origin main --quiet

Write-Host "✓ Documentation and workflows pushed" -ForegroundColor Green
Write-Host ""

# Step 4: Instructions for tagging a release
Write-Host "╔════════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║  🚀 To trigger a release build:                              ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""
Write-Host "1. Tag a new version:" -ForegroundColor White
Write-Host "   git tag v1.0.0" -ForegroundColor Cyan
Write-Host ""
Write-Host "2. Push the tag:" -ForegroundColor White
Write-Host "   git push origin v1.0.0" -ForegroundColor Cyan
Write-Host ""
Write-Host "3. GitHub Actions will automatically:" -ForegroundColor White
Write-Host "   • Build Windows VST3 + Standalone → ZIP" -ForegroundColor Cyan
Write-Host "   • Build macOS AU + Standalone → DMG" -ForegroundColor Cyan
Write-Host "   • Create Release with both artifacts" -ForegroundColor Cyan
Write-Host ""
Write-Host "4. Download from:" -ForegroundColor White
Write-Host "   https://github.com/YOUR_USERNAME/MPC2077/releases" -ForegroundColor Cyan
Write-Host ""

Write-Host "✅ Setup complete! You're ready to release." -ForegroundColor Green
