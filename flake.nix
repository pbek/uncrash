{
  description = "Uncrash - A system protection daemon to prevent crashes due to temperature and VRM issues";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    {
      nixosModules = {
        uncrash =
          {
            config,
            lib,
            pkgs,
            ...
          }:

          let
            inherit (lib)
              mkEnableOption
              mkOption
              mkIf
              types
              ;
            cfg = config.services.uncrash;
          in
          {
            options.services.uncrash = {
              enable = mkEnableOption "Uncrash system protection daemon";

              package = mkOption {
                type = types.package;
                default = pkgs.callPackage ./package.nix { };
                description = "The Uncrash package to use";
              };

              gpuPowerThreshold = mkOption {
                type = types.int;
                default = 100;
                description = "GPU power threshold in watts before CPU throttling is applied";
              };

              cpuMaxFrequency = mkOption {
                type = types.float;
                default = 3.5;
                description = "Maximum CPU frequency in GHz when GPU threshold is exceeded";
              };

              autoProtection = mkOption {
                type = types.bool;
                default = true;
                description = "Enable automatic CPU throttling when GPU power exceeds threshold";
              };
            };

            config = mkIf cfg.enable {
              # Install the package
              environment.systemPackages = [ cfg.package ];

              # Create config directory and file
              environment.etc."uncrash/uncrash.conf" = {
                text = ''
                  [General]
                  gpuPowerThreshold=${toString cfg.gpuPowerThreshold}
                  cpuMaxFrequency=${toString cfg.cpuMaxFrequency}
                  autoProtection=${if cfg.autoProtection then "true" else "false"}
                '';
                mode = "0644";
              };

              # Enable and configure systemd service
              systemd.services.uncrashd = {
                description = "Uncrash System Protection Daemon";
                documentation = [ "https://github.com/pbek/uncrash" ];
                after = [ "network.target" ];
                wantedBy = [ "multi-user.target" ];

                serviceConfig = {
                  Type = "simple";
                  ExecStart = "${cfg.package}/bin/uncrashd";
                  Restart = "on-failure";
                  RestartSec = "5s";

                  # Run as root (needed for CPU frequency control)
                  User = "root";
                  Group = "root";

                  # Security hardening
                  NoNewPrivileges = true;
                  PrivateTmp = true;
                  ProtectSystem = "strict";
                  ProtectHome = true;
                  ReadWritePaths = [
                    "/etc/uncrash"
                    "/sys/devices/system/cpu"
                  ];

                  # Logging
                  StandardOutput = "journal";
                  StandardError = "journal";
                  SyslogIdentifier = "uncrashd";
                };
              };

              # Register DBus service
              services.dbus.packages = [ cfg.package ];
            };
          };
      };
    }
    // flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages = {
          uncrash = pkgs.callPackage ./package.nix { };
          default = self.packages.${system}.uncrash;
        };

        apps = {
          uncrash = {
            type = "app";
            program = "${self.packages.${system}.uncrash}/bin/uncrash";
          };
          default = self.apps.${system}.uncrash;
        };

        devShells.default = pkgs.mkShell {
          inputsFrom = [ self.packages.${system}.uncrash ];
          packages = with pkgs; [
            just
            git
          ];
        };
      }
    );
}
