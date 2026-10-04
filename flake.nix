{
  description = "Text Editor";
  inputs =
  {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-26.05";
  };
  outputs = {self, nixpkgs, ...}@inputs: 
  let
    supported_systems = ["x86_64-linux"];
    for_supported_systems = nixpkgs.lib.genAttrs supported_systems;
  in
  {
    packages = for_supported_systems
    (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        default = pkgs.stdenv.mkDerivation
        {
          pname = "te";
          version = "1.0";
          src = ./.;
          outputs =
          [
            "out"
          ];
          buildPhase =
          ''
            make build/te
          '';
          installPhase =
          ''
            mkdir -p $out/bin
            cp build/te $out/bin
          '';
        };
      }
    );
  };
}
