using System;
using System.IO;
using System.Net.Http;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;

namespace DWMBlurGlassGUI.Services
{
    public class SymbolDownloader
    {
        private static readonly string[] TargetDlls = { "dwmcore.dll", "uDwm.dll" };
        private static readonly string SymbolServerUrl = "https://msdl.microsoft.com/download/symbols";
        
        private readonly string _symbolsPath;
        private readonly HttpClient _httpClient;

        public event EventHandler<DownloadProgressEventArgs>? ProgressChanged;

        public SymbolDownloader()
        {
            _symbolsPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "data", "symbols");
            _httpClient = new HttpClient();
            _httpClient.Timeout = TimeSpan.FromMinutes(10);
        }

        /// <summary>
        /// Check if symbols are valid for both dwmcore.dll and uDwm.dll
        /// </summary>
        public bool CheckSymbolsValid()
        {
            try
            {
                foreach (var dllName in TargetDlls)
                {
                    if (!CheckSingleSymbolValid(dllName))
                        return false;
                }
                return true;
            }
            catch
            {
                return false;
            }
        }

        private bool CheckSingleSymbolValid(string dllName)
        {
            try
            {
                string systemPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System), dllName);
                if (!File.Exists(systemPath))
                    return false;

                var peInfo = GetPEInfo(systemPath);
                if (peInfo == null)
                    return false;

                string pdbName = Path.GetFileNameWithoutExtension(dllName) + ".pdb";
                string pdbPath = Path.Combine(_symbolsPath, pdbName, peInfo.Value.Guid.ToString("N").ToUpperInvariant() + peInfo.Value.Age.ToString(), pdbName);

