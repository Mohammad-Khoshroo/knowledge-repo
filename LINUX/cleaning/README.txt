# WSL Disk Space Cleanup Guide

## Method 1: Cleanup Inside WSL

```bash
# Clean apt cache
sudo apt clean
sudo apt autoclean
sudo apt autoremove

# Clean log files
sudo journalctl --vacuum-time=7d

# Clean temporary files
sudo rm -rf /tmp/*
rm -rf ~/.cache/*

# Remove old kernel files
sudo apt autoremove --purge
```

## Method 2: WSL Disk Compression (From Windows PowerShell)

```powershell
# Shutdown WSL
wsl --shutdown

# Compress virtual disk (replace Ubuntu with your distribution name)
wsl --manage Ubuntu --set-sparse true
```

## Method 3: Compact with diskpart

### Step 1: Find VHDX file location
```powershell
wsl --list -v
```
Then navigate to: `%USERPROFILE%\AppData\Local\Packages\CanonicalGroupLimited.Ubuntu*\LocalState\`

### Step 2: Use diskpart
```cmd
# Shutdown WSL first
wsl --shutdown

# Run diskpart
diskpart

# Inside diskpart:
select vdisk file="C:\path\to\your\ext4.vhdx"
attach vdisk readonly
compact vdisk
detach vdisk
```

## Additional Tips

- Always shutdown WSL before running diskpart operations
- Replace "Ubuntu" with your actual distribution name
- Update the VHDX file path to match your system
- The compact operation may take several minutes depending on disk size
