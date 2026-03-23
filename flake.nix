{
  description = "real time rendering with glfw";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = {
    self,
    nixpkgs,
  }: let
    system = "x86_64-linux";
    pkgs = import nixpkgs {inherit system;};
  in {
    devShells.${system}.default = pkgs.mkShell {
      buildInputs = [
        pkgs.glfw
        pkgs.pkg-config
        pkgs.mesa
      ];

      shellHook = ''
        echo boooo
      '';
    };
  };
}
