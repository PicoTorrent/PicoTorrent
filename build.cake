#r "./tools/Cake.CMake/lib/net45/Cake.CMake.dll"

//////////////////////////////////////////////////////////////////////
// ARGUMENTS
//////////////////////////////////////////////////////////////////////

var target        = Argument("target", "Default");
var configuration = Argument("configuration", "Release");
var platform      = Argument("platform", "x64");

// Parameters
var OutputDirectory    = Directory("./build-" + platform);
var BuildDirectory     = OutputDirectory + Directory(configuration);
var PublishDirectory   = BuildDirectory + Directory("publish");
var PackagesDirectory  = BuildDirectory + Directory("packages");
var ResourceDirectory  = Directory("./res");

var LibraryDirectory   = Directory("./tools")
                       + Directory("PicoTorrent.Libs")
                       + Directory("bin")
                       + Directory(platform)
                       + Directory(configuration);

var SigningCertificate = EnvironmentVariable("PICO_SIGNING_CERTIFICATE");
var SigningPassword    = EnvironmentVariable("PICO_SIGNING_PASSWORD");

var Version            = System.IO.File.ReadAllText("VERSION").Trim();
var Installer          = string.Format("PicoTorrent-{0}-{1}.msi", Version, platform);
var InstallerBundle    = string.Format("PicoTorrent-{0}-{1}.exe", Version, platform);
var PortablePackage    = string.Format("PicoTorrent-{0}-{1}.zip", Version, platform);
var SymbolsPackage     = string.Format("PicoTorrent-{0}-{1}.symbols.zip", Version, platform);
var AppXPackage        = string.Format("PicoTorrent-{0}-{1}.appx", Version, platform);

bool IsDebug() { return configuration.Equals("Debug"); }

// Boost naming ickiness
var BoostSystem = IsDebug() ? "boost_system-vc140-mt-gd-1_61.dll" : "boost_system-vc140-mt-1_61.dll";

void SignFile(FilePath file, string description = "")
{
    Sign(file,
        new SignToolSignSettings
        {
            Description = description,
            CertPath = SigningCertificate,
            Password = SigningPassword,
            TimeStampUri = new Uri("http://timestamp.digicert.com")
        });
}

//////////////////////////////////////////////////////////////////////
// TASKS
//////////////////////////////////////////////////////////////////////

Task("Clean")
    .Does(() =>
{
    CleanDirectory(BuildDirectory);
});

Task("Generate-Project")
    .IsDependentOn("Clean")
    .Does(() =>
{
    var generator = "Visual Studio 14 Win64";

    if(platform == "x86")
    {
        generator = "Visual Studio 14";
    }

    CMake("./", new CMakeSettings {
      OutputPath = OutputDirectory,
      Generator = generator,
      Toolset = "v140"
    });
});

Task("Build")
    .IsDependentOn("Generate-Project")
    .Does(() =>
{
    var settings = new MSBuildSettings()
                        .SetConfiguration(configuration);

    if(platform == "x86")
    {
        settings.WithProperty("Platform", "Win32")
                .SetPlatformTarget(PlatformTarget.x86);
    }
    else
    {
        settings.WithProperty("Platform", "x64")
                .SetPlatformTarget(PlatformTarget.x64);
    }

    MSBuild(OutputDirectory + File("PicoTorrent.sln"), settings);
});

Task("Setup-Library-Files")
    .Does(() =>
{
    var files = new FilePath[]
    {
        // 3rd party libraries
        LibraryDirectory + File(BoostSystem),
        LibraryDirectory + File("libeay32.dll"),
        LibraryDirectory + File("ssleay32.dll"),
        LibraryDirectory + File("torrent.dll")
    };

    CopyFiles(files, BuildDirectory);
});

Task("Setup-Publish-Directory")
    .IsDependentOn("Build")
    .IsDependentOn("Setup-Library-Files")
    .Does(() =>
{

    var files = new FilePath[]
    {
        BuildDirectory + File("PicoTorrent.exe"),
        // Plugins
        BuildDirectory + File("Importer.dll"),
        BuildDirectory + File("UpdateChecker.dll"),
        // 3rd party libraries
        LibraryDirectory + File(BoostSystem),
        LibraryDirectory + File("libeay32.dll"),
        LibraryDirectory + File("ssleay32.dll"),
        LibraryDirectory + File("torrent.dll")
    };

    CreateDirectory(PublishDirectory);
    CopyFiles(files, PublishDirectory);
    CopyDirectory(Directory("lang"), PublishDirectory + Directory("lang"));
    DeleteFile(PublishDirectory + Directory("lang") + File("1033.json"));
});

