{
  cmake,
  coreutils,
  installShellFiles,
  kdePackages,
  lib,
  makeWrapper,
  ninja,
  pciutils,
  pkg-config,
  polkit,
  qt6,
  stdenv,
  xvfb-run,
}:

stdenv.mkDerivation (
  finalAttrs:
  let
    # Extract version from CMakeLists.txt
    cmakeListsContent = builtins.readFile ./CMakeLists.txt;
    versionMatch = builtins.match ".*VERSION ([0-9]+\.[0-9]+\.[0-9]+).*" cmakeListsContent;
    version = if versionMatch != null then builtins.head versionMatch else "0.0.0";
  in
  {
    pname = "uncrash";
    inherit version;

    src = lib.cleanSourceWith {
      src = ./.;
      filter =
        path: _type:
        let
          baseName = baseNameOf path;
        in
        !(builtins.elem baseName [
          "build"
          "cmake-build-debug"
          "result"
          ".git"
          ".gitignore"
          ".cache"
        ]);
    };

    nativeBuildInputs = [
      cmake
      installShellFiles
      kdePackages.extra-cmake-modules
      makeWrapper
      ninja
      pkg-config
      qt6.wrapQtAppsHook
    ]
    ++ lib.optionals stdenv.hostPlatform.isLinux [ xvfb-run ];

    buildInputs = [
      kdePackages.kconfig
      kdePackages.kcoreaddons
      kdePackages.ki18n
      kdePackages.kirigami
      polkit
      qt6.qtbase
      qt6.qtdeclarative
      qt6.qtwayland
    ];

    # Substitute paths in systemd service file
    postPatch = ''
      substituteInPlace uncrashd.service.in \
        --replace-fail '@out@' "$out"
      mv uncrashd.service.in uncrashd.service
    '';

    # Install shell completion on Linux (with xvfb-run) and systemd service
    postInstall = ''
      installShellCompletion --cmd uncrash \
        --bash <(xvfb-run $out/bin/uncrash --completion-bash) \
        --fish <(xvfb-run $out/bin/uncrash --completion-fish)

      # Install systemd service (service file is in source root after postPatch)
      install -Dm644 ../uncrashd.service $out/lib/systemd/system/uncrashd.service

      # Install DBus policy file
      install -Dm644 ../org.uncrash.Daemon.conf $out/share/dbus-1/system.d/org.uncrash.Daemon.conf

      # Create config directory structure
      mkdir -p $out/etc/uncrash
    '';

    # Wrap both binaries to include necessary system tools in PATH
    # Note: nvidia-smi should be provided by the system if NVIDIA GPU is present
    # We suffix our paths to prioritize system pkexec (which is setuid root)
    postFixup = ''
      wrapProgram $out/bin/uncrash \
        --suffix PATH : ${
          lib.makeBinPath [
            coreutils # for tee (used by pkexec for writing to sysfs)
            pciutils # for lspci (useful for hardware detection)
            polkit # for pkexec (privilege escalation for CPU frequency control)
          ]
        }

      wrapProgram $out/bin/uncrashd \
        --suffix PATH : ${
          lib.makeBinPath [
            coreutils # for tee (used by pkexec for writing to sysfs)
            pciutils # for lspci (useful for hardware detection)
          ]
        }
    '';

    meta = with lib; {
      description = "System protection daemon to prevent desktop crashes due to temperature and VRM issues";
      homepage = "https://github.com/pbek/uncrash";
      changelog = "https://github.com/pbek/uncrash/releases/tag/v${finalAttrs.version}";
      license = licenses.gpl3Plus;
      maintainers = with lib.maintainers; [ pbek ];
      platforms = platforms.linux;
      mainProgram = "uncrash";
    };
  }
)