                return File.Exists(pdbPath);
            }
            catch
            {
                return false;
            }
        }

        /// <summary>
        /// Download symbols for both dwmcore.dll and uDwm.dll
        /// </summary>
        public async Task<bool> DownloadSymbolsAsync(IProgress<DownloadProgressEventArgs>? progress = null, CancellationToken cancellationToken = default)
        {
            try
            {
                Directory.CreateDirectory(_symbolsPath);

                int totalFiles = TargetDlls.Length;
                int currentFile = 0;

                foreach (var dllName in TargetDlls)
                {
                    currentFile++;
                    var progressArgs = new DownloadProgressEventArgs
                    {
                        FileName = dllName,
                        CurrentFile = currentFile,
                        TotalFiles = totalFiles,
                        BytesReceived = 0,
                        TotalBytes = 0,
                        ProgressPercentage = 0
                    };
                    progress?.Report(progressArgs);
                    ProgressChanged?.Invoke(this, progressArgs);

                    bool success = await DownloadSingleSymbolAsync(dllName, progress, cancellationToken);
                    if (!success)
                        return false;
                }

                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }

        private async Task<bool> DownloadSingleSymbolAsync(string dllName, IProgress<DownloadProgressEventArgs>? progress, CancellationToken cancellationToken)
        {
            try
            {
                string systemPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System), dllName);
                if (!File.Exists(systemPath))
                    return false;

                var peInfo = GetPEInfo(systemPath);
                if (peInfo == null)
                    return false;

                string pdbName = Path.GetFileNameWithoutExtension(dllName) + ".pdb";
                string guidAge = peInfo.Value.Guid.ToString("N").ToUpperInvariant() + peInfo.Value.Age.ToString();
                string downloadUrl = $"{SymbolServerUrl}/{pdbName}/{guidAge}/{pdbName}";
                string localDir = Path.Combine(_symbolsPath, pdbName, guidAge);
                string localPath = Path.Combine(localDir, pdbName);

                // Check if already exists
                if (File.Exists(localPath))
                    return true;

                Directory.CreateDirectory(localDir);

                using var response = await _httpClient.GetAsync(downloadUrl, HttpCompletionOption.ResponseHeadersRead, cancellationToken);
                response.EnsureSuccessStatusCode();

                var totalBytes = response.Content.Headers.ContentLength ?? -1;
                long bytesReceived = 0;

                using var contentStream = await response.Content.ReadAsStreamAsync(cancellationToken);
                using var fileStream = new FileStream(localPath, FileMode.Create, FileAccess.Write, FileShare.None, 8192, true);

                var buffer = new byte[8192];
                int bytesRead;

                while ((bytesRead = await contentStream.ReadAsync(buffer, cancellationToken)) > 0)
                {
                    await fileStream.WriteAsync(buffer.AsMemory(0, bytesRead), cancellationToken);
                    bytesReceived += bytesRead;

                    var progressArgs = new DownloadProgressEventArgs
                    {
                        FileName = dllName,
                        CurrentFile = Array.IndexOf(TargetDlls, dllName) + 1,
                        TotalFiles = TargetDlls.Length,
                        BytesReceived = bytesReceived,
                        TotalBytes = totalBytes,
                        ProgressPercentage = totalBytes > 0 ? (int)(bytesReceived * 100 / totalBytes) : 0
                    };
                    progress?.Report(progressArgs);
                    ProgressChanged?.Invoke(this, progressArgs);
                }

                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }

        private PEDebugInfo? GetPEInfo(string filePath)
        {
            try
            {
                using var fs = new FileStream(filePath, FileMode.Open, FileAccess.Read, FileShare.Read);
                using var reader = new BinaryReader(fs);

                // Read DOS header
                if (reader.ReadUInt16() != 0x5A4D) // MZ
                    return null;

                fs.Position = 0x3C; // e_lfanew offset
                int peOffset = reader.ReadInt32();

                fs.Position = peOffset;
                if (reader.ReadUInt32() != 0x00004550) // PE\0\0
                    return null;

                // Read COFF header
                ushort machine = reader.ReadUInt16();
                ushort numberOfSections = reader.ReadUInt16();
                reader.ReadUInt32(); // TimeDateStamp
                reader.ReadUInt32(); // PointerToSymbolTable
                reader.ReadUInt32(); // NumberOfSymbols
                ushort sizeOfOptionalHeader = reader.ReadUInt16();
                reader.ReadUInt16(); // Characteristics

                // Read Optional Header
                long optionalHeaderStart = fs.Position;
                ushort magic = reader.ReadUInt16();
                bool is64Bit = magic == 0x20b;

                // Skip to data directories
                int dataDirectoryOffset = is64Bit ? 112 : 96;
                fs.Position = optionalHeaderStart + dataDirectoryOffset;

                // Skip first 6 data directories to get to Debug directory (index 6)
                for (int i = 0; i < 6; i++)
                {
                    reader.ReadUInt32(); // VirtualAddress
                    reader.ReadUInt32(); // Size
                }

                uint debugDirVA = reader.ReadUInt32();
                uint debugDirSize = reader.ReadUInt32();

                if (debugDirVA == 0 || debugDirSize == 0)
                    return null;

                // Read section headers to find Debug directory file offset
                fs.Position = optionalHeaderStart + sizeOfOptionalHeader;

                uint debugDirFileOffset = 0;
                for (int i = 0; i < numberOfSections; i++)
                {
                    byte[] nameBytes = reader.ReadBytes(8);
                    reader.ReadUInt32(); // VirtualSize
                    uint virtualAddress = reader.ReadUInt32();
                    uint sizeOfRawData = reader.ReadUInt32();
                    uint pointerToRawData = reader.ReadUInt32();
                    reader.ReadBytes(16); // Rest of section header

                    if (debugDirVA >= virtualAddress && debugDirVA < virtualAddress + sizeOfRawData)
                    {
                        debugDirFileOffset = pointerToRawData + (debugDirVA - virtualAddress);
                        break;
                    }
                }

                if (debugDirFileOffset == 0)
                    return null;

                // Read debug directory entries
                fs.Position = debugDirFileOffset;
                int numEntries = (int)(debugDirSize / 28); // sizeof(IMAGE_DEBUG_DIRECTORY) = 28

                for (int i = 0; i < numEntries; i++)
                {
                    reader.ReadUInt32(); // Characteristics
                    reader.ReadUInt32(); // TimeDateStamp
                    reader.ReadUInt16(); // MajorVersion
                    reader.ReadUInt16(); // MinorVersion
                    uint type = reader.ReadUInt32();
                    reader.ReadUInt32(); // SizeOfData
                    reader.ReadUInt32(); // AddressOfRawData
                    uint pointerToRawData = reader.ReadUInt32();

                    if (type == 2) // IMAGE_DEBUG_TYPE_CODEVIEW
                    {
                        long currentPos = fs.Position;
                        fs.Position = pointerToRawData;

                        uint signature = reader.ReadUInt32();
                        if (signature == 0x53445352) // RSDS
                        {
                            byte[] guidBytes = reader.ReadBytes(16);
                            uint age = reader.ReadUInt32();
                            return new PEDebugInfo
                            {
                                Guid = new Guid(guidBytes),
                                Age = age
                            };
                        }

                        fs.Position = currentPos;
                    }
                }

                return null;
            }
            catch
            {
                return null;
            }
        }

        private struct PEDebugInfo
        {
            public Guid Guid;
            public uint Age;
        }
    }

    public class DownloadProgressEventArgs : EventArgs
    {
        public string FileName { get; set; } = string.Empty;
        public int CurrentFile { get; set; }
        public int TotalFiles { get; set; }
        public long BytesReceived { get; set; }
        public long TotalBytes { get; set; }
        public int ProgressPercentage { get; set; }
    }
}