Task("Build-Installer")
    .IsDependentOn("Build")
    .IsDependentOn("Setup-Publish-Directory")
    .Does(() =>
{
    var arch = Architecture.X64;

    if(platform == "x86")
    {
        arch = Architecture.X86;
    }

    WiXCandle("./packaging/WiX/PicoTorrent.wxs", new CandleSettings
    {
        Architecture = arch,
        Defines = new Dictionary<string, string>
        {
            { "Configuration", configuration },
            { "PublishDirectory", PublishDirectory },
            { "Platform", platform },
            { "ResourceDirectory", ResourceDirectory },
            { "Version", Version }
        },
        OutputDirectory = BuildDirectory
    });

    WiXLight(BuildDirectory + File("PicoTorrent.wixobj"), new LightSettings
    {
        OutputFile = BuildDirectory + File(Installer)
    });
});

Task("Build-Installer-Bundle")
    .IsDependentOn("Build")
    .Does(() =>
{
    var arch = Architecture.X64;

    if(platform == "x86")
    {
        arch = Architecture.X86;
    }

    WiXCandle("./packaging/WiX/PicoTorrentBundle.wxs", new CandleSettings
    {
        Architecture = arch,
        Extensions = new [] { "WixBalExtension", "WixNetFxExtension", "WixUtilExtension" },
        Defines = new Dictionary<string, string>
        {
            { "PicoTorrentInstaller", BuildDirectory + File(Installer) },
            { "Platform", platform },
            { "Version", Version }
        },
        OutputDirectory = BuildDirectory
    });

    WiXLight(BuildDirectory + File("PicoTorrentBundle.wixobj"), new LightSettings
    {
        Extensions = new [] { "WixBalExtension", "WixNetFxExtension", "WixUtilExtension" },
        OutputFile = PackagesDirectory + File(InstallerBundle)
    });
});

Task("Build-Portable-Package")
    .IsDependentOn("Build")
    .IsDependentOn("Setup-Publish-Directory")
    .Does(() =>
{
    Zip(PublishDirectory, PackagesDirectory + File(PortablePackage));
});

Task("Build-Symbols-Package")
    .IsDependentOn("Build")
    .Does(() =>
{
    var files = new FilePath[]
    {
        BuildDirectory + File("PicoTorrent.pdb"),
        BuildDirectory + File("Importer.pdb"),
        BuildDirectory + File("UpdateChecker.pdb")
    };

    Zip(BuildDirectory, PackagesDirectory + File(SymbolsPackage), files);
});

Task("Build-Chocolatey-Package")
    .IsDependentOn("Build-Installer")
    .Does(() =>
{
    TransformTextFile("./packaging/Chocolatey/tools/chocolateyinstall.ps1.template", "%{", "}")
        .WithToken("Installer", InstallerBundle)
        .WithToken("Version", Version)
        .Save("./packaging/Chocolatey/tools/chocolateyinstall.ps1");

    var currentDirectory = MakeAbsolute(Directory("."));
    var cd = MakeAbsolute(PackagesDirectory);
    var nuspec = MakeAbsolute(File("./packaging/Chocolatey/picotorrent.nuspec"));

    System.IO.Directory.SetCurrentDirectory(cd.ToString());

    ChocolateyPack(nuspec, new ChocolateyPackSettings
    {
        Version = Version
    });

    System.IO.Directory.SetCurrentDirectory(currentDirectory.ToString());
});

