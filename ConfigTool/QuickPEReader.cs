using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using static DLSSTweaks.ConfigTool.Utility;

namespace DLSSTweaks.ConfigTool
{
    public class QuickPEReader
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct IMAGE_DOS_HEADER
        {
            public ushort e_magic;       // Magic number
            public ushort e_cblp;    // Bytes on last page of file
            public ushort e_cp;      // Pages in file
            public ushort e_crlc;    // Relocations
            public ushort e_cparhdr;     // Size of header in paragraphs
            public ushort e_minalloc;    // Minimum extra paragraphs needed
            public ushort e_maxalloc;    // Maximum extra paragraphs needed
            public ushort e_ss;      // Initial (relative) SS value
            public ushort e_sp;      // Initial SP value
            public ushort e_csum;    // Checksum
            public ushort e_ip;      // Initial IP value
            public ushort e_cs;      // Initial (relative) CS value
            public ushort e_lfarlc;      // File address of relocation table
            public ushort e_ovno;    // Overlay number
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
            public ushort[] e_res1;    // Reserved words
            public ushort e_oemid;       // OEM identifier (for e_oeminfo)
            public ushort e_oeminfo;     // OEM information; e_oemid specific
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 10)]
            public ushort[] e_res2;    // Reserved words
            public int e_lfanew;      // File address of new exe header

            public bool IsValid => e_magic == 0x5A4D || e_magic == 0x4D5A;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct IMAGE_FILE_HEADER
        {
            public ushort Machine;
            public ushort NumberOfSections;
            public uint TimeDateStamp;
            public uint PointerToSymbolTable;
            public uint NumberOfSymbols;
            public ushort SizeOfOptionalHeader;
            public ushort Characteristics;

            public bool IsValidWin3264 => Machine == 0x8664 || Machine == 0x6486 || Machine == 0x014c || Machine == 0x4c01;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct IMAGE_DATA_DIRECTORY
        {
            public uint VirtualAddress;
            public uint Size;
        }

        [StructLayout(LayoutKind.Explicit)]
        public struct IMAGE_OPTIONAL_HEADER32
        {
            [FieldOffset(0)]
            public ushort Magic;

            [FieldOffset(2)]
            public byte MajorLinkerVersion;

            [FieldOffset(3)]
            public byte MinorLinkerVersion;

            [FieldOffset(4)]
            public uint SizeOfCode;

            [FieldOffset(8)]
            public uint SizeOfInitializedData;

            [FieldOffset(12)]
            public uint SizeOfUninitializedData;

            [FieldOffset(16)]
            public uint AddressOfEntryPoint;

            [FieldOffset(20)]
            public uint BaseOfCode;

            // PE32 contains this additional field
            [FieldOffset(24)]
            public uint BaseOfData;

            [FieldOffset(28)]
            public uint ImageBase;

            [FieldOffset(32)]
            public uint SectionAlignment;

            [FieldOffset(36)]
            public uint FileAlignment;

            [FieldOffset(40)]
            public ushort MajorOperatingSystemVersion;

            [FieldOffset(42)]
            public ushort MinorOperatingSystemVersion;

            [FieldOffset(44)]
            public ushort MajorImageVersion;

            [FieldOffset(46)]
            public ushort MinorImageVersion;

            [FieldOffset(48)]
            public ushort MajorSubsystemVersion;

            [FieldOffset(50)]
            public ushort MinorSubsystemVersion;

            [FieldOffset(52)]
            public uint Win32VersionValue;

            [FieldOffset(56)]
            public uint SizeOfImage;

            [FieldOffset(60)]
            public uint SizeOfHeaders;

            [FieldOffset(64)]
            public uint CheckSum;

            [FieldOffset(68)]
            public ushort Subsystem;

            [FieldOffset(70)]
            public ushort DllCharacteristics;

            [FieldOffset(72)]
            public uint SizeOfStackReserve;

            [FieldOffset(76)]
            public uint SizeOfStackCommit;

            [FieldOffset(80)]
            public uint SizeOfHeapReserve;

            [FieldOffset(84)]
            public uint SizeOfHeapCommit;

            [FieldOffset(88)]
            public uint LoaderFlags;

            [FieldOffset(92)]
            public uint NumberOfRvaAndSizes;

            public bool IsValid => Magic == 0x10b || Magic == 0x0b01;
        }
        [StructLayout(LayoutKind.Explicit)]
        public struct IMAGE_OPTIONAL_HEADER64
        {
            [FieldOffset(0)]
            public ushort Magic;

            [FieldOffset(2)]
            public byte MajorLinkerVersion;

            [FieldOffset(3)]
            public byte MinorLinkerVersion;

            [FieldOffset(4)]
            public uint SizeOfCode;

            [FieldOffset(8)]
            public uint SizeOfInitializedData;

            [FieldOffset(12)]
            public uint SizeOfUninitializedData;

            [FieldOffset(16)]
            public uint AddressOfEntryPoint;

            [FieldOffset(20)]
            public uint BaseOfCode;

            [FieldOffset(24)]
            public ulong ImageBase;

            [FieldOffset(32)]
            public uint SectionAlignment;

            [FieldOffset(36)]
            public uint FileAlignment;

            [FieldOffset(40)]
            public ushort MajorOperatingSystemVersion;

            [FieldOffset(42)]
            public ushort MinorOperatingSystemVersion;

            [FieldOffset(44)]
            public ushort MajorImageVersion;

            [FieldOffset(46)]
            public ushort MinorImageVersion;

            [FieldOffset(48)]
            public ushort MajorSubsystemVersion;

            [FieldOffset(50)]
            public ushort MinorSubsystemVersion;

            [FieldOffset(52)]
            public uint Win32VersionValue;

            [FieldOffset(56)]
            public uint SizeOfImage;

            [FieldOffset(60)]
            public uint SizeOfHeaders;

            [FieldOffset(64)]
            public uint CheckSum;

            [FieldOffset(68)]
            public ushort Subsystem;

            [FieldOffset(70)]
            public ushort DllCharacteristics;

            [FieldOffset(72)]
            public ulong SizeOfStackReserve;

            [FieldOffset(80)]
            public ulong SizeOfStackCommit;

            [FieldOffset(88)]
            public ulong SizeOfHeapReserve;

            [FieldOffset(96)]
            public ulong SizeOfHeapCommit;

            [FieldOffset(104)]
            public uint LoaderFlags;

            [FieldOffset(108)]
            public uint NumberOfRvaAndSizes;

            public bool IsValid => Magic == 0x20b || Magic == 0x0b02;
        }

        [StructLayout(LayoutKind.Explicit)]
        public struct IMAGE_DATA_DIRECTORIES
        {
            [FieldOffset(112 - 112)]
            public IMAGE_DATA_DIRECTORY ExportTable;

            [FieldOffset(120 - 112)]
            public IMAGE_DATA_DIRECTORY ImportTable;

            [FieldOffset(128 - 112)]
            public IMAGE_DATA_DIRECTORY ResourceTable;

            [FieldOffset(136 - 112)]
            public IMAGE_DATA_DIRECTORY ExceptionTable;

            [FieldOffset(144 - 112)]
            public IMAGE_DATA_DIRECTORY CertificateTable;

            [FieldOffset(152 - 112)]
            public IMAGE_DATA_DIRECTORY BaseRelocationTable;

            [FieldOffset(160 - 112)]
            public IMAGE_DATA_DIRECTORY Debug;

            [FieldOffset(168 - 112)]
            public IMAGE_DATA_DIRECTORY Architecture;

            [FieldOffset(176 - 112)]
            public IMAGE_DATA_DIRECTORY GlobalPtr;

            [FieldOffset(184 - 112)]
            public IMAGE_DATA_DIRECTORY TLSTable;

            [FieldOffset(192 - 112)]
            public IMAGE_DATA_DIRECTORY LoadConfigTable;

            [FieldOffset(200 - 112)]
            public IMAGE_DATA_DIRECTORY BoundImport;

            [FieldOffset(208 - 112)]
            public IMAGE_DATA_DIRECTORY IAT;

            [FieldOffset(216 - 112)]
            public IMAGE_DATA_DIRECTORY DelayImportDescriptor;

            [FieldOffset(224 - 112)]
            public IMAGE_DATA_DIRECTORY CLRRuntimeHeader;

            [FieldOffset(232 - 112)]
            public IMAGE_DATA_DIRECTORY Reserved;
        }

        [StructLayout(LayoutKind.Explicit)]
        public struct IMAGE_NT_HEADERS
        {
            [FieldOffset(0)]
            public uint Signature;

            [FieldOffset(4)]
            public IMAGE_FILE_HEADER FileHeader;

            public bool IsValid => Signature == 0x4550 || Signature == 0x5045;
        }

        [StructLayout(LayoutKind.Explicit)]
        public struct IMAGE_SECTION_HEADER
        {
            [FieldOffset(0)]
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
            public char[] Name;
            [FieldOffset(8)]
            public uint VirtualSize;
            [FieldOffset(12)]
            public uint VirtualAddress;
            [FieldOffset(16)]
            public uint SizeOfRawData;
            [FieldOffset(20)]
            public uint PointerToRawData;
            [FieldOffset(24)]
            public uint PointerToRelocations;
            [FieldOffset(28)]
            public uint PointerToLinenumbers;
            [FieldOffset(32)]
            public ushort NumberOfRelocations;
            [FieldOffset(34)]
            public ushort NumberOfLinenumbers;
            [FieldOffset(36)]
            public uint Characteristics;

            public string Section
            {
                get { return new string(Name).TrimEnd(new char[]{ '\0' }); }
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct IMAGE_IMPORT_DESCRIPTOR
        {
            public uint OriginalFirstThunk;
            public uint TimeDateStamp;
            public uint ForwarderChain;
            public uint Name;
            public uint FirstThunk;
        }

        long BasePosition = 0;
        public IMAGE_DOS_HEADER DosHeader;
        public IMAGE_NT_HEADERS NtHeaders;
        public IMAGE_OPTIONAL_HEADER32 OptionalHeader32;
        public IMAGE_OPTIONAL_HEADER64 OptionalHeader64;
        public IMAGE_DATA_DIRECTORIES DataDirectories;
        public bool IsHeaders64 = false;

        public List<IMAGE_SECTION_HEADER> Sections = new List<IMAGE_SECTION_HEADER>();

        public long RVA2Offset(long rva)
        {
            foreach (var section in Sections)
            {
                var sectionStart = section.VirtualAddress;
                var sectionEnd = section.VirtualAddress + section.VirtualSize;
                if (rva >= sectionStart && rva < sectionEnd)
                {
                    return BasePosition + (rva - sectionStart) + section.PointerToRawData;
                }
            }
            return 0;
        }

        public bool Read(BinaryReader reader)
        {
            BasePosition = reader.BaseStream.Position;

            DosHeader = reader.ReadStruct<IMAGE_DOS_HEADER>();
            if (!DosHeader.IsValid)
                return false;

            reader.BaseStream.Position = BasePosition + DosHeader.e_lfanew;

            NtHeaders = reader.ReadStruct<IMAGE_NT_HEADERS>();
            if (!NtHeaders.IsValid)
                return false;

            var OptionalPosition = reader.BaseStream.Position;
            OptionalHeader32 = reader.ReadStruct<IMAGE_OPTIONAL_HEADER32>();
            if (!OptionalHeader32.IsValid)
            {
                reader.BaseStream.Position = OptionalPosition;
                OptionalHeader64 = reader.ReadStruct<IMAGE_OPTIONAL_HEADER64>();
                if (OptionalHeader64.IsValid)
                    IsHeaders64 = true;
                else
                {
                    IsHeaders64 = false;
                    return false;
                }
            }
            DataDirectories = reader.ReadStruct<IMAGE_DATA_DIRECTORIES>();

            Sections = new List<IMAGE_SECTION_HEADER>();
            for (int i = 0; i < NtHeaders.FileHeader.NumberOfSections; i++)
            {
                var section = reader.ReadStruct<IMAGE_SECTION_HEADER>();
                Sections.Add(section);
            }

            return true;
        }

        public string[] ReadImportedModules(BinaryReader reader)
        {
            var directory = DataDirectories.ImportTable;
            if (directory.Size == 0)
                return null;

            var offset = RVA2Offset(directory.VirtualAddress);
            reader.BaseStream.Position = offset;

            var modules = new List<string>();
            var modulesLower = new List<string>();

            var curOffset = reader.BaseStream.Position;
            while (reader.BaseStream.Length > reader.BaseStream.Position)
            {
                reader.BaseStream.Position = curOffset;
                var descriptor = reader.ReadStruct<IMAGE_IMPORT_DESCRIPTOR>();
                curOffset = reader.BaseStream.Position;

                // Check if we've reached the end of the import directory
                if (descriptor.Name == 0)
                    break;

                // Read the module name
                long descriptorNameOffset = RVA2Offset(descriptor.Name);
                reader.BaseStream.Position = descriptorNameOffset;
                var moduleName = reader.ReadNullTerminatedString();
                var moduleNameLower = moduleName.ToLower();

                if (!modulesLower.Contains(moduleNameLower))
                {
                    modulesLower.Add(moduleNameLower);
                    modules.Add(moduleName);
                }
            }

            return modules.ToArray();
        }
    }
}