Task("Build-AppX-Package")
    .IsDependentOn("Build")
    .IsDependentOn("Setup-Publish-Directory")
    .Does(() =>
{
    var VCRedist = Directory("C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\VC\\redist");
    var VCDir = VCRedist + Directory(platform) + Directory("Microsoft.VC140.CRT");

    var CRTRedist = Directory("C:\\Program Files (x86)\\Windows Kits\\10\\Redist\\ucrt\\DLLs");
    var CRTDir = CRTRedist + Directory(platform);

    TransformTextFile("./packaging/UWP/PicoTorrent.mapping.template", "%{", "}")
        .WithToken("VCDir", VCDir)
        .WithToken("CRTDir", CRTDir)
        .WithToken("PublishDirectory", MakeAbsolute(PublishDirectory))
        .WithToken("ResourceDirectory", MakeAbsolute(ResourceDirectory))
        .WithToken("PackagingDirectory", MakeAbsolute(Directory("./packaging/UWP")))
        .Save("./packaging/UWP/PicoTorrent.mapping");

    TransformTextFile("./packaging/UWP/PicoTorrentManifest.xml.template", "%{", "}")
        .WithToken("Platform", platform)
        .WithToken("Version", Version + ".0")
        .Save("./packaging/UWP/PicoTorrentManifest.xml");

    var argsBuilder = new ProcessArgumentBuilder();

    argsBuilder.Append("pack");
    argsBuilder.Append("/f {0}", MakeAbsolute(File("./packaging/UWP/PicoTorrent.mapping")));
    argsBuilder.Append("/p {0}", MakeAbsolute(PackagesDirectory + File(AppXPackage)));

    var makeAppXTool = "C:\\Program Files (x86)\\Windows Kits\\10\\bin\\x86\\makeappx.exe";
    int exitCode = StartProcess(makeAppXTool, new ProcessSettings
    {
        Arguments = argsBuilder
    });

    if(exitCode != 0)
    {
        throw new CakeException("makeappx.exe exited with error: " + exitCode);
    }
});

Task("Sign")
    .IsDependentOn("Build")
    .WithCriteria(() => SigningCertificate != null)
    .Does(() =>
{
    SignFile(BuildDirectory + File("PicoTorrent.exe"));
});

Task("Sign-AppX-Package")
    .IsDependentOn("Build-AppX-Package")
    .Does(() =>
{
    var signTool = "C:/Program Files (x86)/Windows Kits/10/bin/x86/signtool.exe";
    var args = new ProcessArgumentBuilder();
    args.Append("sign");
    args.Append("/fd SHA256");
    args.Append("/a");
    args.AppendSecret("/f {0}", SigningCertificate);
    args.AppendSecret("/p {0}", SigningPassword);
    args.Append("/tr http://timestamp.digicert.com");
    args.Append(PackagesDirectory + File(AppXPackage));

    int exitCode = StartProcess(signTool, new ProcessSettings { Arguments = args });

    if (exitCode != 0)
    {
        throw new CakeException("Failed to run signtool.exe, exit code " + exitCode);
    }
});

Task("Sign-Installer")
    .IsDependentOn("Build-Installer")
    .WithCriteria(() => SigningCertificate != null)
    .Does(() =>
{
    SignFile(BuildDirectory + File(Installer), "PicoTorrent");
});

Task("Sign-Installer-Bundle")
    .IsDependentOn("Build-Installer-Bundle")
    .WithCriteria(() => SigningCertificate != null)
    .Does(() =>
{
    var bundle = PackagesDirectory + File(InstallerBundle);
    var insignia = Directory("tools")
                   + Directory("WiX")
                   + Directory("tools")
                   + File("insignia.exe");
    var burnEngine = Directory("build-" + platform) + File("BurnEngine.exe");

    // Detach Burn engine
    StartProcess(insignia, "-ib \"" + bundle + "\" -o " + burnEngine);
    SignFile(burnEngine);
    StartProcess(insignia, "-ab " + burnEngine + " \"" + bundle + "\" -o \"" + bundle + "\"");

    // Sign the bundle
    SignFile(bundle, "PicoTorrent");
});

//////////////////////////////////////////////////////////////////////
// TASK TARGETS
//////////////////////////////////////////////////////////////////////

Task("Default")
    .IsDependentOn("Build")
    .IsDependentOn("Build-Installer")
    .IsDependentOn("Build-Installer-Bundle")
    .IsDependentOn("Build-Chocolatey-Package")
    .IsDependentOn("Build-Portable-Package")
    .IsDependentOn("Build-AppX-Package")
    .IsDependentOn("Build-Symbols-Package")
    ;

Task("Publish")
    .IsDependentOn("Build")
    .IsDependentOn("Sign")
    .IsDependentOn("Build-Installer")
    .IsDependentOn("Build-Installer-Bundle")
    .IsDependentOn("Sign-Installer")
    .IsDependentOn("Sign-Installer-Bundle")
    .IsDependentOn("Build-Chocolatey-Package")
    .IsDependentOn("Build-Portable-Package")
    .IsDependentOn("Build-AppX-Package")
    .IsDependentOn("Sign-AppX-Package")
    .IsDependentOn("Build-Symbols-Package");

//////////////////////////////////////////////////////////////////////
// EXECUTION
//////////////////////////////////////////////////////////////////////

RunTarget(target);
